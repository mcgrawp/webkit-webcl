/*
 * Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SAMSUNG ELECTRONICS CORPORATION AND ITS
 * CONTRIBUTORS "AS IS", AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG
 * ELECTRONICS CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS, OR BUSINESS INTERRUPTION), HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
 * NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(WEBCL)

#include "WebCLCommandQueue.h"

#include "WebCL.h"
#include "WebCLContext.h"
#include "WebCLException.h"
#include <wtf/ArrayBuffer.h>
#include <wtf/Uint8ClampedArray.h>

namespace WebCore {

WebCLCommandQueue::~WebCLCommandQueue()
{
    ASSERT(m_ccCommandQueue);
    CCerror computeContextErrorCode = m_context->computeContext()->releaseCommandQueue(m_ccCommandQueue);

    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
    m_ccCommandQueue = 0;
}

PassRefPtr<WebCLCommandQueue> WebCLCommandQueue::create(WebCLContext* context, int commandQueueProperty, const RefPtr<WebCLDevice>& webCLDevice, ExceptionCode& ec)
{
    CCerror error = ComputeContext::SUCCESS;
    CCCommandQueue clCommandQueue = context->computeContext()->createCommandQueue(webCLDevice->getCLDevice(), commandQueueProperty, error);
    if (!clCommandQueue) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    return adoptRef(new WebCLCommandQueue(context, webCLDevice, clCommandQueue));
}

WebCLCommandQueue::WebCLCommandQueue(WebCLContext* context, const RefPtr<WebCLDevice>& webCLDevice, CCCommandQueue commandQueue)
    : m_context(context)
    , m_device(webCLDevice)
    , m_ccCommandQueue(commandQueue)
{
}

WebCLGetInfo WebCLCommandQueue::getInfo(int paramName, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return WebCLGetInfo();
    }

    CCerror err = 0;
    CCCommandQueueProperties ccCommandQueueProperties = 0;

    switch (paramName) {
    case ComputeContext::QUEUE_CONTEXT:
        return WebCLGetInfo(PassRefPtr<WebCLContext>(m_context));
    case ComputeContext::QUEUE_DEVICE:
        return WebCLGetInfo(PassRefPtr<WebCLDevice>(const_cast<WebCLDevice*>(m_device.get())));
    case ComputeContext::QUEUE_PROPERTIES:
        err = ComputeContext::getCommandQueueInfo(m_ccCommandQueue, paramName, sizeof(CCCommandQueueProperties), &ccCommandQueueProperties);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(ccCommandQueueProperties));
        break;
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

void WebCLCommandQueue::enqueueWriteBufferBase(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, void* data, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccBuffer = nullptr;
    if (buffer)
        ccBuffer = buffer->getCLBuffer();
    if (!ccBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (!data) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror error = m_context->computeContext()->enqueueWriteBuffer(m_ccCommandQueue, ccBuffer, blockingWrite, offset, bufferSize, data, events.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ArrayBufferView* ptr, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    enqueueWriteBufferBase(buffer, blockingWrite, offset, bufferSize, ptr ? ptr->baseAddress() : 0, events, event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ImageData* ptr, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!ptr && !ptr->data() && !ptr->data()->data()) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    unsigned char* bufferArray = ptr->data()->data();
    enqueueWriteBufferBase(buffer, blockingWrite, offset, bufferSize, static_cast<void*>(bufferArray), events, event, ec);
}

void WebCLCommandQueue::enqueueWriteBufferRect(WebCLBuffer* buffer, bool blockingWrite, Int32Array* bufferOrigin,
    Int32Array* hostOrigin, Int32Array* region, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch,
    int hostSlicePitch, ArrayBufferView* ptr, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccBuffer = 0;
    if (buffer)
        ccBuffer = buffer->getCLBuffer();
    if (!ccBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    if (!ptr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    if (!bufferOrigin || !hostOrigin || !region) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (bufferOrigin->length() < 3
        || hostOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t, 3> bufferOriginData;
    Vector<size_t, 3> hostOriginData;
    Vector<size_t, 3> regionData;

    for (size_t i = 0; i < 3; ++i) {
        bufferOriginData[i] = bufferOrigin->item(i);
        hostOriginData[i] = hostOrigin->item(i);
        regionData[i] = region->item(i);
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueWriteBufferRect(m_ccCommandQueue, ccBuffer, blockingWrite,
        bufferOriginData.data(), hostOriginData.data(), regionData.data(), bufferRowPitch, bufferSlicePitch,
        hostRowPitch, hostSlicePitch, ptr->baseAddress(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize,
    ImageData* ptr, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccBuffer = 0;
    if (buffer)
        ccBuffer = buffer->getCLBuffer();
    if (!ccBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    unsigned char* bufferArray;
    if (ptr && ptr->data() && ptr->data()->data()) {
        bufferArray = ptr->data()->data();
        bufferSize =  ptr->data()->length();
    } else {
        ec = ComputeContext::INVALID_VALUE;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueReadBuffer(m_ccCommandQueue, ccBuffer, blockingRead, offset,
        bufferSize, bufferArray, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, bool blockingRead, Int32Array* origin, Int32Array* region,
    int rowPitch, int slicePitch, ArrayBufferView* ptr, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccImage = 0;
    if (image)
        ccImage = image->getCLImage();
    if (!ccImage) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (!origin || !region || !ptr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    if (origin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t, 3> originData;
    Vector<size_t, 3> regionData;
    for (size_t i = 0; i < 3; ++i) {
        originData[i] = origin->item(i);
        regionData[i] = region->item(i);
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueReadImage(m_ccCommandQueue, ccImage, blockingRead, originData.data(),
        regionData.data(), rowPitch, slicePitch, ptr->baseAddress(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* sourceBuffer, bool blockingRead, int offset, int bufferSize,
    ArrayBufferView* ptr, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }
    if (!ptr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    PlatformComputeObject ccBuffer = 0;
    if (sourceBuffer)
        ccBuffer = sourceBuffer->getCLBuffer();
    if (!ccBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueReadBuffer(m_ccCommandQueue, ccBuffer, blockingRead, offset,
        bufferSize, ptr->baseAddress(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, bool blockingRead,
    Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
    int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch,
    ArrayBufferView* ptr, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccBuffer = 0;
    if (buffer)
        ccBuffer = buffer->getCLBuffer();
    if (!ccBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    if (!bufferOrigin || !hostOrigin ||!region || !ptr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (bufferOrigin->length() < 3
        || hostOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t, 3> bufferOriginData;
    Vector<size_t, 3> hostOriginData;
    Vector<size_t, 3> regionData;
    for (size_t i = 0; i < 3; ++i) {
        bufferOriginData[i] = bufferOrigin->item(i);
        hostOriginData[i] = hostOrigin->item(i);
        regionData[i] = region->item(i);
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueReadBufferRect(m_ccCommandQueue, ccBuffer, blockingRead,
        bufferOriginData.data(), hostOriginData.data(), regionData.data(), bufferRowPitch, bufferSlicePitch,
        hostRowPitch, hostSlicePitch, ptr->baseAddress(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* globalWorkOffsets,
    Int32Array* globalWorkSize, Int32Array* localWorkSize, const Vector<WebCLEvent*>& events,
    WebCLEvent* event, ExceptionCode& ec)
{
    printf(" DEBUG:: in enqueueNDRangeKernel\n");
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!kernel || !kernel->getCLKernel()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }


    CCKernel ccKernel = kernel->getCLKernel();

    // FIXME: The block of code is repeated all over this class.
    Vector<size_t> globalWorkSizeCopy;
    int workItemDimensions = 0;
    if (globalWorkSize) {
        for (size_t i = 0; i < globalWorkSize->length(); ++i)
            globalWorkSizeCopy.append(globalWorkSize->item(i));
        workItemDimensions = globalWorkSize->length();
    }

    if (workItemDimensions < 0 || workItemDimensions > 3) {
        ec = WebCLException::INVALID_WORK_DIMENSION;
        return;
    }

    Vector<size_t> localWorkSizeCopy;
    if (localWorkSize) {
        for (size_t i = 0; i < localWorkSize->length(); ++i)
            localWorkSizeCopy.append(localWorkSize->item(i));
    }

    Vector<size_t> globalWorkOffsetCopy;
    if (globalWorkOffsets) {
        for (unsigned i = 0; i < globalWorkOffsets->length(); ++i)
            globalWorkOffsetCopy.append(globalWorkOffsets->item(i));
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror computeContextError = m_context->computeContext()->enqueueNDRangeKernel(m_ccCommandQueue, ccKernel,
        workItemDimensions, globalWorkOffsetCopy.data(), globalWorkSizeCopy.data(), localWorkSizeCopy.data(),
        ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::finish(ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCerror computeContextError = m_context->computeContext()->finishCommandQueue(m_ccCommandQueue);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}


void WebCLCommandQueue::flush(ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCerror computeContextError = m_context->computeContext()->flushCommandQueue(m_ccCommandQueue);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, bool blockingWrite, Int32Array* origin, Int32Array* region,
    int inputRowPitch, int inputSlicePitch, ArrayBufferView* ptr, const Vector<WebCLEvent*>& events, WebCLEvent* event,
    ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccImage = 0;
    if (image)
        ccImage = image->getCLImage();
    if (!ccImage) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    if (!origin || !region || !ptr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (origin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t, 3> originData;
    Vector<size_t, 3> regionData;
    for (size_t i = 0; i < 3; ++i) {
        originData[i] = origin->item(i);
        regionData[i] = region->item(i);
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueWriteImage(m_ccCommandQueue, ccImage, blockingWrite,
        originData.data(), regionData.data(), inputRowPitch, inputSlicePitch, ptr->baseAddress(), ccEvents.size(),
        ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueAcquireGLObjects(WebCLMemoryObject* memoryObjects, const Vector<WebCLEvent*>& events,
    WebCLEvent* event, ExceptionCode& ec)
{

    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccMemoryObjectsIDs[1] = {0};
    if (memoryObjects && memoryObjects->isShared())
        ccMemoryObjectsIDs[0] = memoryObjects->getCLMemoryObject();
    if (!ccMemoryObjectsIDs[0]) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueAcquireGLObjects(m_ccCommandQueue, 1, ccMemoryObjectsIDs,
        ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReleaseGLObjects(WebCLMemoryObject* memoryObjects, const Vector<WebCLEvent*>& events,
    WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccMemoryObjectsIDs = 0;
    if (memoryObjects && memoryObjects->isShared())
        ccMemoryObjectsIDs = memoryObjects->getCLMemoryObject();
    if (!ccMemoryObjectsIDs) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror error = m_context->computeContext()->enqueueReleaseGLObjects(m_ccCommandQueue, 1, &ccMemoryObjectsIDs,
        ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
}
void WebCLCommandQueue::enqueueCopyImage(WebCLImage* sourceImage, WebCLImage* targetImage, Int32Array* sourceOrigin,
    Int32Array* targetOrigin, Int32Array* region, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccSourceImage = 0;
    if (sourceImage)
        ccSourceImage = sourceImage->getCLImage();
    if (!ccSourceImage) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccTargetImage = 0;
    if (targetImage)
        ccTargetImage = targetImage->getCLImage();
    if (!ccTargetImage) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (!sourceOrigin || !targetOrigin || !region) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    if (sourceOrigin->length() < 3
        || targetOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t, 3> sourceOriginData;
    Vector<size_t, 3> targetOriginData;
    Vector<size_t, 3> regionData;

    for (size_t i = 0; i < 3; ++i) {
        sourceOriginData[i] = sourceOrigin->item(i);
        targetOriginData[i] = targetOrigin->item(i);
        regionData[i] = region->item(i);
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyImage(m_ccCommandQueue, ccSourceImage, ccTargetImage,
        sourceOriginData.data(), targetOriginData.data(), regionData.data(), ccEvents.size(), ccEvents.data(), ccEvent);
    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyImageToBuffer(WebCLImage *sourceImage, WebCLBuffer *targetBuffer, Int32Array*
    sourceOrigin, Int32Array* region, int targetOffset, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccSourceImage = 0;
    if (sourceImage)
        ccSourceImage = sourceImage->getCLImage();
    if (!ccSourceImage) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccTargetBuffer = 0;
    if (targetBuffer)
        ccTargetBuffer = targetBuffer->getCLBuffer();
    if (!ccTargetBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    if (!sourceOrigin || !region) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    if (sourceOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t, 3> sourceOriginData;
    Vector<size_t, 3> regionData;
    for (size_t i = 0; i < 3; ++i) {
        sourceOriginData[i] = sourceOrigin->item(i);
        regionData[i] = region->item(i);
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyImageToBuffer(m_ccCommandQueue, ccSourceImage, ccTargetBuffer,
        sourceOriginData.data(), regionData.data(), targetOffset, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBufferToImage(WebCLBuffer *sourceBuffer, WebCLImage *targetImage, int sourceOffset,
    Int32Array* targetOrigin, Int32Array* region, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccSourceBuffer = 0;
    if (sourceBuffer)
        ccSourceBuffer = sourceBuffer->getCLBuffer();
    if (!ccSourceBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccTargetImage = 0;
    if (targetImage)
        ccTargetImage = targetImage->getCLImage();
    if (!ccTargetImage) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (!targetOrigin || !region) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (targetOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t, 3> targetOriginData;
    Vector<size_t, 3> regionData;
    for (size_t i = 0; i < 3; ++i) {
        targetOriginData[i] = targetOrigin->item(i);
        regionData[i] = region->item(i);
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyBufferToImage(m_ccCommandQueue, ccSourceBuffer, ccTargetImage,
        sourceOffset, targetOriginData.data(), regionData.data(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBuffer(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, int sourceOffset,
    int targetOffset, int sizeInBytes, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccSourceBuffer = 0;
    if (sourceBuffer)
        ccSourceBuffer = sourceBuffer->getCLBuffer();
    if (!ccSourceBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccTargetBuffer = 0;
    if (targetBuffer)
        ccTargetBuffer = targetBuffer->getCLBuffer();
    if (!ccTargetBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyBuffer(m_ccCommandQueue, ccSourceBuffer, ccTargetBuffer,
        sourceOffset, targetOffset, sizeInBytes, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBufferRect(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, Int32Array*
    sourceOrigin, Int32Array* targetOrigin, Int32Array* region, int sourceRowPitch, int sourceSlicePitch, int
    targetRowPitch, int targetSlicePitch, const Vector<WebCLEvent*>& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    PlatformComputeObject ccSourceBuffer = 0;
    if (sourceBuffer)
        ccSourceBuffer = sourceBuffer->getCLBuffer();
    if (!ccSourceBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccTargetBuffer = 0;
    if (targetBuffer)
        ccTargetBuffer = targetBuffer->getCLBuffer();
    if (!ccTargetBuffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    if (!sourceOrigin || !targetOrigin || !region) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    if (sourceOrigin->length() < 3
        || targetOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t, 3> sourceOriginData;
    Vector<size_t, 3> targetOriginData;
    Vector<size_t, 3> regionData;
    for (size_t i = 0; i < 3; ++i) {
        sourceOriginData[i] = sourceOrigin->item(i);
        targetOriginData[i] = targetOrigin->item(i);
        regionData[i] = region->item(i);
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyBufferRect(m_ccCommandQueue, ccSourceBuffer, ccTargetBuffer,
        sourceOriginData.data(), targetOriginData.data(), regionData.data(), sourceRowPitch, sourceSlicePitch,
        targetRowPitch, targetSlicePitch, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueBarrier(ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCerror computeContextError = m_context->computeContext()->enqueueBarrier(m_ccCommandQueue);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueMarker(WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    // FIXME: Crash!?
    CCEvent* ccEvent = 0;
    if (event)
        *ccEvent = event->getCLEvent();
    if (!ccEvent) {
        ec = WebCLException::INVALID_EVENT;
        return;
    }

    CCerror computeContextError = m_context->computeContext()->enqueueMarker(m_ccCommandQueue, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueTask(WebCLKernel* kernel, const Vector<WebCLEvent*>& events, WebCLEvent* event,
    ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCKernel ccKernel = 0;
    if (kernel)
        ccKernel = kernel->getCLKernel();
    if (!ccKernel) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    // FIXME: Crash!?
    CCEvent* ccEvent = nullptr;
    if (event)
        *ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueTask(m_ccCommandQueue, ccKernel, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
