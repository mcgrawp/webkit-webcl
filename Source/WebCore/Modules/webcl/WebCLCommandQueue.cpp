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
    ASSERT(m_clCommandQueue);
    CCerror computeContextErrorCode = m_context->computeContext()->releaseCommandQueue(m_clCommandQueue);

    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
    m_clCommandQueue = 0;
}

PassRefPtr<WebCLCommandQueue> WebCLCommandQueue::create(WebCLContext* context, cl_command_queue commandQueue)
{
    return adoptRef(new WebCLCommandQueue(context, commandQueue));
}

WebCLCommandQueue::WebCLCommandQueue(WebCLContext* context, cl_command_queue commandQueue)
    : m_context(context)
    , m_clCommandQueue(commandQueue)
{
}

WebCLGetInfo WebCLCommandQueue::getInfo(int paramName, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_uint uint_units = 0;
    cl_device_id clDevice = 0;
    cl_command_queue_properties queueProperties = 0;
    RefPtr<WebCLContext> contextObj = 0;
    RefPtr<WebCLDevice> deviceObj = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return WebCLGetInfo();
    }

    switch (paramName) {
    case WebCL::QUEUE_REFERENCE_COUNT:
        err = clGetCommandQueueInfo(m_clCommandQueue, CL_QUEUE_REFERENCE_COUNT, sizeof(cl_uint), &uint_units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uint_units));
        break;
    // FIXME: We should not create a WebCLContext here.
    /*
    case WebCL::QUEUE_CONTEXT:
        err = clGetCommandQueueInfo(m_clCommandQueue, CL_QUEUE_CONTEXT, sizeof(cl_context), &cl_context_id, 0);
        contextObj = WebCLContext::create(m_context, cl_context_id);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
        break;
    */
    case WebCL::QUEUE_DEVICE:
        err = clGetCommandQueueInfo(m_clCommandQueue, CL_QUEUE_DEVICE, sizeof(cl_device_id), &clDevice, 0);
        if (err == CL_SUCCESS) {
            deviceObj = WebCLDevice::create(clDevice);
            return WebCLGetInfo(PassRefPtr<WebCLDevice>(deviceObj));
        }
        break;
    case WebCL::QUEUE_PROPERTIES:
        err = clGetCommandQueueInfo(m_clCommandQueue, CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &queueProperties, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(queueProperties));
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
    cl_mem clMemID = 0;
    int eventsLength = 0;
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (buffer) {
        clMemID = buffer->getCLBuffer();
        if (!clMemID) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error: clMemID null\n");
            return;
        }
    }

    if (!ptr) {
        printf("Error: Invalid Buffer\n");
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (events) {
        clEventWaitLists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueWriteBuffer(m_clCommandQueue, clMemID, blockingWrite, offset,
        bufferSize, ptr->baseAddress(), eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ImageData* ptr, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clMemID = 0;
    int eventsLength = 0;
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCL::FAILURE;
        return;
    }

    if (buffer) {
        clMemID = buffer->getCLBuffer();
        if (!clMemID) {
            printf("Error: clMemID null\n");
            return;
        }
    }

    if (events) {
        clEventWaitLists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    unsigned char* bufferArray;
    if (ptr && ptr->data() && ptr->data()->data() && ptr->data()->data()) {
        bufferArray = ptr->data()->data();
        bufferSize =  ptr->data()->length();
    } else {
        printf("Error: Invalid ImageData\n");
        ec = WebCL::FAILURE;
        return;
    }

    CCerror err = m_context->computeContext()->enqueueWriteBuffer(m_clCommandQueue, clMemID, blockingWrite, offset,
        bufferSize, bufferArray, eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueWriteBufferRect(WebCLBuffer* buffer, bool blockingWrite, Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
    int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch, ArrayBufferView* ptr,
    WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clMemID = 0;
    cl_event clEventID = 0;
    cl_event* clEventWaitLists = 0;
    int eventsLength = 0;
    size_t* bufferOriginArray = 0;
    size_t* hostOriginArray = 0;
    size_t* regionArray = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (buffer) {
        clMemID = buffer->getCLBuffer();
        if (!clMemID) {
            printf("Error: clMemID null\n");
            return;
        }
    }

    if (eventWaitList) {
        clEventWaitLists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    unsigned bufferOriginArrayLength = bufferOrigin->length();
    bufferOriginArray = (size_t*) malloc(bufferOriginArrayLength * sizeof(size_t));
    if (!bufferOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < bufferOriginArrayLength; ++i)
        bufferOriginArray[i] = bufferOrigin->item(i);

    unsigned hostOriginArrayLength = hostOrigin->length();
    hostOriginArray = (size_t*) malloc(hostOriginArrayLength * sizeof(size_t));
    if (!hostOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < hostOriginArrayLength; ++i)
        hostOriginArray[i] = hostOrigin->item(i);

    unsigned regionArrayLength = region->length();
    regionArray = (size_t*) malloc(regionArrayLength * sizeof(size_t));
    if (!regionArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < regionArrayLength; ++i)
        regionArray[i] = region->item(i);

    CCerror err = m_context->computeContext()->enqueueWriteBufferRect(m_clCommandQueue, clMemID, blockingWrite, bufferOriginArray,
        hostOriginArray, regionArray, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr->baseAddress(), eventsLength,
        clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ImageData* ptr, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clMemID = 0;
    cl_event clEventID = 0;
    int eventsLength = 0;
    cl_event* clEventWaitLists = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (buffer) {
        clMemID = buffer->getCLBuffer();
        if (!clMemID) {
            printf("Error: clMemID null\n");
            return;
        }
    }

    if (events) {
        clEventWaitLists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    unsigned char* bufferArray;
    if (ptr && ptr->data() && ptr->data()->data() && ptr->data()->data()) {
        bufferArray = ptr->data()->data();
        bufferSize =  ptr->data()->length();
    } else {
        printf("Error: Invalid ImageData\n");
        ec = WebCL::FAILURE;
        return;
    }

    CCerror err = m_context->computeContext()->enqueueReadBuffer(m_clCommandQueue, clMemID, blockingRead, offset,
        bufferSize, bufferArray, eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, bool blockingRead, Int32Array* origin, Int32Array* region, int rowPitch, int slicePitch,
    ArrayBufferView* ptr, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clMemID = 0;
    cl_event clEventID = 0;
    cl_event* clEventWaitLists = 0;
    int eventsLength = 0;
    size_t* originArray = 0;
    size_t* regionArray = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (image) {
        clMemID = image->getCLImage();
        if (!clMemID) {
            printf("Error: clMemID null\n");
            return;
        }
    }

    if (events) {
        clEventWaitLists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    unsigned originArrayLength = origin->length();
    originArray = (size_t*) malloc(originArrayLength * sizeof(size_t));
    if (!originArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < originArrayLength; ++i)
        originArray[i] = origin->item(i);

    unsigned regionArrayLength = region->length();
    regionArray = (size_t*) malloc(regionArrayLength * sizeof(size_t));
    if (!regionArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < regionArrayLength; ++i)
        regionArray[i] = region->item(i);

    // FIXME: '0' parameters need to be addressed later.
    CCerror err = m_context->computeContext()->enqueueReadImage(m_clCommandQueue, clMemID, blockingRead, originArray,
        regionArray, rowPitch, slicePitch, ptr->baseAddress(), eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* bufferSrc, bool blockingRead, int offset, int bufferSize, ArrayBufferView* ptr, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clMemID = 0;
    cl_event clEventID = 0;
    cl_event* clEventWaitLists = 0;
    int eventsLength = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (bufferSrc) {
        clMemID = bufferSrc->getCLBuffer();
        if (!clMemID) {
            printf("Error: clMemID null\n");
            return;
        }
    }

    if (events) {
        clEventWaitLists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueReadBuffer(m_clCommandQueue, clMemID, blockingRead, offset, bufferSize,
        ptr->baseAddress(), eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, bool blockingRead,
    Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
    int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch,
    ArrayBufferView* ptr, WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clMemID = 0;
    cl_event clEventID = 0;
    cl_event* clEventWaitLists = 0;
    int eventsLength = 0;
    size_t* bufferOriginArray = 0;
    size_t* hostOriginArray = 0;
    size_t* regionArray = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    if (buffer) {
        clMemID = buffer->getCLBuffer();
        if (!clMemID) {
            printf("Error: clMemID null\n");
            return;
        }
    }

    if (eventWaitList) {
        clEventWaitLists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    unsigned bufferOriginArrayLength = bufferOrigin->length();
    bufferOriginArray = (size_t*) malloc(bufferOriginArrayLength * sizeof(size_t));
    if (!bufferOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < bufferOriginArrayLength; ++i)
        bufferOriginArray[i] = bufferOrigin->item(i);

    unsigned hostOriginArrayLength = hostOrigin->length();
    hostOriginArray = (size_t*) malloc(hostOriginArrayLength * sizeof(size_t));
    if (!hostOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < hostOriginArrayLength; ++i)
        hostOriginArray[i] = hostOrigin->item(i);

    unsigned regionArrayLength = region->length();
    regionArray = (size_t*) malloc(regionArrayLength * sizeof(size_t));
    if (!regionArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < regionArrayLength; ++i)
        regionArray[i] = region->item(i);

    CCerror err = m_context->computeContext()->enqueueReadBufferRect(m_clCommandQueue, clMemID, blockingRead, bufferOriginArray,
        hostOriginArray, regionArray, bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr->baseAddress(),
        eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* globalWorkOffsets,
    Int32Array* globalWorkSize, Int32Array* localWorkSize, WebCLEventList* events,
    WebCLEvent* event, ExceptionCode& ec)
{
    cl_kernel clKernelID;
    cl_event* clEventsWaitList = 0;
    cl_event clEventID;
    int eventsLength = 0;

    size_t* globalWorkSizeCopy = 0;
    size_t* localWorkSizeCopy = 0;
    size_t* globalWorkOffsetCopy = 0;
    int workItemDimensions = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (kernel) {
        clKernelID = kernel->getCLKernel();
        if (!clKernelID) {
            ec = WebCLException::INVALID_KERNEL;
            printf("Error: clKernelID null\n");
            return;
        }
    }

    if (events) {
        clEventsWaitList = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    // FIXME: The block of code is repeated all over this class.
    if (globalWorkSize) {
        globalWorkSizeCopy = (size_t*) malloc(globalWorkSize->length() * sizeof(size_t));
        if (!globalWorkSizeCopy) {
            printf("Error: Error allocating memory");
            return;
        }
        for (unsigned i = 0; i < globalWorkSize->length(); ++i)
            globalWorkSizeCopy[i] = globalWorkSize->item(i);
        workItemDimensions = globalWorkSize->length();
    }

    if (localWorkSize) {
        localWorkSizeCopy = (size_t*) malloc(localWorkSize->length() * sizeof(size_t));
        if (!localWorkSizeCopy) {
            printf("Error: Error allocating memory");
            return;
        }
        for (unsigned i = 0; i < localWorkSize->length(); ++i)
            localWorkSizeCopy[i] = localWorkSize->item(i);
    }

    if (globalWorkOffsets) {
        globalWorkOffsetCopy = (size_t*) malloc(globalWorkOffsets->length() * sizeof(size_t));
        if (!globalWorkOffsetCopy) {
            printf("Error: Error allocating memory");
            return;
        }
        for (unsigned i = 0; i < globalWorkOffsets->length(); ++i)
            globalWorkOffsetCopy[i] = globalWorkOffsets->item(i);
    }

    CCerror computeContextError = m_context->computeContext()->enqueueNDRangeKernel(m_clCommandQueue, clKernelID, workItemDimensions,
        globalWorkOffsetCopy, globalWorkSizeCopy, localWorkSizeCopy, eventsLength, clEventsWaitList, &clEventID);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);

    free(globalWorkSizeCopy);
    globalWorkSizeCopy = 0;
    free(localWorkSizeCopy);
    localWorkSizeCopy = 0;
    free(globalWorkOffsetCopy);
    globalWorkOffsetCopy = 0;
}

void WebCLCommandQueue::finish(ExceptionCode& ec)
{
    if (!m_clCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        printf("Error: Invalid Command Queue\n");
        return;
    }

    CCerror computeContextError = m_context->computeContext()->finishCommandQueue(m_clCommandQueue);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}


void WebCLCommandQueue::flush(ExceptionCode& ec)
{
    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }

    CCerror computeContextError = m_context->computeContext()->flushCommandQueue(m_clCommandQueue);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image, bool blockingWrite, Int32Array* origin, Int32Array* region, int inputRowPitch, int inputSlicePitch, ArrayBufferView* ptr, WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clImageID = 0;
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;
    int eventsLength = 0;
    size_t* originArray = 0;
    size_t* regionArray = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (image) {
        clImageID = image->getCLImage();
        if (!clImageID) {
            printf("Error: clImageID null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    unsigned originArrayLength = origin->length();
    originArray = (size_t*) malloc(originArrayLength * sizeof(size_t));
    if (!originArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < originArrayLength; ++i)
        originArray[i] = origin->item(i);

    unsigned regionArrayLength = region->length();
    regionArray = (size_t*) malloc(regionArrayLength * sizeof(size_t));
    if (!regionArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < regionArrayLength; ++i)
        regionArray[i] = region->item(i);

    if (eventWaitList) {
        clEventWaitLists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueWriteImage(m_clCommandQueue, clImageID, blockingWrite, originArray, regionArray,
        inputRowPitch, inputSlicePitch, ptr->baseAddress(), eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueAcquireGLObjects(WebCLMemoryObject* memoryObjects, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clMemIDs[1] = {0};
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;
    int eventsLength = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (memoryObjects) {
        if (memoryObjects->isShared()) {
            clMemIDs[0] = memoryObjects->getCLMemoryObject();
            if (!clMemIDs[0]) {
                printf("Error: clMemIDs null\n");
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
        clEventWaitLists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueAcquireGLObjects(m_clCommandQueue, 1, clMemIDs, eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueReleaseGLObjects(WebCLMemoryObject* memoryObjects, WebCLEventList* events,
    WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clMemIDs = 0;
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;
    int eventsLength = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (memoryObjects && memoryObjects->isShared()) {
        clMemIDs = memoryObjects->getCLMemoryObject();
        if (!clMemIDs) {
            printf("Error: clMemIDs null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (events) {
        clEventWaitLists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror error = m_context->computeContext()->enqueueReleaseGLObjects(m_clCommandQueue, 1, &clMemIDs, eventsLength, clEventWaitLists, &clEventID);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return;
    }
}
void WebCLCommandQueue::enqueueCopyImage(WebCLImage* srcImage, WebCLImage* dstImage, Int32Array* srcOrigin, Int32Array* dstOrigin, Int32Array* region,
    WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clSrcImageID = 0;
    cl_mem clDstImageID = 0;
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;
    int eventsLength = 0;

    size_t* srcOriginArray = 0;
    size_t* dstOriginArray = 0;
    size_t* regionArray = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (srcImage) {
        clSrcImageID = srcImage->getCLImage();
        if (!clSrcImageID) {
            printf("Error: clSrcImageID null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (dstImage) {
        clDstImageID = dstImage->getCLImage();
        if (!clDstImageID) {
            printf("Error: clDstImageID null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    unsigned srcOriginArrayLength = srcOrigin->length();
    srcOriginArray = (size_t*) malloc(srcOriginArrayLength * sizeof(size_t));
    if (!srcOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < srcOriginArrayLength; ++i)
        srcOriginArray[i] = srcOrigin->item(i);

    unsigned dstOriginArrayLength = dstOrigin->length();
    dstOriginArray = (size_t*) malloc(dstOriginArrayLength * sizeof(size_t));
    if (!dstOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < dstOriginArrayLength; ++i)
        dstOriginArray[i] = dstOrigin->item(i);

    unsigned regionArrayLength = region->length();
    regionArray = (size_t*) malloc(regionArrayLength * sizeof(size_t));
    if (!regionArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < regionArrayLength; ++i)
        regionArray[i] = region->item(i);

    if (events) {
        clEventWaitLists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror err = clEnqueueCopyImage(m_clCommandQueue, clSrcImageID, clDstImageID,
        srcOriginArray, dstOriginArray, regionArray, eventsLength, clEventWaitLists, &clEventID);
    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyImageToBuffer(WebCLImage *srcImage, WebCLBuffer *dstBuffer, Int32Array* srcOrigin, Int32Array* region, int dstOffset,
    WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clSrcImageID = 0;
    cl_mem clDstBufferID = 0;
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;
    int eventsLength = 0;
    size_t* srcOriginArray = 0;
    size_t* regionArray = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (srcImage) {
        clSrcImageID = srcImage->getCLImage();
        if (!clSrcImageID) {
            printf("Error: clSrcImageID null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (dstBuffer) {
        clDstBufferID = dstBuffer->getCLBuffer();
        if (!clDstBufferID) {
            printf("Error: clDstBufferID null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    unsigned srcOriginArrayLength = srcOrigin->length();
    srcOriginArray = (size_t*) malloc(srcOriginArrayLength * sizeof(size_t));
    if (!srcOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < srcOriginArrayLength; ++i)
        srcOriginArray[i] = srcOrigin->item(i);

    unsigned regionArrayLength = region->length();
    regionArray = (size_t*)malloc(regionArrayLength*sizeof(size_t));
    if (!regionArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < regionArrayLength; ++i)
        regionArray[i] = region->item(i);

    if (eventWaitList) {
        clEventWaitLists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyImageToBuffer(m_clCommandQueue, clSrcImageID, clDstBufferID,
        srcOriginArray, regionArray, dstOffset, eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBufferToImage(WebCLBuffer *srcBuffer, WebCLImage *dstImage, int srcOffset, Int32Array* dstOrigin, Int32Array* region,
    WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clSrcBufferID = 0;
    cl_mem clDstImageID = 0;
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;
    int eventsLength = 0;
    size_t* dstOriginArray = 0;
    size_t* regionArray = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (srcBuffer) {
        clSrcBufferID = srcBuffer->getCLBuffer();
        if (!clSrcBufferID) {
            printf("Error:  clSrcBufferID null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (dstImage) {
        clDstImageID = dstImage->getCLImage();
        if (!clDstImageID) {
            printf("Error:  clDstImageID null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    unsigned dstOriginArrayLength = dstOrigin->length();
    dstOriginArray = (size_t*) malloc(dstOriginArrayLength * sizeof(size_t));
    if (!dstOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }

    for (unsigned i = 0; i < dstOriginArrayLength; ++i)
        dstOriginArray[i] = dstOrigin->item(i);

    unsigned regionArrayLength = region->length();
    regionArray = (size_t*) malloc(regionArrayLength * sizeof(size_t));
    if (!regionArray) {
        printf("Error: Error allocating memory");
        return;
    }

    for (unsigned i = 0; i < regionArrayLength; ++i)
        regionArray[i] = region->item(i);

    if (eventWaitList) {
        clEventWaitLists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyBufferToImage(m_clCommandQueue, clSrcBufferID, clDstImageID, srcOffset,
        dstOriginArray, regionArray, eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueCopyBuffer(WebCLBuffer* srcBuffer, WebCLBuffer* dstBuffer, int srcOffset, int dstOffset, int sizeInBytes, WebCLEventList* events,
    WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clSrcBufferID = 0;
    cl_mem clDstBufferID = 0;
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;
    int eventsLength = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (srcBuffer) {
        clSrcBufferID = srcBuffer->getCLBuffer();
        if (!clSrcBufferID) {
            printf("Error: clSrcBufferID null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (dstBuffer) {
        clDstBufferID = dstBuffer->getCLBuffer();
        if (!clDstBufferID) {
            printf("Error: clDstBufferID null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    if (events) {
        clEventWaitLists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyBuffer(m_clCommandQueue, clSrcBufferID, clDstBufferID,
        srcOffset, dstOffset, sizeInBytes, eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}
void WebCLCommandQueue::enqueueCopyBufferRect(WebCLBuffer* srcBuffer, WebCLBuffer* dstBuffer, Int32Array* srcOrigin, Int32Array* dstOrigin, Int32Array* region, int srcRowPitch, int srcSlicePitch, int dstRowPitch, int dstSlicePitch, WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem clSrcBufferID = 0;
    cl_mem clDstBufferID = 0;
    cl_event* clEventWaitLists = 0;
    cl_event clEventID = 0;
    int eventsLength = 0;
    size_t* srcOriginArray = 0;
    size_t* dstOriginArray = 0;
    size_t* regionArray = 0;

    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (srcBuffer) {
        clSrcBufferID = srcBuffer->getCLBuffer();
        if (!clSrcBufferID) {
            printf("Error: clSrcBufferID null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }

    if (dstBuffer) {
        clDstBufferID = dstBuffer->getCLBuffer();
        if (!clDstBufferID) {
            printf("Error: clDstBufferID null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    unsigned srcOriginArrayLength = srcOrigin->length();
    srcOriginArray = (size_t*) malloc(srcOriginArrayLength * sizeof(size_t));
    if (!srcOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < srcOriginArrayLength; ++i)
        srcOriginArray[i] = srcOrigin->item(i);

    unsigned dstOriginArrayLength = dstOrigin->length();
    dstOriginArray = (size_t*) malloc(dstOriginArrayLength * sizeof(size_t));
    if (!dstOriginArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < dstOriginArrayLength; ++i)
        dstOriginArray[i] = dstOrigin->item(i);

    unsigned regionArrayLength = region->length();
    regionArray = (size_t*) malloc(regionArrayLength * sizeof(size_t));
    if (!regionArray) {
        printf("Error: Error allocating memory");
        return;
    }
    for (unsigned i = 0; i < regionArrayLength; ++i)
        regionArray[i] = region->item(i);

    if (eventWaitList) {
        clEventWaitLists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }

    if (event)
        clEventID = event->getCLEvent();

    CCerror err = m_context->computeContext()->enqueueCopyBufferRect(m_clCommandQueue, clSrcBufferID, clDstBufferID,
        srcOriginArray, dstOriginArray, regionArray, srcRowPitch, srcSlicePitch, dstRowPitch, dstSlicePitch,
        eventsLength, clEventWaitLists, &clEventID);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLCommandQueue::enqueueBarrier(WebCLEventList* eventsWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    cl_event* clEventsWaitList = 0;
    size_t eventsLength = 0;
    if (eventsWaitList) {
        clEventsWaitList = eventsWaitList->getCLEvents();
        eventsLength = eventsWaitList->length();
    }

    cl_event clEventID = 0;
    if (event) {
        clEventID = event->getCLEvent();
        if (!clEventID) {
            printf("clEventID null\n");
            ec = WebCLException::INVALID_EVENT;
            return;
        }
    }

    CCerror computeContextError = m_context->computeContext()->enqueueBarrier(m_clCommandQueue, eventsLength, clEventsWaitList, &clEventID);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueMarker(WebCLEventList* eventsWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_clCommandQueue) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    cl_event clEventID = 0;
    if (event) {
        clEventID = event->getCLEvent();
        if (!clEventID) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }
    }

    cl_event* clEventsWaitList = 0;
    size_t eventsLength = 0;
    if (eventsWaitList) {
        clEventsWaitList = eventsWaitList->getCLEvents();
        eventsLength = eventsWaitList->length();
    }

    CCerror computeContextError = m_context->computeContext()->enqueueMarker(m_clCommandQueue, eventsLength, clEventsWaitList, &clEventID);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);
}

void WebCLCommandQueue::enqueueTask(WebCLKernel* kernel, WebCLEventList* eventsWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    if (!m_clCommandQueue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    cl_kernel clKernelID = 0;
    if (kernel) {
        clKernelID = kernel->getCLKernel();
        if (!clKernelID) {
            printf("Error: clKernelID null\n");
            ec = WebCLException::INVALID_KERNEL;
            return;
        }
    }

    cl_event* clEventsWaitList = 0;
    size_t eventsLength = 0;
    if (eventsWaitList) {
        clEventsWaitList = eventsWaitList->getCLEvents();
        eventsLength = eventsWaitList->length();
    }

    cl_event clEventID = 0;
    if (event) {
        clEventID = event->getCLEvent();
        if (!clEventID) {
            printf("clEventID null\n");
            ec = WebCLException::INVALID_EVENT;
            return;
        }
    }

    CCerror err = m_context->computeContext()->enqueueTask(m_clCommandQueue, clKernelID, eventsLength, clEventsWaitList, &clEventID);
    if (err != ComputeContext::SUCCESS) {
        printf("Error: clEnqueueTask\n");
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
        return;
    }
}

cl_command_queue WebCLCommandQueue::getCLCommandQueue()
{
    return m_clCommandQueue;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
