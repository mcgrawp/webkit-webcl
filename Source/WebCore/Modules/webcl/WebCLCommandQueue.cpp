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
#include "WebCLMemoryObject.h"
#include "WebCLProgram.h"

namespace WebCore {

WebCLCommandQueue::~WebCLCommandQueue()
{
    releasePlatformObject();
}

PassRefPtr<WebCLCommandQueue> WebCLCommandQueue::create(WebCLContext* context, CCenum properties, const RefPtr<WebCLDevice>& webCLDevice, ExceptionCode& ec)
{
    CCerror error = ComputeContext::SUCCESS;
    CCCommandQueue clCommandQueue = context->computeContext()->createCommandQueue(webCLDevice->platformObject(), properties, error);
    if (!clCommandQueue) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLCommandQueue> queue = adoptRef(new WebCLCommandQueue(context, webCLDevice, clCommandQueue));
    return queue.release();
}

WebCLCommandQueue::WebCLCommandQueue(WebCLContext* context, const RefPtr<WebCLDevice>& webCLDevice, CCCommandQueue commandQueue)
    : WebCLObject(commandQueue)
    , m_context(context)
    , m_device(webCLDevice)
{
}

WebCLGetInfo WebCLCommandQueue::getInfo(CCenum paramName, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return WebCLGetInfo();
    }

    CCerror err = 0;

    switch (paramName) {
    case ComputeContext::QUEUE_CONTEXT:
        return WebCLGetInfo(m_context.get());
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

void WebCLCommandQueue::enqueueWriteBufferBase(WebCLBuffer* buffer, CCbool blockingWrite, CCuint offset, CCuint bufferSize, void* data,
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

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, CCbool blockingWrite, CCuint offset, CCuint bufferSize, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    enqueueWriteBufferBase(buffer, blockingWrite, offset, bufferSize, ptr ? ptr->baseAddress() : 0, events, event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, CCbool blockingWrite, CCuint offset, ImageData* imageData, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
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

void WebCLCommandQueue::enqueueWriteBufferRect(WebCLBuffer* buffer, CCbool blockingWrite, Int32Array* bufferOrigin,
    Int32Array* hostOrigin, Int32Array* region, CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch,
    CCuint hostSlicePitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
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

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, CCbool blockingRead, CCuint offset, ImageData* imageData, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
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

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, CCbool blockingRead, Int32Array* origin, Int32Array* region,
    CCuint rowPitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
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

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* sourceBuffer, CCbool blockingRead, CCuint offset, CCuint bufferSize,
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

void WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, CCbool blockingRead,
    Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
    CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch,
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

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, CCbool blockingWrite, Int32Array* origin, Int32Array* region,
    CCuint inputRowPitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event,
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
    sourceOrigin, Int32Array* region, CCuint targetOffset, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
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

void WebCLCommandQueue::enqueueCopyBufferToImage(WebCLBuffer *sourceBuffer, WebCLImage *targetImage, CCuint sourceOffset,
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

void WebCLCommandQueue::enqueueCopyBuffer(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, CCuint sourceOffset,
    CCuint targetOffset, CCuint sizeInBytes, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
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
    sourceOrigin, Int32Array* targetOrigin, Int32Array* region, CCuint sourceRowPitch, CCuint sourceSlicePitch, CCuint
    targetRowPitch, CCuint targetSlicePitch, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
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

#if ENABLE(WEBGL)
void WebCLCommandQueue::enqueueAcquireGLObjects(const Vector<RefPtr<WebCLMemoryObject> >& memoryObjects, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    Vector<PlatformComputeObject> ccMemoryObjectIDs;
    for (size_t i = 0; i < memoryObjects.size(); ++i) {
        if (!memoryObjects[i]->sharesGLResources()) {
            ec = WebCLException::INVALID_GL_OBJECT;
            return;
        }
        ccMemoryObjectIDs.append(memoryObjects[i]->platformObject());
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

    CCerror err = m_context->computeContext()->enqueueAcquireGLObjects(platformObject(), ccMemoryObjectIDs, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReleaseGLObjects(const Vector<RefPtr<WebCLMemoryObject> >& memoryObjects, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    Vector<PlatformComputeObject> ccMemoryObjectIDs;
    for (size_t i = 0; i < memoryObjects.size(); ++i) {
        if (!memoryObjects[i]->sharesGLResources()) {
            ec = WebCLException::INVALID_GL_OBJECT;
            return;
        }
        ccMemoryObjectIDs.append(memoryObjects[i]->platformObject());
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

    CCerror err = m_context->computeContext()->enqueueReleaseGLObjects(platformObject(), ccMemoryObjectIDs, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}
#endif

} // namespace WebCore

#endif // ENABLE(WEBCL)
