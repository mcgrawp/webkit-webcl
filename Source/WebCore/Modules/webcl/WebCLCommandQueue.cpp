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

#include "WebCLException.h"
#include "WebCLInputChecker.h"
#include <wtf/ArrayBuffer.h>
#include <wtf/Uint8ClampedArray.h>

namespace WebCore {

WebCLCommandQueue::~WebCLCommandQueue()
{
    releasePlatformObject();
}

PassRefPtr<WebCLCommandQueue> WebCLCommandQueue::create(WebCLContext* context, int queueProperties, const RefPtr<WebCLDevice>& webCLDevice, ExceptionCode& ec)
{
    RefPtr<WebCLCommandQueue> queue;
    createBase(context, queueProperties, webCLDevice, ec, queue);
    return queue.release();
}

WebCLCommandQueue::WebCLCommandQueue(WebCLContext* context, const RefPtr<WebCLDevice>& webCLDevice, CCCommandQueue commandQueue)
    : WebCLObject(commandQueue)
    , m_context(context)
    , m_device(webCLDevice)
{
}

WebCLGetInfo WebCLCommandQueue::getInfo(int paramName, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return WebCLGetInfo();
    }

    CCerror err = 0;

    switch (paramName) {
    case ComputeContext::QUEUE_CONTEXT:
        return WebCLGetInfo(m_context);
    case ComputeContext::QUEUE_DEVICE:
        return WebCLGetInfo(m_device.get());
    case ComputeContext::QUEUE_PROPERTIES: {
        CCCommandQueueProperties ccCommandQueueProperties = 0;
        err = ComputeContext::getCommandQueueInfo(platformObject(), paramName, &ccCommandQueueProperties);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(ccCommandQueueProperties));
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

void WebCLCommandQueue::enqueueWriteBufferBase(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, void* data,
    const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    PlatformComputeObject ccBuffer = buffer->platformObject();

    if (!data) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror error = m_context->computeContext()->enqueueWriteBuffer(platformObject(), ccBuffer, blockingWrite, offset, bufferSize, data, events.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    enqueueWriteBufferBase(buffer, blockingWrite, offset, bufferSize, ptr ? ptr->baseAddress() : 0, events, event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ImageData* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
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
    int hostSlicePitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccBuffer = buffer->platformObject();

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
        bufferOriginData.append(bufferOrigin->item(i));
        hostOriginData.append(hostOrigin->item(i));
        regionData.append(region->item(i));
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueWriteBufferRect(platformObject(), ccBuffer, blockingWrite,
        bufferOriginData.data(), hostOriginData.data(), regionData.data(), bufferRowPitch, bufferSlicePitch,
        hostRowPitch, hostSlicePitch, ptr->baseAddress(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize,
    ImageData* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccBuffer = buffer->platformObject();

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
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueReadBuffer(platformObject(), ccBuffer, blockingRead, offset,
        bufferSize, bufferArray, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, bool blockingRead, Int32Array* origin, Int32Array* region,
    int rowPitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(image)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccImage = image->platformObject();

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
        originData.append(origin->item(i));
        regionData.append(region->item(i));
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueReadImage(platformObject(), ccImage, blockingRead, originData.data(),
        regionData.data(), rowPitch, 0, ptr->baseAddress(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* sourceBuffer, bool blockingRead, int offset, int bufferSize,
    ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(sourceBuffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccBuffer = sourceBuffer->platformObject();

    if (!ptr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueReadBuffer(platformObject(), ccBuffer, blockingRead, offset,
        bufferSize, ptr->baseAddress(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, bool blockingRead,
    Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
    int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch,
    ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccBuffer = buffer->platformObject();

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
        bufferOriginData.append(bufferOrigin->item(i));
        hostOriginData.append(hostOrigin->item(i));
        regionData.append(region->item(i));
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueReadBufferRect(platformObject(), ccBuffer, blockingRead,
        bufferOriginData.data(), hostOriginData.data(), regionData.data(), bufferRowPitch, bufferSlicePitch,
        hostRowPitch, hostSlicePitch, ptr->baseAddress(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* globalWorkOffsets,
    Int32Array* globalWorkSize, Int32Array* localWorkSize, const Vector<RefPtr<WebCLEvent> >& events,
    WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(kernel)) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    CCKernel ccKernel = kernel->platformObject();

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
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror computeContextError = m_context->computeContext()->enqueueNDRangeKernel(platformObject(), ccKernel,
        workItemDimensions, globalWorkOffsetCopy.data(), globalWorkSizeCopy.data(), localWorkSizeCopy.data(),
        ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::finish(ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCerror computeContextError = m_context->computeContext()->finishCommandQueue(platformObject());
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}


void WebCLCommandQueue::flush(ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCerror computeContextError = m_context->computeContext()->flushCommandQueue(platformObject());
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, bool blockingWrite, Int32Array* origin, Int32Array* region,
    int inputRowPitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event,
    ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(image)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccImage = image->platformObject();

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
        originData.append(origin->item(i));
        regionData.append(region->item(i));
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueWriteImage(platformObject(), ccImage, blockingWrite,
        originData.data(), regionData.data(), inputRowPitch, 0, ptr->baseAddress(), ccEvents.size(),
        ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyImage(WebCLImage* sourceImage, WebCLImage* targetImage, Int32Array* sourceOrigin,
    Int32Array* targetOrigin, Int32Array* region, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(sourceImage)
        || !WebCLInputChecker::validateWebCLObject(targetImage)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccSourceImage = sourceImage->platformObject();
    PlatformComputeObject ccTargetImage = targetImage->platformObject();

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
        sourceOriginData.append(sourceOrigin->item(i));
        targetOriginData.append(targetOrigin->item(i));
        regionData.append(region->item(i));
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueCopyImage(platformObject(), ccSourceImage, ccTargetImage,
        sourceOriginData.data(), targetOriginData.data(), regionData.data(), ccEvents.size(), ccEvents.data(), ccEvent);
    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyImageToBuffer(WebCLImage *sourceImage, WebCLBuffer *targetBuffer, Int32Array*
    sourceOrigin, Int32Array* region, int targetOffset, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(sourceImage)
        || !WebCLInputChecker::validateWebCLObject(targetBuffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccSourceImage = sourceImage->platformObject();
    PlatformComputeObject ccTargetBuffer = targetBuffer->platformObject();

    if (sourceOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t, 3> sourceOriginData;
    Vector<size_t, 3> regionData;
    for (size_t i = 0; i < 3; ++i) {
        sourceOriginData.append(sourceOrigin->item(i));
        regionData.append(region->item(i));
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueCopyImageToBuffer(platformObject(), ccSourceImage, ccTargetBuffer,
        sourceOriginData.data(), regionData.data(), targetOffset, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBufferToImage(WebCLBuffer *sourceBuffer, WebCLImage *targetImage, int sourceOffset,
    Int32Array* targetOrigin, Int32Array* region, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(sourceBuffer)
        || !WebCLInputChecker::validateWebCLObject(targetImage)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccSourceBuffer = sourceBuffer->platformObject();
    PlatformComputeObject ccTargetImage = targetImage->platformObject();

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
        targetOriginData.append(targetOrigin->item(i));
        regionData.append(region->item(i));
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueCopyBufferToImage(platformObject(), ccSourceBuffer, ccTargetImage,
        sourceOffset, targetOriginData.data(), regionData.data(), ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBuffer(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, int sourceOffset,
    int targetOffset, int sizeInBytes, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(sourceBuffer)
        || !WebCLInputChecker::validateWebCLObject(targetBuffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccSourceBuffer = sourceBuffer->platformObject();
    PlatformComputeObject ccTargetBuffer = targetBuffer->platformObject();

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueCopyBuffer(platformObject(), ccSourceBuffer, ccTargetBuffer,
        sourceOffset, targetOffset, sizeInBytes, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBufferRect(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, Int32Array*
    sourceOrigin, Int32Array* targetOrigin, Int32Array* region, int sourceRowPitch, int sourceSlicePitch, int
    targetRowPitch, int targetSlicePitch, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(sourceBuffer)
        || !WebCLInputChecker::validateWebCLObject(targetBuffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccSourceBuffer = sourceBuffer->platformObject();
    PlatformComputeObject ccTargetBuffer = targetBuffer->platformObject();

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
        sourceOriginData.append(sourceOrigin->item(i));
        targetOriginData.append(targetOrigin->item(i));
        regionData.append(region->item(i));
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueCopyBufferRect(platformObject(), ccSourceBuffer, ccTargetBuffer,
        sourceOriginData.data(), targetOriginData.data(), regionData.data(), sourceRowPitch, sourceSlicePitch,
        targetRowPitch, targetSlicePitch, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueBarrier(ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCerror computeContextError = m_context->computeContext()->enqueueBarrier(platformObject());
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueMarker(WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!event || event->isReleased()) {
        ec = WebCLException::INVALID_EVENT;
        return;
    }

    CCEvent ccEvent = event->platformObject();
    event->setAssociatedCommandQueue(this);

    CCerror computeContextError = m_context->computeContext()->enqueueMarker(platformObject(), &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueTask(WebCLKernel* kernel, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event,
    ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(kernel)) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    CCKernel ccKernel = kernel->platformObject();

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = 0;
    if (event) {
        CCEvent platformEventObject = event->platformObject();
        if (event->isReleased()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }

    CCerror err = m_context->computeContext()->enqueueTask(platformObject(), ccKernel, ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::releasePlatformObjectImpl()
{
    CCerror computeContextErrorCode = m_context->computeContext()->releaseCommandQueue(platformObject());
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
}

CCCommandQueue WebCLCommandQueue::createBaseInternal(WebCLContext* context, int commandQueueProperty, const RefPtr<WebCLDevice>& webCLDevice, CCerror& error)
{
    return context->computeContext()->createCommandQueue(webCLDevice->platformObject(), commandQueueProperty, error);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
