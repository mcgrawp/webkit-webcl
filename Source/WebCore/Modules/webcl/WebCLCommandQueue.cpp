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
#include "WebCLEventList.h"
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

// wrap
PassRefPtr<WebCLCommandQueue> WebCLCommandQueue::create(WebCLContext* context, CCCommandQueue commandQueue)
{
    return adoptRef(new WebCLCommandQueue(context, commandQueue));
}

WebCLCommandQueue::WebCLCommandQueue(WebCLContext* context, CCCommandQueue commandQueue)
    : m_context(context)
    , m_ccCommandQueue(commandQueue)
{
}

WebCLGetInfo WebCLCommandQueue::getInfo(int paramName, ExceptionCode& ec)
{
    CCint err = 0; // FIXME: Switch to CCerror when it gets routed through ComputeContext.
    CCuint uint_units = 0;
    CCDeviceID ccDeviceID = 0;
    CCCommandQueueProperties ccCommandQueueProperties = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return WebCLGetInfo();
    }

    switch (paramName) {
    case WebCL::QUEUE_REFERENCE_COUNT:
        err = clGetCommandQueueInfo(m_ccCommandQueue, CL_QUEUE_REFERENCE_COUNT, sizeof(CCuint), &uint_units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uint_units));
        break;
    // FIXME: We should not create a WebCLContext here.
    /*
    case WebCL::QUEUE_CONTEXT:
        err = clGetCommandQueueInfo(m_ccCommandQueue, CL_QUEUE_CONTEXT, sizeof(cl_context), &cl_context_id, 0);
        if (err == CL_SUCCESS) {
            RefPtr<WebCLContext> contextObj = WebCLContext::create(m_context, cl_context_id);
            // FIXME: The the wrapping PassRefPtr below REALLY needed?
            return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
        break;
    */
    case WebCL::QUEUE_DEVICE:
        err = clGetCommandQueueInfo(m_ccCommandQueue, CL_QUEUE_DEVICE, sizeof(CCDeviceID), &ccDeviceID, 0);
        if (err == CL_SUCCESS) {
            RefPtr<WebCLDevice> deviceObj = WebCLDevice::create(ccDeviceID);
            // FIXME: The the wrapping PassRefPtr below REALLY needed?
            return WebCLGetInfo(PassRefPtr<WebCLDevice>(deviceObj));
        }
        break;
    case WebCL::QUEUE_PROPERTIES:
        err = clGetCommandQueueInfo(m_ccCommandQueue, CL_QUEUE_PROPERTIES, sizeof(CCCommandQueueProperties), &ccCommandQueueProperties, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(ccCommandQueueProperties));
        break;
    default:
        printf("Error: Unsupported Commans Queue Info type\n");
        ec = WebCLException::FAILURE;
        return WebCLGetInfo();
    }

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite,
    int offset, int bufferSize, ArrayBufferView* ptr, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccBuffer = 0;
    int eventsLength = 0;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (buffer) {
        ccBuffer = buffer->getCLBuffer();
        if (!ccBuffer) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error: ccBuffer null\n");
            return;
        }
    }

    if (!ptr) {
        printf("Error: Invalid Buffer\n");
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueWriteBuffer(m_ccCommandQueue, ccBuffer, blockingWrite, offset,
        bufferSize, ptr->baseAddress(), eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ImageData* ptr, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccBuffer = 0;
    int eventsLength = 0;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCL::FAILURE;
        return;
    }

    if (buffer) {
        ccBuffer = buffer->getCLBuffer();
        if (!ccBuffer) {
            printf("Error: ccBuffer null\n");
            return;
        }
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    unsigned char* bufferArray;
    if (ptr && ptr->data() && ptr->data()->data() && ptr->data()->data()) {
        bufferArray = ptr->data()->data();
        bufferSize =  ptr->data()->length();
    } else {
        printf("Error: Invalid ImageData\n");
        ec = WebCL::FAILURE;
        return;
    }

    CCerror err = m_context->computeContext()->enqueueWriteBuffer(m_ccCommandQueue, ccBuffer, blockingWrite, offset,
        bufferSize, bufferArray, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueWriteBufferRect(WebCLBuffer* buffer, bool blockingWrite, Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
    int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch, ArrayBufferView* ptr,
    WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccBuffer = 0;
    CCEvent ccEvent = 0;
    CCEvent* ccEventWaitList = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (buffer) {
        ccBuffer = buffer->getCLBuffer();
        if (!ccBuffer) {
            printf("Error: ccBuffer null\n");
            return;
        }
    }

    if (eventWaitList) {
        ccEventWaitList = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    if (bufferOrigin->length() < 3
        || hostOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::FAILURE;
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

    CCerror err = m_context->computeContext()->enqueueWriteBufferRect(m_ccCommandQueue, ccBuffer, blockingWrite, bufferOriginData.data(), hostOriginData.data(), regionData.data(), bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr->baseAddress(), eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ImageData* ptr, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccBuffer = 0;
    CCEvent ccEvent = 0;
    int eventsLength = 0;
    CCEvent* ccEventWaitList = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (buffer) {
        ccBuffer = buffer->getCLBuffer();
        if (!ccBuffer) {
            printf("Error: ccBuffer null\n");
            return;
        }
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    unsigned char* bufferArray;
    if (ptr && ptr->data() && ptr->data()->data() && ptr->data()->data()) {
        bufferArray = ptr->data()->data();
        bufferSize =  ptr->data()->length();
    } else {
        printf("Error: Invalid ImageData\n");
        ec = WebCL::FAILURE;
        return;
    }

    CCerror err = m_context->computeContext()->enqueueReadBuffer(m_ccCommandQueue, ccBuffer, blockingRead, offset,
        bufferSize, bufferArray, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, bool blockingRead, Int32Array* origin, Int32Array* region, int rowPitch, int slicePitch, ArrayBufferView* ptr, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccBuffer = 0;
    CCEvent ccEvent = 0;
    CCEvent* ccEventWaitList = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (image) {
        ccBuffer = image->getCLImage();
        if (!ccBuffer) {
            printf("Error: ccBuffer null\n");
            return;
        }
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    if (origin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::FAILURE;
        return;
    }

    Vector<size_t, 3> originData;
    Vector<size_t, 3> regionData;
    for (size_t i = 0; i < 3; ++i) {
        originData[i] = origin->item(i);
        regionData[i] = region->item(i);
    }

    CCerror err = m_context->computeContext()->enqueueReadImage(m_ccCommandQueue, ccBuffer, blockingRead, originData.data(), regionData.data(), rowPitch, slicePitch, ptr->baseAddress(), eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* bufferSrc, bool blockingRead, int offset, int bufferSize, ArrayBufferView* ptr, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccBuffer = 0;
    CCEvent ccEvent = 0;
    CCEvent* ccEventWaitList = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (bufferSrc) {
        ccBuffer = bufferSrc->getCLBuffer();
        if (!ccBuffer) {
            printf("Error: ccBuffer null\n");
            return;
        }
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueReadBuffer(m_ccCommandQueue, ccBuffer, blockingRead, offset, bufferSize,
        ptr->baseAddress(), eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, bool blockingRead,
    Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
    int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch,
    ArrayBufferView* ptr, WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccBuffer = 0;
    CCEvent ccEvent = 0;
    CCEvent* ccEventWaitList = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (buffer) {
        ccBuffer = buffer->getCLBuffer();
        if (!ccBuffer) {
            printf("Error: ccBuffer null\n");
            return;
        }
    }

    if (eventWaitList) {
        ccEventWaitList = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    if (bufferOrigin->length() < 3
        || hostOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::FAILURE;
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

    CCerror err = m_context->computeContext()->enqueueReadBufferRect(m_ccCommandQueue, ccBuffer, blockingRead, bufferOriginData.data(), hostOriginData.data(), regionData.data(), bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr->baseAddress(), eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* globalWorkOffsets,
    Int32Array* globalWorkSize, Int32Array* localWorkSize, WebCLEventList* events,
    WebCLEvent* event, ExceptionCode& ec)
{
    CCKernel ccKernel;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent;
    int eventsLength = 0;
    int workItemDimensions = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (kernel) {
        ccKernel = kernel->getCLKernel();
        if (!ccKernel) {
            ec = WebCLException::INVALID_KERNEL;
            printf("Error: ccKernel null\n");
            return;
        }
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    // FIXME: The block of code is repeated all over this class.
    Vector<size_t> globalWorkSizeCopy;
    if (globalWorkSize) {
        for (size_t i = 0; i < globalWorkSize->length(); ++i)
            globalWorkSizeCopy.append(globalWorkSize->item(i));
        workItemDimensions = globalWorkSize->length();
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

    CCerror computeContextError = m_context->computeContext()->enqueueNDRangeKernel(m_ccCommandQueue, ccKernel, workItemDimensions, globalWorkOffsetCopy.data(), globalWorkSizeCopy.data(), localWorkSizeCopy.data(), eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::finish(ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        printf("Error: Invalid Command Queue\n");
        return;
    }

    CCerror computeContextError = m_context->computeContext()->finishCommandQueue(m_ccCommandQueue);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}


void WebCLCommandQueue::flush(ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    CCerror computeContextError = m_context->computeContext()->flushCommandQueue(m_ccCommandQueue);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, bool blockingWrite, Int32Array* origin, Int32Array* region, int inputRowPitch, int inputSlicePitch, ArrayBufferView* ptr, WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccImage = 0;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (image) {
        ccImage = image->getCLImage();
        if (!ccImage) {
            printf("Error: ccImage null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (eventWaitList) {
        ccEventWaitList = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    if (origin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::FAILURE;
        return;
    }

    Vector<size_t, 3> originData;
    Vector<size_t, 3> regionData;
    for (size_t i = 0; i < 3; ++i) {
        originData[i] = origin->item(i);
        regionData[i] = region->item(i);
    }

    CCerror err = m_context->computeContext()->enqueueWriteImage(m_ccCommandQueue, ccImage, blockingWrite, originData.data(), regionData.data(), inputRowPitch, inputSlicePitch, ptr->baseAddress(), eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueAcquireGLObjects(WebCLMemoryObject* memoryObjects, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccMemoryObjectsIDs[1] = {0};
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (memoryObjects) {
        if (memoryObjects->isShared()) {
            ccMemoryObjectsIDs[0] = memoryObjects->getCLMemoryObject();
            if (!ccMemoryObjectsIDs[0]) {
                printf("Error: ccMemoryObjectsIDs null\n");
                ec = WebCLException::FAILURE;
                return;
            }
        } else {
            printf("Error:: memoryObjects is not shared with GL\n");
            ec = WebCLException::FAILURE;
            return;
        }
    } else {
        printf("Error: Memory objects passed is Null\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueAcquireGLObjects(m_ccCommandQueue, 1, ccMemoryObjectsIDs, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReleaseGLObjects(WebCLMemoryObject* memoryObjects, WebCLEventList* events,
    WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccMemoryObjectsIDs = 0;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (memoryObjects && memoryObjects->isShared()) {
        ccMemoryObjectsIDs = memoryObjects->getCLMemoryObject();
        if (!ccMemoryObjectsIDs) {
            printf("Error: ccMemoryObjectsIDs null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    CCerror error = m_context->computeContext()->enqueueReleaseGLObjects(m_ccCommandQueue, 1, &ccMemoryObjectsIDs, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
}
void WebCLCommandQueue::enqueueCopyImage(WebCLImage* sourceImage, WebCLImage* targetImage, Int32Array* sourceOrigin, Int32Array* targetOrigin, Int32Array* region,
    WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccSourceImage = 0;
    PlatformComputeObject ccTargetImage = 0;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (sourceImage) {
        ccSourceImage = sourceImage->getCLImage();
        if (!ccSourceImage) {
            printf("Error: ccSourceImage null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (targetImage) {
        ccTargetImage = targetImage->getCLImage();
        if (!ccTargetImage) {
            printf("Error: ccTargetImage null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    if (sourceOrigin->length() < 3
        || targetOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::FAILURE;
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

    CCerror err = m_context->computeContext()->enqueueCopyImage(m_ccCommandQueue, ccSourceImage, ccTargetImage, sourceOriginData.data(), targetOriginData.data(), regionData.data(), eventsLength, ccEventWaitList, &ccEvent);
    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyImageToBuffer(WebCLImage *sourceImage, WebCLBuffer *targetBuffer, Int32Array* sourceOrigin, Int32Array* region, int targetOffset,
    WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccSourceImage = 0;
    PlatformComputeObject ccTargetBuffer = 0;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (sourceImage) {
        ccSourceImage = sourceImage->getCLImage();
        if (!ccSourceImage) {
            printf("Error: ccSourceImage null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (targetBuffer) {
        ccTargetBuffer = targetBuffer->getCLBuffer();
        if (!ccTargetBuffer) {
            printf("Error: ccTargetBuffer null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    if (eventWaitList) {
        ccEventWaitList = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    if (sourceOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::FAILURE;
        return;
    }

    Vector<size_t, 3> sourceOriginData;
    Vector<size_t, 3> regionData;

    for (size_t i = 0; i < 3; ++i) {
        sourceOriginData[i] = sourceOrigin->item(i);
        regionData[i] = region->item(i);
    }

    CCerror err = m_context->computeContext()->enqueueCopyImageToBuffer(m_ccCommandQueue, ccSourceImage, ccTargetBuffer, sourceOriginData.data(), regionData.data(), targetOffset, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBufferToImage(WebCLBuffer *sourceBuffer, WebCLImage *targetImage, int sourceOffset, Int32Array* targetOrigin, Int32Array* region,
    WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccSourceBuffer = 0;
    PlatformComputeObject ccTargetImage = 0;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (sourceBuffer) {
        ccSourceBuffer = sourceBuffer->getCLBuffer();
        if (!ccSourceBuffer) {
            printf("Error:  ccSourceBuffer null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (targetImage) {
        ccTargetImage = targetImage->getCLImage();
        if (!ccTargetImage) {
            printf("Error:  ccTargetImage null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    if (eventWaitList) {
        ccEventWaitList = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    if (targetOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::FAILURE;
        return;
    }

    Vector<size_t, 3> targetOriginData;
    Vector<size_t, 3> regionData;

    for (size_t i = 0; i < 3; ++i) {
        targetOriginData[i] = targetOrigin->item(i);
        regionData[i] = region->item(i);
    }

    if (event)
        ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyBufferToImage(m_ccCommandQueue, ccSourceBuffer, ccTargetImage, sourceOffset, targetOriginData.data(), regionData.data(), eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBuffer(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, int sourceOffset, int targetOffset, int sizeInBytes, WebCLEventList* events,
    WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccSourceBuffer = 0;
    PlatformComputeObject ccTargetBuffer = 0;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (sourceBuffer) {
        ccSourceBuffer = sourceBuffer->getCLBuffer();
        if (!ccSourceBuffer) {
            printf("Error: ccSourceBuffer null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (targetBuffer) {
        ccTargetBuffer = targetBuffer->getCLBuffer();
        if (!ccTargetBuffer) {
            printf("Error: ccTargetBuffer null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    if (events) {
        ccEventWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyBuffer(m_ccCommandQueue, ccSourceBuffer, ccTargetBuffer,
        sourceOffset, targetOffset, sizeInBytes, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBufferRect(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, Int32Array* sourceOrigin, Int32Array* targetOrigin, Int32Array* region, int sourceRowPitch, int sourceSlicePitch, int targetRowPitch, int targetSlicePitch, WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    PlatformComputeObject ccSourceBuffer = 0;
    PlatformComputeObject ccTargetBuffer = 0;
    CCEvent* ccEventWaitList = 0;
    CCEvent ccEvent = 0;
    int eventsLength = 0;

    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (sourceBuffer) {
        ccSourceBuffer = sourceBuffer->getCLBuffer();
        if (!ccSourceBuffer) {
            printf("Error: ccSourceBuffer null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (targetBuffer) {
        ccTargetBuffer = targetBuffer->getCLBuffer();
        if (!ccTargetBuffer) {
            printf("Error: ccTargetBuffer null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    if (eventWaitList) {
        ccEventWaitList = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        ccEvent = event->getCLEvent();

    if (sourceOrigin->length() < 3
        || targetOrigin->length() < 3
        || region->length() < 3) {
        ec = WebCLException::FAILURE;
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

    CCerror err = m_context->computeContext()->enqueueCopyBufferRect(m_ccCommandQueue, ccSourceBuffer, ccTargetBuffer, sourceOriginData.data(), targetOriginData.data(), regionData.data(), sourceRowPitch, sourceSlicePitch, targetRowPitch, targetSlicePitch, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueBarrier(WebCLEventList* eventsWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCEvent* ccEventWaitList = 0;
    size_t eventsLength = 0;
    if (eventsWaitList) {
        ccEventWaitList = eventsWaitList->getCLEvents();
        eventsLength = eventsWaitList->length();
    }

    CCEvent ccEvent = 0;
    if (event) {
        ccEvent = event->getCLEvent();
        if (!ccEvent) {
            printf("ccEvent null\n");
            ec = WebCLException::INVALID_EVENT;
            return;
        }
    }

    CCerror computeContextError = m_context->computeContext()->enqueueBarrier(m_ccCommandQueue, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueMarker(WebCLEventList* eventsWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCEvent ccEvent = 0;
    if (event) {
        ccEvent = event->getCLEvent();
        if (!ccEvent) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
    }

    CCEvent* ccEventWaitList = 0;
    size_t eventsLength = 0;
    if (eventsWaitList) {
        ccEventWaitList = eventsWaitList->getCLEvents();
        eventsLength = eventsWaitList->length();
    }

    CCerror computeContextError = m_context->computeContext()->enqueueMarker(m_ccCommandQueue, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueTask(WebCLKernel* kernel, WebCLEventList* eventsWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_ccCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    CCKernel ccKernel = 0;
    if (kernel) {
        ccKernel = kernel->getCLKernel();
        if (!ccKernel) {
            printf("Error: ccKernel null\n");
            ec = WebCLException::INVALID_KERNEL;
            return;
        }
    }

    CCEvent* ccEventWaitList = 0;
    size_t eventsLength = 0;
    if (eventsWaitList) {
        ccEventWaitList = eventsWaitList->getCLEvents();
        eventsLength = eventsWaitList->length();
    }

    CCEvent ccEvent = 0;
    if (event) {
        ccEvent = event->getCLEvent();
        if (!ccEvent) {
            printf("ccEvent null\n");
            ec = WebCLException::INVALID_EVENT;
            return;
        }
    }

    CCerror err = m_context->computeContext()->enqueueTask(m_ccCommandQueue, ccKernel, eventsLength, ccEventWaitList, &ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

CCCommandQueue WebCLCommandQueue::getCLCommandQueue()
{
    return m_ccCommandQueue;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
