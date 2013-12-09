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
#include "WebCLHTMLInterop.h"
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
    ComputeCommandQueue* computeCommandQueue = context->computeContext()->createCommandQueue(webCLDevice->platformObject(), properties, error);
    if (error != ComputeContext::SUCCESS) {
        delete computeCommandQueue;
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLCommandQueue> queue = adoptRef(new WebCLCommandQueue(context, computeCommandQueue, webCLDevice));
    return queue.release();
}

WebCLCommandQueue::WebCLCommandQueue(WebCLContext* context, ComputeCommandQueue *computeCommandQueue, const RefPtr<WebCLDevice>& webCLDevice)
    : WebCLObjectImpl(computeCommandQueue)
    , m_context(context)
    , m_device(webCLDevice)
{
    context->trackReleaseableWebCLObject(createWeakPtr());
}

WebCLGetInfo WebCLCommandQueue::getInfo(CCenum paramName, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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
        err = platformObject()->commandQueueInfo(paramName, &ccCommandQueueProperties);
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

CCEvent* WebCLCommandQueue::ccEventFromWebCLEvent(WebCLEvent* event, ExceptionCode& ec)
{
    CCEvent* ccEvent = 0;
    if (event) {
        if (event->isPlatformObjectNeutralized()) {
            ec = WebCLException::INVALID_EVENT;
            return ccEvent;
        }
        CCEvent& platformEventObject = event->platformObjectRef();
        ccEvent = &platformEventObject;
        event->setAssociatedCommandQueue(this);
    }
    return ccEvent;
}

void WebCLCommandQueue::enqueueWriteBufferBase(WebCLBuffer* buffer, CCbool blockingWrite, CCuint offset, CCuint bufferSize, void* data,
    const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror error = platformObject()->enqueueWriteBuffer(ccBuffer, blockingWrite, offset, bufferSize, data, ccEvents, ccEvent);
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

void WebCLCommandQueue::enqueueWriteBufferRect(WebCLBuffer* buffer, CCbool blockingWrite, const Vector<unsigned>& bufferOrigin,
    const Vector<unsigned>& hostOrigin, const Vector<unsigned>& region, CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch,
    CCuint hostSlicePitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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

    if (!WebCLInputChecker::isValidPitchForArrayBufferView(hostRowPitch, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidPitchForArrayBufferView(hostSlicePitch, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t> bufferOriginCopy, hostOriginCopy, regionCopy;
    bufferOriginCopy.appendVector(bufferOrigin);
    hostOriginCopy.appendVector(hostOrigin);
    regionCopy.appendVector(region);

    if (bufferOriginCopy.size() != 3 || hostOriginCopy.size() != 3 || regionCopy.size() != 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueWriteBufferRect(ccBuffer, blockingWrite, bufferOriginCopy,
        hostOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr->baseAddress(), ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBufferBase(WebCLBuffer* buffer, CCbool blockingRead, CCuint bufferOffset, CCuint numBytes, void* hostPtr,
    const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccBuffer = buffer->platformObject();

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueReadBuffer(ccBuffer, blockingRead, bufferOffset, numBytes, hostPtr, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, CCbool blockingRead, CCuint bufferOffset, CCuint numBytes,
    ArrayBufferView* hostPtr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!hostPtr || hostPtr->byteLength() < numBytes) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidPitchForArrayBufferView(numBytes, hostPtr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    enqueueReadBufferBase(buffer, blockingRead, bufferOffset, numBytes, hostPtr->baseAddress(), events, event, ec);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, CCbool blockingRead, CCuint offset, CCuint numBytes,
    HTMLCanvasElement* dstCanvas, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    // FIXME :: Need to check if WEBCL_html_sharing is enabled.
    if (!dstCanvas) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }

    void* hostPtr = nullptr;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(dstCanvas, &hostPtr, canvasSize);
    if (!hostPtr || !canvasSize || numBytes > canvasSize) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }
    enqueueReadBufferBase(buffer, blockingRead, offset, numBytes, hostPtr, events, event, ec);
}

void WebCLCommandQueue::enqueueReadImageBase(WebCLImage* image, CCbool blockingRead, const Vector<CCuint>& origin, const Vector<CCuint>& region,
    CCuint hostRowPitch, void* hostPtr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(image)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    PlatformComputeObject ccImage = image->platformObject();

    if (!hostPtr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t> originCopy, regionCopy;
    originCopy.appendVector(origin);
    regionCopy.appendVector(region);

    if (originCopy.size() != 2 || regionCopy.size() != 2) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    originCopy.append(0);
    regionCopy.append(1);

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueReadImage(ccImage, blockingRead, originCopy, regionCopy, hostRowPitch, 0 /* slice_pitch */, hostPtr, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, CCbool blockingRead, const Vector<CCuint>& origin, const Vector<CCuint>& region,
    CCuint rowPitch, ArrayBufferView* hostPtr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!hostPtr || !hostPtr->baseAddress()) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidPitchForArrayBufferView(rowPitch, hostPtr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    enqueueReadImageBase(image, blockingRead, origin, region, rowPitch, hostPtr->baseAddress(), events, event, ec);
}

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, CCbool blockingRead, const Vector<CCuint>& origin, const Vector<CCuint>& region,
    HTMLCanvasElement* dstCanvas, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    // FIXME :: Need to check if WEBCL_html_sharing is enabled.
    if (!dstCanvas) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }
    void* hostPtr = nullptr;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(dstCanvas, &hostPtr, canvasSize);

    if (!hostPtr || !canvasSize) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }
    enqueueReadImageBase(image, blockingRead, origin, region, 0 /* rowPitch */, hostPtr, events, event, ec);
}

void WebCLCommandQueue::enqueueReadBufferRectBase(WebCLBuffer* buffer, CCbool blockingRead,
    const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin, const Vector<CCuint>& region,
    CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch,
    void* hostPtr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    PlatformComputeObject ccBuffer = buffer->platformObject();

    if (!hostPtr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t> bufferOriginCopy, hostOriginCopy, regionCopy;
    bufferOriginCopy.appendVector(bufferOrigin);
    hostOriginCopy.appendVector(hostOrigin);
    regionCopy.appendVector(region);

    if (bufferOriginCopy.size() != 3 || hostOriginCopy.size() != 3 || regionCopy.size() != 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueReadBufferRect(ccBuffer, blockingRead, bufferOriginCopy, hostOriginCopy,
        regionCopy, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, hostPtr, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, CCbool blockingRead,
    const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin, const Vector<CCuint>& region,
    CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch,
    ArrayBufferView* hostPtr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!hostPtr || !hostPtr->baseAddress()) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidPitchForArrayBufferView(hostRowPitch, hostPtr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidPitchForArrayBufferView(hostSlicePitch, hostPtr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    enqueueReadBufferRectBase(buffer, blockingRead, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch,
        hostRowPitch, hostSlicePitch, hostPtr->baseAddress(), events, event, ec);
}

void WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, CCbool blockingRead,
    const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin, const Vector<CCuint>& region,
    CCuint bufferRowPitch, HTMLCanvasElement* dstCanvas, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    // FIXME :: Need to check if WEBCL_html_sharing is enabled.
    if (!dstCanvas) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }

    void* hostPtr = nullptr;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(dstCanvas, &hostPtr, canvasSize);

    if (!hostPtr || !canvasSize) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }

    enqueueReadBufferRectBase(buffer, blockingRead, bufferOrigin, hostOrigin, region, bufferRowPitch, 0 /* bufferSlicePitch */,
        0 /* hostRowPitch */, 0 /* hostSlicePitch */, hostPtr, events, event, ec);
}

void WebCLCommandQueue::enqueueNDRangeKernel(WebCLKernel* kernel, CCuint workDim, const Vector<unsigned>& globalWorkOffsets,
    const Vector<unsigned>& globalWorkSize, const Vector<unsigned>& localWorkSize, const Vector<RefPtr<WebCLEvent> >& events,
    WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(kernel)) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    CCKernel ccKernel = kernel->platformObject();

    if (workDim > 3) {
        ec = WebCLException::INVALID_WORK_DIMENSION;
        return;
    }

    if (workDim != globalWorkSize.size()) {
        ec = WebCLException::INVALID_GLOBAL_WORK_SIZE;
        return;
    }

    if (globalWorkOffsets.size() && workDim != globalWorkOffsets.size()) {
        ec = WebCLException::INVALID_GLOBAL_OFFSET;
        return;
    }
    if (localWorkSize.size() && workDim != localWorkSize.size()) {
        ec = WebCLException::INVALID_WORK_GROUP_SIZE;
        return;
    }

    // FIXME :: Need to add validation if user sent value in each of globalWorkSize, globalWorkOffset and localWorkSize
    // array are valid (not more than 2^32 -1). Currently it is auto clamped by webkit.

    Vector<size_t> globalWorkSizeCopy, localWorkSizeCopy, globalWorkOffsetCopy;
    globalWorkSizeCopy.appendVector(globalWorkSize);
    globalWorkOffsetCopy.appendVector(globalWorkOffsets);
    localWorkSizeCopy.appendVector(localWorkSize);

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror computeContextError = platformObject()->enqueueNDRangeKernel(ccKernel,
        workDim, globalWorkOffsetCopy, globalWorkSizeCopy, localWorkSizeCopy, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::finish(ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCerror computeContextError = platformObject()->finish();
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}


void WebCLCommandQueue::flush(ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCerror computeContextError = platformObject()->flush();
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, CCbool blockingWrite, const Vector<unsigned>& origin, const Vector<unsigned>& region,
    CCuint hostRowPitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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


    if (!WebCLInputChecker::isValidPitchForArrayBufferView(hostRowPitch, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }


    Vector<size_t> originCopy, regionCopy;
    originCopy.appendVector(origin);
    regionCopy.appendVector(region);

    if (originCopy.size() != 2 || regionCopy.size() != 2) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    originCopy.append(0);
    regionCopy.append(1);

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueWriteImage(ccImage, blockingWrite,
        originCopy, regionCopy, hostRowPitch, 0, ptr->baseAddress(), ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyImage(WebCLImage* sourceImage, WebCLImage* targetImage, const Vector<unsigned>& sourceOrigin,
    const Vector<unsigned>& targetOrigin, const Vector<unsigned>& region, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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

    Vector<size_t> sourceOriginCopy, targetOriginCopy, regionCopy;
    sourceOriginCopy.appendVector(sourceOrigin);
    targetOriginCopy.appendVector(targetOrigin);
    regionCopy.appendVector(region);

    if (sourceOriginCopy.size() != 2 || targetOriginCopy.size() != 2 || regionCopy.size() != 2) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    sourceOriginCopy.append(0);
    targetOriginCopy.append(0);
    regionCopy.append(1);

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyImage(ccSourceImage, ccTargetImage,
        sourceOriginCopy, targetOriginCopy, regionCopy, ccEvents, ccEvent);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyImageToBuffer(WebCLImage *sourceImage, WebCLBuffer *targetBuffer, const Vector<unsigned>&
    sourceOrigin, const Vector<unsigned>& region, CCuint targetOffset, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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

    Vector<size_t> sourceOriginCopy, regionCopy;
    sourceOriginCopy.appendVector(sourceOrigin);
    regionCopy.appendVector(region);

    if (sourceOriginCopy.size() != 2 || regionCopy.size() != 2) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    sourceOriginCopy.append(0);
    regionCopy.append(1);


    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyImageToBuffer(ccSourceImage, ccTargetBuffer,
        sourceOriginCopy, regionCopy, targetOffset, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBufferToImage(WebCLBuffer* sourceBuffer, WebCLImage* targetImage, CCuint sourceOffset,
    const Vector<unsigned>& targetOrigin, const Vector<unsigned>& region, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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

    Vector<size_t> targetOriginCopy, regionCopy;
    targetOriginCopy.appendVector(targetOrigin);
    regionCopy.appendVector(region);

    if (targetOriginCopy.size() != 2 || regionCopy.size() != 2) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    targetOriginCopy.append(0);
    regionCopy.append(1);

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyBufferToImage(ccSourceBuffer, ccTargetImage,
        sourceOffset, targetOriginCopy, regionCopy, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBuffer(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, CCuint sourceOffset,
    CCuint targetOffset, CCuint sizeInBytes, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyBuffer(ccSourceBuffer, ccTargetBuffer,
        sourceOffset, targetOffset, sizeInBytes, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBufferRect(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, const Vector<unsigned>& sourceOrigin,
    const Vector<unsigned>& targetOrigin, const Vector<unsigned>& region, CCuint sourceRowPitch, CCuint sourceSlicePitch, CCuint targetRowPitch,
    CCuint targetSlicePitch, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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

    Vector<size_t> sourceOriginCopy, targetOriginCopy, regionCopy;
    sourceOriginCopy.appendVector(sourceOrigin);
    targetOriginCopy.appendVector(targetOrigin);
    regionCopy.appendVector(region);

    if (sourceOriginCopy.size() != 3 || targetOriginCopy.size() != 3 || regionCopy.size() != 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyBufferRect(ccSourceBuffer, ccTargetBuffer,
        sourceOriginCopy, targetOriginCopy, regionCopy, sourceRowPitch, sourceSlicePitch, targetRowPitch, targetSlicePitch, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueBarrier(ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCerror computeContextError = platformObject()->enqueueBarrier();
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueMarker(WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(event)) {
        ec = WebCLException::INVALID_EVENT;
        return;
    }

    CCEvent ccEvent = event->platformObject();
    event->setAssociatedCommandQueue(this);

    CCerror computeContextError = platformObject()->enqueueMarker(&ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::releasePlatformObjectImpl()
{
    CCerror computeContextErrorCode = platformObject()->releaseCommandQueue();
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
}

#if ENABLE(WEBGL)
void WebCLCommandQueue::enqueueAcquireGLObjects(const Vector<RefPtr<WebCLMemoryObject> >& memoryObjects, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueAcquireGLObjects(ccMemoryObjectIDs, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReleaseGLObjects(const Vector<RefPtr<WebCLMemoryObject> >& memoryObjects, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
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

    CCEvent* ccEvent = ccEventFromWebCLEvent(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueReleaseGLObjects(ccMemoryObjectIDs, ccEvents, ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}
#endif

} // namespace WebCore

#endif // ENABLE(WEBCL)
