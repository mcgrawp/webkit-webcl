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

#include "ComputeEvent.h"
#include "ComputeMemoryObject.h"
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
#include <wtf/Int32Array.h>

namespace WebCore {

WebCLCommandQueue::~WebCLCommandQueue()
{
    releasePlatformObject();
}

PassRefPtr<WebCLCommandQueue> WebCLCommandQueue::create(WebCLContext* context, CCenum properties, WebCLDevice* webCLDevice, ExceptionCode& ec)
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

WebCLCommandQueue::WebCLCommandQueue(WebCLContext* context, ComputeCommandQueue *computeCommandQueue, WebCLDevice* webCLDevice)
    : WebCLObjectImpl(computeCommandQueue)
    , m_context(context)
    , m_device(webCLDevice)
    , m_weakFactoryForLazyInitialization(this)
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
            return WebCLGetInfo(static_cast<CCenum>(ccCommandQueueProperties));
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

void WebCLCommandQueue::ccEventListFromWebCLEventList(const Vector<RefPtr<WebCLEvent> >& events, Vector<ComputeEvent*>& computeEvents, ExceptionCode& ec, WebCLToCCEventsFilterCriteria criteria)
{
    for (size_t i = 0; i < events.size(); ++i) {
        RefPtr<WebCLEvent> event = events[i];
        if (event->isPlatformObjectNeutralized()
            || !event->holdsValidCLObject()) {
            ec = WebCLException::INVALID_EVENT_WAIT_LIST;
            return;
        }

        if (criteria == DoNotAcceptUserEvent && event->isUserEvent()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }

        ASSERT(event->context());
        if (!WebCLInputChecker::compareContext(event->context(), m_context.get())) {
            ec = WebCLException::INVALID_CONTEXT;
            return;
        }

        computeEvents.append(event->platformObject());
    }
}

ComputeEvent* WebCLCommandQueue::computeEventFromWebCLEventIfApplicable(WebCLEvent* event, ExceptionCode& ec)
{
    if (!event)
        return 0;

    // Throw an exception if:
    // #1 - Event has been released.
    // #2 - Event has been used before
    // #3 - Event is a user event.
    if (event->isPlatformObjectNeutralized()
        || event->holdsValidCLObject()
        || event->isUserEvent()) {
        ec = WebCLException::INVALID_EVENT;
        return 0;
    }

    event->setAssociatedCommandQueue(this);
    return event->platformObject();
}

void WebCLCommandQueue::enqueueWriteBufferBase(WebCLBuffer* buffer, CCbool blockingWrite, CCuint offset, CCuint numBytes, void* hostPtr, size_t hostPtrLength,
    const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::compareContext(m_context.get(), buffer->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    ComputeMemoryObject* computeMemory = buffer->platformObject();

    if (!hostPtr) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }

    if (hostPtrLength < numBytes
        || buffer->sizeInBytes() < (offset + numBytes)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec, blockingWrite ? DoNotAcceptUserEvent : AcceptUserEvent);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror error = platformObject()->enqueueWriteBuffer(computeMemory, blockingWrite, offset, numBytes, hostPtr, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, CCbool blockingWrite, CCuint offset, CCuint numBytes, ArrayBufferView* ptr,
    const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!ptr || !WebCLInputChecker::isValidDataSizeForArrayBufferView(numBytes, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    enqueueWriteBufferBase(buffer, blockingWrite, offset, numBytes, ptr->baseAddress(), ptr->byteLength(), events, event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, CCbool blockingWrite, CCuint offset, ImageData* srcPixels,
    const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t pixelSize = 0;
    WebCLHTMLInterop::extractDataFromImageData(srcPixels, hostPtr, pixelSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueWriteBufferBase(buffer, blockingWrite, offset, pixelSize, hostPtr, pixelSize, events, event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, CCbool blockingWrite, CCuint offset, HTMLCanvasElement* srcCanvas,
    const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(srcCanvas, hostPtr, canvasSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueWriteBufferBase(buffer, blockingWrite, offset, canvasSize, hostPtr, canvasSize, events, event, ec);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, CCbool blockingWrite, CCuint offset, HTMLImageElement* srcImage,
    const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t imageSize = 0;
    WebCLHTMLInterop::extractDataFromImage(srcImage, hostPtr, imageSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueWriteBufferBase(buffer, blockingWrite, offset, imageSize, hostPtr, imageSize, events, event, ec);
}

void WebCLCommandQueue::enqueueWriteBufferRectBase(WebCLBuffer* buffer, CCbool blockingWrite, const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin,
    const Vector<CCuint>& region, CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch,
    void* hostPtr, size_t hostPtrLength, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::compareContext(m_context.get(), buffer->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    ComputeMemoryObject* computeMemory = buffer->platformObject();

    if (!hostPtr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (bufferOrigin.size() != 3 || hostOrigin.size() != 3 || region.size() != 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t> bufferOriginCopy, hostOriginCopy, regionCopy;
    bufferOriginCopy.appendVector(bufferOrigin);
    hostOriginCopy.appendVector(hostOrigin);
    regionCopy.appendVector(region);

    if (!WebCLInputChecker::isValidRegionForMemoryObject(bufferOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, buffer->sizeInBytes())
        || !WebCLInputChecker::isValidRegionForMemoryObject(hostOriginCopy, regionCopy, hostRowPitch, hostSlicePitch, hostPtrLength)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec, blockingWrite ? DoNotAcceptUserEvent : AcceptUserEvent);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueWriteBufferRect(computeMemory, blockingWrite, bufferOriginCopy,
        hostOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, hostPtr, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}

void WebCLCommandQueue::enqueueWriteBufferRect(WebCLBuffer* buffer, CCbool blockingWrite, const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin,
    const Vector<CCuint>& region, CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch,
    ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& eventWaitlist, WebCLEvent* event, ExceptionCode& ec)
{
    if (!ptr || !WebCLInputChecker::isValidDataSizeForArrayBufferView(hostRowPitch, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidDataSizeForArrayBufferView(hostSlicePitch, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    enqueueWriteBufferRectBase(buffer, blockingWrite, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch,
        ptr->baseAddress(), ptr->byteLength(), eventWaitlist, event, ec);
}

void WebCLCommandQueue::enqueueWriteBufferRect(WebCLBuffer* buffer, CCbool blockingWrite, const Vector<CCuint>& bufferOrigin,
    const Vector<CCuint>& hostOrigin, const Vector<CCuint>& region, CCuint bufferRowPitch, CCuint bufferSlicePitch,
    ImageData* srcPixels, const Vector<RefPtr<WebCLEvent> >& eventWaitlist, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t pixelSize = 0;
    WebCLHTMLInterop::extractDataFromImageData(srcPixels, hostPtr, pixelSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueWriteBufferRectBase(buffer, blockingWrite, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch, 0 /* hostRowPitch */, 0 /* hostSlicePitch */,
        hostPtr, pixelSize, eventWaitlist, event, ec);
}

void WebCLCommandQueue::enqueueWriteBufferRect(WebCLBuffer* buffer, CCbool blockingWrite, const Vector<CCuint>& bufferOrigin,
    const Vector<CCuint>& hostOrigin, const Vector<CCuint>& region, CCuint bufferRowPitch, CCuint bufferSlicePitch,
    HTMLCanvasElement* srcCanvas, const Vector<RefPtr<WebCLEvent> >& eventWaitlist, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(srcCanvas, hostPtr, canvasSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueWriteBufferRectBase(buffer, blockingWrite, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch,
        0 /* hostRowPitch */, 0 /* hostSlicePitch */, hostPtr, canvasSize, eventWaitlist, event, ec);
}

void WebCLCommandQueue::enqueueWriteBufferRect(WebCLBuffer* buffer, CCbool blockingWrite, const Vector<CCuint>& bufferOrigin,
    const Vector<CCuint>& hostOrigin, const Vector<CCuint>& region, CCuint bufferRowPitch, CCuint bufferSlicePitch,
    HTMLImageElement* srcImage, const Vector<RefPtr<WebCLEvent> >& eventWaitlist, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t imageSize = 0;
    WebCLHTMLInterop::extractDataFromImage(srcImage, hostPtr, imageSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueWriteBufferRectBase(buffer, blockingWrite, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch, 0 /* hostRowPitch */, 0 /*hostSlicePitch*/,
        hostPtr, imageSize, eventWaitlist, event, ec);
}

void WebCLCommandQueue::enqueueReadBufferBase(WebCLBuffer* buffer, CCbool blockingRead, CCuint offset, CCuint numBytes, void* hostPtr, size_t hostPtrLength,
    const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::compareContext(m_context.get(), buffer->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (hostPtrLength < numBytes
        || buffer->sizeInBytes() < (offset + numBytes)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    ComputeMemoryObject* computeMemory = buffer->platformObject();

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec, blockingRead ? DoNotAcceptUserEvent : AcceptUserEvent);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueReadBuffer(computeMemory, blockingRead, offset, numBytes, hostPtr, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, CCbool blockingRead, CCuint offset, CCuint numBytes,
    ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!ptr || !WebCLInputChecker::isValidDataSizeForArrayBufferView(numBytes, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    enqueueReadBufferBase(buffer, blockingRead, offset, numBytes, ptr->baseAddress(), ptr->byteLength(), events, event, ec);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, CCbool blockingRead, CCuint offset, CCuint numBytes,
    HTMLCanvasElement* dstCanvas, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }

    void* hostPtr = 0;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(dstCanvas, hostPtr, canvasSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueReadBufferBase(buffer, blockingRead, offset, numBytes, hostPtr, canvasSize, events, event, ec);
}

void WebCLCommandQueue::enqueueReadImageBase(WebCLImage* image, CCbool blockingRead, const Vector<CCuint>& origin, const Vector<CCuint>& region,
    CCuint hostRowPitch, void* hostPtr, size_t hostPtrLength, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::compareContext(m_context.get(), image->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(image)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    ComputeMemoryObject* computeMemory = image->platformObject();

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

    if (!WebCLInputChecker::isValidRegionForMemoryObject(originCopy, regionCopy, hostRowPitch, 0 /* hostSlicePitch */, image->sizeInBytes())
        || !WebCLInputChecker::isValidRegionForHostPtr(regionCopy, hostPtrLength)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec, blockingRead ? DoNotAcceptUserEvent : AcceptUserEvent);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueReadImage(computeMemory, blockingRead, originCopy, regionCopy, hostRowPitch, 0 /* slice_pitch */, hostPtr, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, CCbool blockingRead, const Vector<CCuint>& origin, const Vector<CCuint>& region,
    CCuint hostRowPitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!ptr || !WebCLInputChecker::isValidDataSizeForArrayBufferView(hostRowPitch, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    enqueueReadImageBase(image, blockingRead, origin, region, hostRowPitch, ptr->baseAddress(), ptr->byteLength(), events, event, ec);
}

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, CCbool blockingRead, const Vector<CCuint>& origin, const Vector<CCuint>& region,
    HTMLCanvasElement* dstCanvas, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(dstCanvas, hostPtr, canvasSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueReadImageBase(image, blockingRead, origin, region, 0 /* rowPitch */, hostPtr, canvasSize, events, event, ec);
}

void WebCLCommandQueue::enqueueReadBufferRectBase(WebCLBuffer* buffer, CCbool blockingRead,
    const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin, const Vector<CCuint>& region,
    CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch,
    void* hostPtr, size_t hostPtrLength, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::compareContext(m_context.get(), buffer->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(buffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    ComputeMemoryObject* computeMemory = buffer->platformObject();

    if (!hostPtr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (bufferOrigin.size() != 3 || hostOrigin.size() != 3 || region.size() != 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<size_t> bufferOriginCopy, hostOriginCopy, regionCopy;
    bufferOriginCopy.appendVector(bufferOrigin);
    hostOriginCopy.appendVector(hostOrigin);
    regionCopy.appendVector(region);

    if (!WebCLInputChecker::isValidRegionForMemoryObject(hostOriginCopy, regionCopy, hostRowPitch, hostSlicePitch, hostPtrLength)
        || !WebCLInputChecker::isValidRegionForMemoryObject(bufferOriginCopy, regionCopy, bufferRowPitch, bufferSlicePitch, buffer->sizeInBytes())) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec, blockingRead ? DoNotAcceptUserEvent : AcceptUserEvent);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueReadBufferRect(computeMemory, blockingRead, bufferOriginCopy, hostOriginCopy,
        regionCopy, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, hostPtr, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}

void WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, CCbool blockingRead,
    const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin, const Vector<CCuint>& region,
    CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch,
    ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!ptr || !WebCLInputChecker::isValidDataSizeForArrayBufferView(hostRowPitch, ptr)
        || !WebCLInputChecker::isValidDataSizeForArrayBufferView(hostSlicePitch, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    enqueueReadBufferRectBase(buffer, blockingRead, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch,
        hostRowPitch, hostSlicePitch, ptr->baseAddress(), ptr->byteLength(), events, event, ec);
}

void WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, CCbool blockingRead,
    const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin, const Vector<CCuint>& region,
    CCuint bufferRowPitch, CCuint bufferSlicePitch, HTMLCanvasElement* dstCanvas, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(dstCanvas, hostPtr, canvasSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueReadBufferRectBase(buffer, blockingRead, bufferOrigin, hostOrigin, region, bufferRowPitch, bufferSlicePitch,
        0 /* hostRowPitch */, 0 /* hostSlicePitch */, hostPtr, canvasSize, events, event, ec);
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

    if (!WebCLInputChecker::compareContext(m_context.get(), kernel->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }
    ComputeKernel* computeKernel = kernel->computeKernel();

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

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror computeContextError = platformObject()->enqueueNDRangeKernel(computeKernel,
        workDim, globalWorkOffsetCopy, globalWorkSizeCopy, localWorkSizeCopy, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}

void WebCLCommandQueue::enqueueWaitForEvents(const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
{
    if (!events.size()) {
        ec = WebCLException::INVALID_EVENT_WAIT_LIST;
        return;
    }

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror error = platformObject()->enqueueWaitForEvents(computeEvents);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
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

void WebCLCommandQueue::enqueueWriteImageBase(WebCLImage* image, CCbool blockingWrite, const Vector<unsigned>& origin, const Vector<unsigned>& region,
    CCuint hostRowPitch, void* hostPtr, const size_t hostPtrLength, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::compareContext(m_context.get(), image->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(image)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    ComputeMemoryObject* computeMemory = image->platformObject();

    if (!hostPtr) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (origin.size() != 2 || region.size() != 2) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    Vector<size_t> originCopy, regionCopy;
    originCopy.appendVector(origin);
    regionCopy.appendVector(region);

    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    originCopy.append(0);
    regionCopy.append(1);

    if (!WebCLInputChecker::isValidRegionForHostPtr(regionCopy, hostPtrLength)
        || !WebCLInputChecker::isValidRegionForMemoryObject(originCopy, regionCopy, hostRowPitch, 0 /*bufferSlicePitch */, image->sizeInBytes())) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec, blockingWrite ? DoNotAcceptUserEvent : AcceptUserEvent);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueWriteImage(computeMemory, blockingWrite, originCopy, regionCopy, hostRowPitch, 0 /* input_slice_pitch */, hostPtr, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, CCbool blockingWrite, const Vector<unsigned>& origin, const Vector<unsigned>& region,
    CCuint hostRowPitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!ptr || !WebCLInputChecker::isValidDataSizeForArrayBufferView(hostRowPitch, ptr)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    enqueueWriteImageBase(image, blockingWrite, origin, region, hostRowPitch, ptr->baseAddress(), ptr->byteLength(), events, event, ec);
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, CCbool blockingWrite, const Vector<unsigned>& origin, const Vector<unsigned>& region,
    ImageData* srcPixels, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t pixelSize = 0;
    WebCLHTMLInterop::extractDataFromImageData(srcPixels, hostPtr, pixelSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueWriteImageBase(image, blockingWrite, origin, region, 0 /* hostRowPitch */, hostPtr, pixelSize, events, event, ec);
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, CCbool blockingWrite, const Vector<unsigned>& origin, const Vector<unsigned>& region,
    HTMLCanvasElement* srcCanvas, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(srcCanvas, hostPtr, canvasSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueWriteImageBase(image, blockingWrite, origin, region, 0 /* hostRowPitch */, hostPtr, canvasSize, events, event, ec);
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, CCbool blockingWrite, const Vector<unsigned>& origin, const Vector<unsigned>& region,
    HTMLImageElement* srcImage , const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!isExtensionEnabled(m_context, "WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return;
    }
    void* hostPtr = 0;
    size_t imageSize = 0;
    WebCLHTMLInterop::extractDataFromImage(srcImage, hostPtr, imageSize, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    enqueueWriteImageBase(image, blockingWrite, origin, region, 0 /* hostRowPitch */, hostPtr, imageSize, events, event, ec);
}

void WebCLCommandQueue::enqueueCopyImage(WebCLImage* sourceImage, WebCLImage* targetImage, const Vector<unsigned>& sourceOrigin,
    const Vector<unsigned>& targetOrigin, const Vector<unsigned>& region, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::compareContext(m_context.get(), sourceImage->context())
        || !WebCLInputChecker::compareContext(m_context.get(), targetImage->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(sourceImage)
        || !WebCLInputChecker::validateWebCLObject(targetImage)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (!WebCLInputChecker::compareImageFormat(sourceImage->imageFormat(), targetImage->imageFormat())) {
        ec = WebCLException::IMAGE_FORMAT_MISMATCH;
        return;
    }

    if (sourceOrigin.size() != 2 || targetOrigin.size() != 2 || region.size() != 2) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidRegionForImage(sourceImage, sourceOrigin, region)
        || !WebCLInputChecker::isValidRegionForImage(targetImage, targetOrigin, region)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    ComputeMemoryObject* computeSourceImage = sourceImage->platformObject();
    ComputeMemoryObject* computeTargetImage = targetImage->platformObject();

    Vector<size_t> sourceOriginCopy, targetOriginCopy, regionCopy;
    sourceOriginCopy.appendVector(sourceOrigin);
    targetOriginCopy.appendVector(targetOrigin);
    regionCopy.appendVector(region);

    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    sourceOriginCopy.append(0);
    targetOriginCopy.append(0);
    regionCopy.append(1);

    if (WebCLInputChecker::isRegionOverlapping(sourceImage, targetImage, sourceOrigin, targetOrigin, region)) {
        ec = WebCLException::MEM_COPY_OVERLAP;
        return;
    }

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyImage(computeSourceImage, computeTargetImage, sourceOriginCopy, targetOriginCopy, regionCopy, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}

void WebCLCommandQueue::enqueueCopyImageToBuffer(WebCLImage* sourceImage, WebCLBuffer* targetBuffer, const Vector<unsigned>&
    sourceOrigin, const Vector<unsigned>& region, CCuint targetOffset, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (!WebCLInputChecker::compareContext(m_context.get(), sourceImage->context())
        || !WebCLInputChecker::compareContext(m_context.get(), targetBuffer->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(sourceImage)
        || !WebCLInputChecker::validateWebCLObject(targetBuffer)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (sourceOrigin.size() != 2 || region.size() != 2) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidRegionForBuffer(targetBuffer->sizeInBytes(), region, targetOffset, sourceImage->imageDescriptor())
        || !WebCLInputChecker::isValidRegionForImage(sourceImage, sourceOrigin, region)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    ComputeMemoryObject* computeSourceImage = sourceImage->platformObject();
    ComputeMemoryObject* ccTargetBuffer = targetBuffer->platformObject();

    Vector<size_t> sourceOriginCopy, regionCopy;
    sourceOriginCopy.appendVector(sourceOrigin);
    regionCopy.appendVector(region);

    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    sourceOriginCopy.append(0);
    regionCopy.append(1);

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyImageToBuffer(computeSourceImage, ccTargetBuffer,
        sourceOriginCopy, regionCopy, targetOffset, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
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

    if (!WebCLInputChecker::compareContext(m_context.get(), sourceBuffer->context())
        || !WebCLInputChecker::compareContext(m_context.get(), targetImage->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (targetOrigin.size() != 2 || region.size() != 2) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidRegionForBuffer(sourceBuffer->sizeInBytes(), region, sourceOffset, targetImage->imageDescriptor())
        || !WebCLInputChecker::isValidRegionForImage(targetImage, targetOrigin, region)) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    ComputeMemoryObject* ccSourceBuffer = sourceBuffer->platformObject();
    ComputeMemoryObject* computeTargetImage = targetImage->platformObject();

    Vector<size_t> targetOriginCopy, regionCopy;
    targetOriginCopy.appendVector(targetOrigin);
    regionCopy.appendVector(region);

    // No support for 3D-images, so set default values of 0 for all origin & region arrays at 3rd index.
    targetOriginCopy.append(0);
    regionCopy.append(1);

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyBufferToImage(ccSourceBuffer, computeTargetImage,
        sourceOffset, targetOriginCopy, regionCopy, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
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

    if (!WebCLInputChecker::compareContext(m_context.get(), sourceBuffer->context())
        || !WebCLInputChecker::compareContext(m_context.get(), targetBuffer->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    if (WebCLInputChecker::isRegionOverlapping(sourceBuffer, targetBuffer, sourceOffset, targetOffset, sizeInBytes)) {
        ec = WebCLException::MEM_COPY_OVERLAP;
        return;
    }

    if ((sourceOffset + sizeInBytes) > sourceBuffer->sizeInBytes()
        || (targetOffset + sizeInBytes) > targetBuffer->sizeInBytes()) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    ComputeMemoryObject* ccSourceBuffer = sourceBuffer->platformObject();
    ComputeMemoryObject* ccTargetBuffer = targetBuffer->platformObject();

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyBuffer(ccSourceBuffer, ccTargetBuffer,
        sourceOffset, targetOffset, sizeInBytes, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
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

    if (!WebCLInputChecker::compareContext(m_context.get(), sourceBuffer->context())
        || !WebCLInputChecker::compareContext(m_context.get(), targetBuffer->context())) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    ComputeMemoryObject* ccSourceBuffer = sourceBuffer->platformObject();
    ComputeMemoryObject* ccTargetBuffer = targetBuffer->platformObject();

    if (sourceOrigin.size() != 3 || targetOrigin.size() != 3 || region.size() != 3) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }
    size_t sourceOffset = sourceOrigin[2] * sourceSlicePitch + sourceOrigin[1] * sourceRowPitch + sourceOrigin[0];
    size_t targetOffset = targetOrigin[2] * targetSlicePitch + targetOrigin[1] * targetRowPitch + targetOrigin[0];
    size_t numBytes = region[2] * region[1] * region[0];
    if (WebCLInputChecker::isRegionOverlapping(sourceBuffer, targetBuffer, sourceOffset, targetOffset, numBytes)) {
        ec = WebCLException::MEM_COPY_OVERLAP;
        return;
    }

    Vector<size_t> sourceOriginCopy, targetOriginCopy, regionCopy;
    sourceOriginCopy.appendVector(sourceOrigin);
    targetOriginCopy.appendVector(targetOrigin);
    regionCopy.appendVector(region);

    if (!WebCLInputChecker::isValidRegionForMemoryObject(sourceOriginCopy, regionCopy, sourceRowPitch, sourceSlicePitch, sourceBuffer->sizeInBytes())
        || !WebCLInputChecker::isValidRegionForMemoryObject(targetOriginCopy, regionCopy, targetSlicePitch, targetRowPitch, targetBuffer->sizeInBytes())) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }


    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueCopyBufferRect(ccSourceBuffer, ccTargetBuffer,
        sourceOriginCopy, targetOriginCopy, regionCopy, sourceRowPitch, sourceSlicePitch, targetRowPitch, targetSlicePitch, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
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
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    CCerror computeContextError = platformObject()->enqueueMarker(computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::releasePlatformObjectImpl()
{
    delete platformObject();
}

bool WebCLCommandQueue::isExtensionEnabled(RefPtr<WebCLContext> context, const String& name)
{
    return context->isExtensionEnabled(name);
};

#if ENABLE(WEBGL)
void WebCLCommandQueue::enqueueAcquireGLObjects(const Vector<RefPtr<WebCLMemoryObject> >& memoryObjects, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    Vector<ComputeMemoryObject*> computeMemoryObjects;
    for (size_t i = 0; i < memoryObjects.size(); ++i) {
        if (!memoryObjects[i]->sharesGLResources()) {
            ec = WebCLException::INVALID_GL_OBJECT;
            return;
        }
        computeMemoryObjects.append(memoryObjects[i]->platformObject());
    }

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueAcquireGLObjects(computeMemoryObjects, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}

void WebCLCommandQueue::enqueueReleaseGLObjects(const Vector<RefPtr<WebCLMemoryObject> >& memoryObjects, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    Vector<ComputeMemoryObject*> computeMemoryObjects;
    for (size_t i = 0; i < memoryObjects.size(); ++i) {
        if (!memoryObjects[i]->sharesGLResources()) {
            ec = WebCLException::INVALID_GL_OBJECT;
            return;
        }
        computeMemoryObjects.append(memoryObjects[i]->platformObject());
    }

    Vector<ComputeEvent*> computeEvents;
    ccEventListFromWebCLEventList(events, computeEvents, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    ComputeEvent* computeEvent = computeEventFromWebCLEventIfApplicable(event, ec);
    if (ec != WebCLException::SUCCESS)
        return;

    CCerror err = platformObject()->enqueueReleaseGLObjects(computeMemoryObjects, computeEvents, computeEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    WebCLEvent::processCallbackRegisterQueueForEvent(event, ec);
}
#endif

} // namespace WebCore

#endif // ENABLE(WEBCL)
