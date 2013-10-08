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

#include "ImageData.h"
#include "WebCLBuffer.h"
#include "WebCLContext.h"
#include "WebCLEvent.h"
#include "WebCLGetInfo.h"
#include "WebCLImage.h"
#include "WebCLImageDescriptor.h"
#include "WebCLInputChecker.h"
#include "WebCLKernel.h"
#include "WebCLProgram.h"

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
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(ccCommandQueueProperties));
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != ComputeContext::SUCCESS);
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

    CCerror error = m_context->computeContext()->enqueueWriteBuffer(platformObject(), ccBuffer, blockingWrite, offset, bufferSize, data, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    enqueueWriteBufferBase(buffer, blockingWrite, offset, bufferSize, ptr ? ptr->baseAddress() : 0, events, event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, ImageData* imageData, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!imageData && !imageData->data() && !imageData->data()->data()) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    unsigned byteLength = imageData->data()->length();
    unsigned char* bufferArray = imageData->data()->data();

    enqueueWriteBufferBase(buffer, blockingWrite, offset, byteLength, static_cast<void*>(bufferArray), events, event, ec);
}

static bool toVector(Int32Array* array, Vector<size_t>& result, int sizeCheck = 0)
{
    if (!array)
        return false;
    int length = array->length();
    // When no sizeCheck is specified, take length of Int32Array as sizeCheck
    // to avoid return false on size check and to copy whole array into vector.
    sizeCheck = sizeCheck ? sizeCheck : length;

    if (length < sizeCheck)
        return false;

    for (int i = 0; i < sizeCheck; i++)
        result.append(array->item(i));
    return true;
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

    if (bufferRowPitch < 0 || bufferSlicePitch < 0 || hostRowPitch < 0 || hostSlicePitch < 0) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t> bufferOriginData, hostOriginData, regionData;
    if (!toVector(bufferOrigin, bufferOriginData, 3)
        || !toVector(hostOrigin, hostOriginData, 3)
        || !toVector(region, regionData, 3)) {
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

    CCerror err = m_context->computeContext()->enqueueWriteBufferRect(platformObject(), ccBuffer, blockingWrite, bufferOriginData,
        hostOriginData, regionData, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr->baseAddress(), ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, ImageData* imageData, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (!imageData && !imageData->data() && !imageData->data()->data()) {
        ec = ComputeContext::INVALID_VALUE;
        return;
    }

    PlatformComputeObject ccBuffer = buffer->platformObject();
    unsigned char* bufferArray = imageData->data()->data();
    unsigned bufferSize = imageData->data()->length();

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
        bufferSize, bufferArray, ccEvents, ccEvent);
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

    if (!ptr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (rowPitch < 0) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t> originData, regionData;
    if (!toVector(origin, originData, 2)
        || !toVector(region, regionData, 2)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    originData.append(0);
    regionData.append(1);


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

    CCerror err = m_context->computeContext()->enqueueReadImage(platformObject(), ccImage, blockingRead, originData,
        regionData, rowPitch, 0, ptr->baseAddress(), ccEvents, ccEvent);
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
        bufferSize, ptr->baseAddress(), ccEvents, ccEvent);
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

    if (!ptr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (bufferRowPitch < 0 || bufferSlicePitch < 0 || hostRowPitch < 0 || hostSlicePitch < 0) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t> bufferOriginData, hostOriginData, regionData;
    if (!toVector(bufferOrigin, bufferOriginData, 3)
        || !toVector(hostOrigin, hostOriginData, 3)
        || !toVector(region, regionData, 3)) {
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

    CCerror err = m_context->computeContext()->enqueueReadBufferRect(platformObject(), ccBuffer, blockingRead, bufferOriginData, hostOriginData,
        regionData, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr->baseAddress(), ccEvents, ccEvent);
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

    Vector<size_t> globalWorkSizeCopy;
    if ((globalWorkSize && !toVector(globalWorkSize, globalWorkSizeCopy))) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    int workItemDimensions = globalWorkSizeCopy.size();

    if (workItemDimensions < 0 || workItemDimensions > 3) {
        ec = WebCLException::INVALID_WORK_DIMENSION;
        return;
    }

    Vector<size_t> localWorkSizeCopy, globalWorkOffsetCopy;
    if ( (localWorkSize && !toVector(localWorkSize, localWorkSizeCopy))
        || (globalWorkOffsets && !toVector(globalWorkOffsets, globalWorkOffsetCopy))) {
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
    CCerror computeContextError = m_context->computeContext()->enqueueNDRangeKernel(platformObject(), ccKernel,
        workItemDimensions, globalWorkOffsetCopy, globalWorkSizeCopy, localWorkSizeCopy,
        ccEvents, ccEvent);
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

    if (!ptr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (inputRowPitch < 0) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t> originData, regionData;
    if (!toVector(origin, originData, 2)
        || !toVector(region, regionData, 2)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    originData.append(0);
    regionData.append(1);

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
        originData, regionData, inputRowPitch, 0, ptr->baseAddress(), ccEvents, ccEvent);
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

    Vector<size_t> sourceOriginData, targetOriginData, regionData;
    if (!toVector(sourceOrigin, sourceOriginData, 2)
        || !toVector(targetOrigin, targetOriginData, 2)
        || !toVector(region, regionData, 2)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    sourceOriginData.append(0);
    targetOriginData.append(0);
    regionData.append(1);

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
        sourceOriginData, targetOriginData, regionData, ccEvents, ccEvent);
    if (err != ComputeContext::SUCCESS)
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

    Vector<size_t> sourceOriginData, regionData;
    if (!toVector(sourceOrigin, sourceOriginData, 2)
        || !toVector(region, regionData, 2)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    sourceOriginData.append(0);
    regionData.append(1);


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
        sourceOriginData, regionData, targetOffset, ccEvents, ccEvent);
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

    Vector<size_t> targetOriginData, regionData;
    if (!toVector(targetOrigin, targetOriginData, 2)
        || !toVector(region, regionData, 2)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    targetOriginData.append(0);
    regionData.append(1);

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
        sourceOffset, targetOriginData, regionData, ccEvents, ccEvent);
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
        sourceOffset, targetOffset, sizeInBytes, ccEvents, ccEvent);
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

    if (sourceRowPitch < 0 || sourceSlicePitch < 0 || targetRowPitch < 0 || targetSlicePitch < 0) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    PlatformComputeObject ccSourceBuffer = sourceBuffer->platformObject();
    PlatformComputeObject ccTargetBuffer = targetBuffer->platformObject();

    Vector<size_t> sourceOriginData, targetOriginData, regionData;
    if (!toVector(sourceOrigin, sourceOriginData, 3)
        || !toVector(targetOrigin, targetOriginData, 3)
        || !toVector(region, regionData, 3)) {
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

    CCerror err = m_context->computeContext()->enqueueCopyBufferRect(platformObject(), ccSourceBuffer, ccTargetBuffer,
        sourceOriginData, targetOriginData, regionData, sourceRowPitch, sourceSlicePitch,
        targetRowPitch, targetSlicePitch, ccEvents, ccEvent);
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
