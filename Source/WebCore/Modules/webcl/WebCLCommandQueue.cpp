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

#include "WebCL.h"
#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLEventList.h"
#include "WebCLException.h"

#include <wtf/ArrayBuffer.h>
#include <wtf/Uint8ClampedArray.h>

namespace WebCore {

WebCLCommandQueue::~WebCLCommandQueue()
{
    ASSERT(m_cl_command_queue);
    int computeContextErrorCode = m_context->computeContext()->releaseCommandQueue(m_cl_command_queue);

    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
    m_cl_command_queue = NULL;
}

PassRefPtr<WebCLCommandQueue> WebCLCommandQueue::create(WebCLContext* context, cl_command_queue command_queue)
{
    return adoptRef(new WebCLCommandQueue(context, command_queue));
}

WebCLCommandQueue::WebCLCommandQueue(WebCLContext* context, cl_command_queue command_queue) : m_context(context), m_cl_command_queue(command_queue)
{
}

WebCLGetInfo WebCLCommandQueue:: getInfo(int param_name, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_uint uint_units = 0;
    cl_device_id cl_device = NULL;
    cl_command_queue_properties queue_properties = NULL;
    RefPtr<WebCLContext> contextObj = NULL;
    RefPtr<WebCLDevice> deviceObj = NULL;

    if (m_cl_command_queue == NULL) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return WebCLGetInfo();
    }
    switch(param_name)
    {

        case WebCL::QUEUE_REFERENCE_COUNT:
            err=clGetCommandQueueInfo(m_cl_command_queue, CL_QUEUE_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
            // FIXME: We should not create a WebCLContext here.
            /*        case WebCL::QUEUE_CONTEXT:
            clGetCommandQueueInfo(m_cl_command_queue, CL_QUEUE_CONTEXT, sizeof(cl_context), &cl_context_id, NULL);
            contextObj = WebCLContext::create(m_context, cl_context_id);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
                break;*/
        case WebCL::QUEUE_DEVICE:
            clGetCommandQueueInfo(m_cl_command_queue, CL_QUEUE_DEVICE, sizeof(cl_device_id), &cl_device, NULL);
            deviceObj = WebCLDevice::create(cl_device);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(PassRefPtr<WebCLDevice>(deviceObj));
            break;
        case WebCL::QUEUE_PROPERTIES:
            clGetCommandQueueInfo(m_cl_command_queue, CL_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &queue_properties, NULL);
            return WebCLGetInfo(static_cast<unsigned int>(queue_properties));
            break;

        default:
            printf("Error: Unsupported Commans Queue Info type\n");
            ec = WebCLException::FAILURE;
            return WebCLGetInfo();
    }
    switch (err) {
        case CL_INVALID_COMMAND_QUEUE:
            printf("Error: CL_INVALID_COMMAND_QUEUE \n");
            ec = WebCLException::INVALID_COMMAND_QUEUE;
            break;
        case CL_INVALID_VALUE:
            printf("Error: CL_INVALID_VALUE \n");
            ec = WebCLException::INVALID_VALUE;
            break;
        case CL_OUT_OF_RESOURCES:
            printf("Error: CL_OUT_OF_RESOURCES \n");
            ec = WebCLException::OUT_OF_RESOURCES;
            break;
        case CL_OUT_OF_HOST_MEMORY:
            printf("Error: CL_OUT_OF_HOST_MEMORY \n");
            ec = WebCLException::OUT_OF_HOST_MEMORY;
            break;
        default:
            printf("Error: Invaild Error Type\n");
            ec = WebCLException::FAILURE;
            break;
    }				
    return WebCLGetInfo();
}



void  WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blocking_write,
        int offset, int buffer_size, ArrayBufferView* ptr, WebCLEventList* events,
        WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem cl_mem_id = NULL;
    cl_int err = 0;
    int eventsLength = 0;
    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;

    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }	

    if (buffer!= NULL)
    {
        cl_mem_id = buffer->getCLBuffer();
        if (cl_mem_id == NULL)
        {
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error: cl_mem_id null\n");
            return;
        }
    }

    if(ptr == NULL) 
    {
        printf("Error: Invalid Buffer\n");
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (events != NULL) 
    {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }

    // TODO(siba samal) - NULL parameters need to be addressed later
    if(blocking_write)
    {
        err = clEnqueueWriteBuffer(m_cl_command_queue, cl_mem_id, CL_TRUE, offset, 
                buffer_size, ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);
    }
    else
    {
        err = clEnqueueWriteBuffer(m_cl_command_queue, cl_mem_id, CL_FALSE, offset, 
                buffer_size, ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);
    }
    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueWriteBuffer\n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
                //case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                //	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
                //	ec = WebCLException::MISALIGNED_SUB_BUFFER_OFFSET;
                //	break;
                //case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
                //	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n");
                //	ec = WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
                //	break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    } 
    return;
}

void WebCLCommandQueue::enqueueWriteBuffer(WebCLBuffer* buffer, bool blocking_write, int offset, int buffer_size,ImageData* ptr, WebCLEventList* events,WebCLEvent* event, ExceptionCode& ec)
{

    cl_mem cl_mem_id = NULL;
    cl_int err = 0;
    int eventsLength = 0;
    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;

    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCL::FAILURE;
        return ;
    }   
    if (buffer != NULL) 
    {
        cl_mem_id = buffer->getCLBuffer();
        if (cl_mem_id == NULL) 
        {
            printf("Error: cl_mem_id null\n");
            return ;
        }
    }

    if (events != NULL) 
    {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }
    unsigned char* buffer_array;
    if(ptr!=NULL && ptr->data()!=NULL && ptr->data()->data()!=NULL && ptr->data()->data()!=NULL )
    {
        buffer_array = ptr->data()->data();
        buffer_size =  ptr->data()->length();
    }
    else
    {
        printf("Error: Invalid ImageData\n");
        ec = WebCL::FAILURE;
        return;
    }
    // TODO(won.jeon) - NULL parameters need to be addressed later
    if(blocking_write) 
    {
        err = clEnqueueWriteBuffer(m_cl_command_queue, cl_mem_id, CL_TRUE, 
                offset, buffer_size, buffer_array, eventsLength, cl_event_wait_lists, &cl_event_id);
    }
    else
    {
        err = clEnqueueWriteBuffer(m_cl_command_queue, cl_mem_id, CL_FALSE, 
                offset, buffer_size, buffer_array, eventsLength, cl_event_wait_lists, &cl_event_id);
    }

    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueWriteBuffer\n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                ec = WebCL::INVALID_COMMAND_QUEUE;
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                break;
            case CL_INVALID_CONTEXT:
                ec = WebCL::INVALID_CONTEXT;
                printf("Error: CL_INVALID_CONTEXT\n");
                break;
            case CL_INVALID_MEM_OBJECT:
                ec = WebCL::INVALID_MEM_OBJECT;
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                break;

            case CL_INVALID_VALUE:
                ec = WebCL::INVALID_VALUE;
                printf("Error: CL_INVALID_VALUE\n");
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                ec = WebCL::INVALID_EVENT_WAIT_LIST;
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                ec = WebCL::MEM_OBJECT_ALLOCATION_FAILURE;
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                break;
            case CL_OUT_OF_HOST_MEMORY:
                ec = WebCL::OUT_OF_HOST_MEMORY;
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                break;
            default:
                ec = WebCL::FAILURE;
                printf("Error: Invaild Error Type\n");
                break;
        }
    } 
    return ;
}

void WebCLCommandQueue::enqueueWriteBufferRect( WebCLBuffer* buffer,bool blocking_write,Int32Array* bufferOrigin,Int32Array* hostOrigin,Int32Array* region,
        int bufferRowPitch,int bufferSlicePitch,int hostRowPitch,int hostSlicePitch,ArrayBufferView* ptr,
        WebCLEventList* eventWaitList, WebCLEvent* event, ExceptionCode& ec )
{
    cl_mem cl_mem_id = NULL;
    cl_int err = 0;
    cl_event cl_event_id = NULL;
    cl_event* cl_event_wait_lists = NULL;
    int eventsLength = 0;
    size_t *bufferOrigin_array= NULL;
    size_t *hostOrigin_array = NULL;
    size_t *region_array = NULL;

    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return; 
    }
    if ( buffer != NULL) 
    {
        cl_mem_id = buffer->getCLBuffer();
        if (cl_mem_id == NULL) 
        {
            printf("Error: cl_mem_id null\n");
            return;
        }
    }
    if (eventWaitList != NULL) 
    {
        cl_event_wait_lists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }
    unsigned bufferOrigin_array_length = bufferOrigin->length();
    bufferOrigin_array = (size_t*)malloc(bufferOrigin_array_length*sizeof(size_t));
    if(NULL == bufferOrigin_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < bufferOrigin_array_length; i++) 
    {
        bufferOrigin_array[i] = bufferOrigin->item(i);
    }
    unsigned hostOrigin_array_length = hostOrigin->length();
    hostOrigin_array = (size_t*)malloc(hostOrigin_array_length*sizeof(size_t));
    if(NULL == hostOrigin_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < hostOrigin_array_length; i++) 
    {
        hostOrigin_array[i] = hostOrigin->item(i);
    }
    unsigned  region_array_length = region->length();
    region_array = (size_t*)malloc(region_array_length*sizeof(size_t));
    if(NULL == region_array)
    {
        printf("Error: Error allocating memorry");
        return;
    }
 
    for (unsigned int i = 0; i < region_array_length; i++) 
    {
        region_array[i] = region->item(i);
    }

    if(blocking_write)
    {
        err = clEnqueueWriteBufferRect(m_cl_command_queue, cl_mem_id, CL_TRUE, bufferOrigin_array , hostOrigin_array , region_array , bufferRowPitch , bufferSlicePitch , hostRowPitch , hostSlicePitch , ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);
    }
    else
    {
        err = clEnqueueWriteBufferRect(m_cl_command_queue, cl_mem_id, CL_FALSE ,bufferOrigin_array ,hostOrigin_array ,region_array , bufferRowPitch , bufferSlicePitch , hostRowPitch , hostSlicePitch , ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);
    }
    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueWriteBufferRect \n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
                //case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                //	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
                //	ec = WebCLException::MISALIGNED_SUB_BUFFER_OFFSET;
                //	break;
                //case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
                //	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n");
                //	ec = WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
                //	break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;

        }
    }
    else
    {
        return ;
    }
}

void WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer, bool blocking_read,int offset, int buffer_size, ImageData* ptr, WebCLEventList* events,WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem cl_mem_id = NULL;
    cl_int err = 0;
    cl_event cl_event_id = NULL;
    int eventsLength = 0;
    cl_event *cl_event_wait_lists = NULL;

    if (m_cl_command_queue == NULL)
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return ;
    }
    if (buffer != NULL) 
    {
        cl_mem_id = buffer->getCLBuffer();
        if (cl_mem_id == NULL) 
        {
            printf("Error: cl_mem_id null\n");
            return ;
        }
    }

    if (events != NULL) 
    {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }
    unsigned char* buffer_array;
    if(ptr!=NULL && ptr->data()!=NULL && ptr->data()->data()!=NULL && ptr->data()->data()!=NULL )
    {
        buffer_array = ptr->data()->data();
        buffer_size =  ptr->data()->length();
    }
    else
    {
        printf("Error: Invalid ImageData\n");
        ec = WebCL::FAILURE;
        return ;
    }
    // TODO(siba samal) - NULL parameters need to be addressed later
    if(blocking_read)
    {
        err = clEnqueueReadBuffer(m_cl_command_queue, cl_mem_id, CL_TRUE, 
                offset,buffer_size, buffer_array, eventsLength, cl_event_wait_lists, &cl_event_id);
    }
    else
    {
        err = clEnqueueReadBuffer(m_cl_command_queue, cl_mem_id, CL_FALSE, 
                offset, buffer_size,  buffer_array , eventsLength, cl_event_wait_lists, &cl_event_id);
    }

    if (err != CL_SUCCESS)
    {
        printf("Error: clEnqueueReadBuffer\n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
                //case CL_MISALIGNED_SUB_BUFFER_OFFSET :
                //	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET \n");
                //	ec = WebCLException::MISALIGNED_SUB_BUFFER_OFFSET;
                //	break;
                //case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  :
                //	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  \n");
                //	ec = WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
                //	break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES \n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    } 
    return ;
}

void WebCLCommandQueue::enqueueReadImage(WebCLImage* image, bool blocking_read,Int32Array* origin, Int32Array* region,int rowPitch,int slicePitch,
    ArrayBufferView* ptr,WebCLEventList* events,WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem cl_mem_id = NULL;
    cl_int err = 0;
    cl_event cl_event_id = NULL;
    cl_event* cl_event_wait_lists = NULL;
    int eventsLength = 0;
    size_t *origin_array = NULL;
    size_t *region_array = NULL;

    if (m_cl_command_queue == NULL)
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }
    if (image != NULL) 
    {
        cl_mem_id = image->getCLImage();
        if (cl_mem_id == NULL) 
        {
            printf("Error: cl_mem_id null\n");
            return;
        }
    }
    if (events != NULL)
    {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }

    unsigned origin_array_length = origin->length();
    origin_array = (size_t*)malloc(origin_array_length*sizeof(size_t));
    if( NULL == origin_array){
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < origin_array_length; i++) 
    {
        origin_array[i] = origin->item(i);
    }
    unsigned  region_array_length = region->length();
    region_array = (size_t*)malloc(region_array_length*sizeof(size_t));
    if(NULL == region_array ){
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < region_array_length; i++) 
    {
        region_array[i] = region->item(i);
    }

    // TODO(siba samal) - NULL parameters need to be addressed later
    if(blocking_read)
    {
        for (unsigned int i = 0; i < 24; i++)
        {
            printf(" Before clEnqueueReadImage => %d ptr->baseAddress() => %u   \n ", i,((unsigned char*)ptr->baseAddress())[i]);
        }
        err = clEnqueueReadImage(m_cl_command_queue, cl_mem_id, CL_TRUE, origin_array, region_array, rowPitch, slicePitch, ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);
        for (unsigned int i = 0; i < 24; i++)
        {
            printf(" After clEnqueueReadImage => %d ptr->baseAddress() => %u   \n ", i,((unsigned char*)ptr->baseAddress())[i]);
        }
    }       
    else
    {
        for (unsigned int i = 0; i < 24; i++)
        {
            printf(" Before clEnqueueReadImage => %d ptr->baseAddress() => %u   \n ", i,((unsigned char*)ptr->baseAddress())[i]);
        }
        err = clEnqueueReadImage(m_cl_command_queue, cl_mem_id, CL_FALSE, origin_array, region_array, rowPitch, slicePitch, ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);
        for (unsigned int i = 0; i < 24; i++)
        {
            printf(" After clEnqueueReadImage => %d ptr->baseAddress() => %u   \n ", i,((unsigned char*)ptr->baseAddress())[i]);
        }

    }
    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueReadImage\n");
        switch (err)
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
                //case CL_MISALIGNED_SUB_BUFFER_OFFSET :
                //	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET \n");
                //	ec = WebCLException::MISALIGNED_SUB_BUFFER_OFFSET;
                //	break;
                //case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  :
                //	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  \n");
                //	ec = WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
                //	break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES \n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    }
    return ;
}


void  WebCLCommandQueue::enqueueReadBuffer(WebCLBuffer* buffer_src , bool blocking_read,int offset, int buffer_size, ArrayBufferView* ptr, WebCLEventList* events,WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem cl_mem_id = NULL;
    cl_int err = 0;
    cl_event cl_event_id = NULL;
    cl_event* cl_event_wait_lists = NULL;
    int eventsLength = 0;

    if (m_cl_command_queue == NULL) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return; 
    }
    if (buffer_src != NULL) {
        cl_mem_id = buffer_src->getCLBuffer();
        if (cl_mem_id == NULL) {
            printf("Error: cl_mem_id null\n");
            return;
        }


    }
    if (events != NULL) {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }
    if (event != NULL) {
        cl_event_id = event->getCLEvent();
    }

    // TODO(siba samal) - NULL parameters need to be addressed later
    if(blocking_read)
        err = clEnqueueReadBuffer(m_cl_command_queue, cl_mem_id, CL_TRUE, offset, buffer_size, ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);
    else
        err = clEnqueueReadBuffer(m_cl_command_queue, cl_mem_id, CL_FALSE, offset, buffer_size, ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);

    if (err != CL_SUCCESS) {
        printf("Error: clEnqueueReadBuffer\n");
        switch (err) {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
                //case CL_MISALIGNED_SUB_BUFFER_OFFSET :
                //	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET \n");
                //	ec = WebCLException::MISALIGNED_SUB_BUFFER_OFFSET;
                //	break;
                //case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  :
                //	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  \n");
                //	ec = WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
                //	break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES \n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;

        }
    }
    return ;
}

void  WebCLCommandQueue::enqueueReadBufferRect(WebCLBuffer* buffer, bool blocking_read,
        Int32Array* bufferOrigin,Int32Array* hostOrigin,Int32Array* region,
        int bufferRowPitch,int bufferSlicePitch,int hostRowPitch,int hostSlicePitch,
        ArrayBufferView* ptr, WebCLEventList* eventWaitList,WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem cl_mem_id = NULL;
    cl_int err = 0;
    cl_event cl_event_id = NULL;
    cl_event* cl_event_wait_lists = NULL;
    int eventsLength = 0;
    size_t *bufferOrigin_array= NULL;
    size_t *hostOrigin_array = NULL;
    size_t *region_array = NULL;

    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return; 
    }
    if ( buffer != NULL) 
    {
        cl_mem_id = buffer->getCLBuffer();
        if (cl_mem_id == NULL) {
            printf("Error: cl_mem_id null\n");
            return;
        }
    }
    if (eventWaitList != NULL) 
    {
        cl_event_wait_lists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }
    unsigned bufferOrigin_array_length = bufferOrigin->length();
    bufferOrigin_array = (size_t*)malloc(bufferOrigin_array_length*sizeof(size_t));
    if(NULL == bufferOrigin_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < bufferOrigin_array_length; i++) 
    {
        bufferOrigin_array[i] = bufferOrigin->item(i);
    }
    unsigned hostOrigin_array_length = hostOrigin->length();
    hostOrigin_array = (size_t*)malloc(hostOrigin_array_length*sizeof(size_t));
    if(NULL == hostOrigin_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < hostOrigin_array_length; i++) 
    {
        hostOrigin_array[i] = hostOrigin->item(i);
    }
    unsigned  region_array_length = region->length();
    region_array = (size_t*)malloc(region_array_length*sizeof(size_t));
    if(NULL == region_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < region_array_length; i++) 
    {
        region_array[i] = region->item(i);
    }

    if(blocking_read)
    {
        err = clEnqueueReadBufferRect(m_cl_command_queue, cl_mem_id, CL_TRUE, bufferOrigin_array , hostOrigin_array , region_array , bufferRowPitch , bufferSlicePitch , hostRowPitch , hostSlicePitch , ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);
    }
    else
    {
        err = clEnqueueReadBufferRect(m_cl_command_queue, cl_mem_id, CL_FALSE ,bufferOrigin_array ,hostOrigin_array ,region_array , bufferRowPitch , bufferSlicePitch , hostRowPitch , hostSlicePitch , ptr->baseAddress(), eventsLength, cl_event_wait_lists, &cl_event_id);
    }
    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueReadBufferRect\n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
                //case CL_MISALIGNED_SUB_BUFFER_OFFSET :
                //	printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET \n");
                //	ec = WebCLException::MISALIGNED_SUB_BUFFER_OFFSET;
                //	break;
                //case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  :
                //	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST  \n");
                //	ec = WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
                //	break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES \n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;

        }
    }
    return ;
}

void WebCLCommandQueue::enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* globalWorkOffsets,
        Int32Array* globalWorkSize, Int32Array* localWorkSize, WebCLEventList* events,
        WebCLEvent* event, ExceptionCode& ec)
{
    cl_kernel cl_kernel_id;
    cl_event* cl_event_wait_lists = 0;
    cl_event cl_event_id;
    int eventsLength = 0;

    size_t* globalWorkSizeCopy = 0;
    size_t* localWorkSizeCopy = 0;
    size_t* globalWorkOffsetCopy = 0;
    int workItemDimensions = 0;

    if (!m_cl_command_queue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return ;
    }

    if (kernel) {
        cl_kernel_id = kernel->getCLKernel();
        if (!cl_kernel_id) {
            ec = WebCLException::INVALID_KERNEL;
            printf("Error: cl_kernel_id null\n");
            return ;
        }
    }

    if (events) {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }

    if (event)
        cl_event_id = event->getCLEvent();

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
        if (!localWorkSizeCopy){
            printf("Error: Error allocating memory");
            return;
        }
        for (unsigned i = 0; i < localWorkSize->length(); ++i) {
            localWorkSizeCopy[i] = localWorkSize->item(i);
        }
    }

    if (globalWorkOffsets) {
        globalWorkOffsetCopy = (size_t*) malloc(globalWorkOffsets->length() * sizeof(size_t));
        if (!globalWorkOffsetCopy) {
            printf("Error: Error allocating memorry");
            return;
        }
        for (unsigned i = 0; i < globalWorkOffsets->length(); ++i)
            globalWorkOffsetCopy[i] = globalWorkOffsets->item(i);
    }

    int computeContextError = m_context->computeContext()->enqueueNDRangeKernel(m_cl_command_queue, cl_kernel_id, workItemDimensions,
	globalWorkOffsetCopy, globalWorkSizeCopy, localWorkSizeCopy, eventsLength, cl_event_wait_lists, &cl_event_id);

    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextError);

    free(globalWorkSizeCopy);
    globalWorkSizeCopy = 0;
    free(localWorkSizeCopy);
    localWorkSizeCopy = 0;
    free(globalWorkOffsetCopy);
    globalWorkOffsetCopy = 0;

    return;	
}

void WebCLCommandQueue::finish(ExceptionCode& ec)
{
    cl_int err = 0;

    if (m_cl_command_queue == NULL) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        printf("Error: Invalid Command Queue\n");
        return;
    }
    err = clFinish(m_cl_command_queue);
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMAND_QUEUE \n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY \n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES \n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }		
    }
    return;
}


void WebCLCommandQueue::flush( ExceptionCode& ec)
{
    cl_int err = 0;

    if (m_cl_command_queue == NULL) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::FAILURE;
        return;
    }
    err = clFlush(m_cl_command_queue);
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY \n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES \n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    } else {
        return;
    }
    return;
}

void WebCLCommandQueue::enqueueWriteImage(WebCLImage* image,bool blockingWrite,Int32Array* origin,Int32Array* region,int inputRowPitch,int inputSlicePitch,ArrayBufferView* ptr,WebCLEventList* eventWaitList,WebCLEvent* event,ExceptionCode& ec)
{
    cl_mem cl_image_id = NULL;
    cl_int err = 0;
    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;
    int eventsLength = 0;
    size_t *origin_array = NULL;
    size_t *region_array = NULL;

    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }
    if (image != NULL) 
    {
        cl_image_id = image->getCLImage();
        if (cl_image_id == NULL) 
        {
            printf("Error: cl_image_id null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }
    unsigned origin_array_length = origin->length();
    origin_array = (size_t*)malloc(origin_array_length*sizeof(size_t));
    if(NULL == origin_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }

    for (unsigned int i = 0; i < origin_array_length; i++) 
    {
        origin_array[i] = origin->item(i);
    }

    unsigned  region_array_length = region->length();
    region_array = (size_t*)malloc(region_array_length*sizeof(size_t));
    if(NULL == region_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < region_array_length; i++) 
    {
        region_array[i] = region->item(i);
    }


    if (eventWaitList != NULL) 
    {
        cl_event_wait_lists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }

    err = clEnqueueWriteImage(m_cl_command_queue,cl_image_id,blockingWrite,origin_array,region_array,inputRowPitch,inputSlicePitch,ptr->baseAddress(),
            eventsLength,cl_event_wait_lists,&cl_event_id);

    if( err!= CL_SUCCESS )
    {
        switch(err)
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: INVALID_COMMAND_QUEUE \n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: INVALID_CONTEXT \n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: INVALID_VALUE \n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_INVALID_IMAGE_SIZE:
                printf("Error: INVALID_IMAGE_SIZE \n");
                ec = WebCLException::INVALID_IMAGE_SIZE;
                break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: MEM_OBJECT_ALLOCATION_FAILURE \n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_INVALID_OPERATION:
                printf("Error: INVALID_OPERATION \n");
                ec = WebCLException::INVALID_OPERATION;
                break;
            case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
                printf("Error: EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST \n");
                ec = WebCLException::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: OUT_OF_RESOURCES \n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: OUT_OF_HOST_MEMORY \n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }// Switch
    }
    return;
}


void WebCLCommandQueue::enqueueAcquireGLObjects(WebCLMemoryObject *mem_objects, WebCLEventList* events, WebCLEvent* event, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_mem cl_mem_ids[1] = {NULL};
    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;
    int eventsLength = 0;
    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (mem_objects != NULL)
    {
        if( mem_objects->isShared() == true )
        {
            cl_mem_ids[0] = mem_objects->getCLMemoryObject();
            if (cl_mem_ids[0] == NULL )
            {
                printf("Error: cl_mem_ids null\n"); 
                ec = WebCLException::FAILURE;
                return;
            }
        }
        else
        {
            printf("Error:: mem_objects is not shared with GL\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }
    else
    {
        printf("Error: Memory objects passed is Null\n");
        ec = WebCLException::FAILURE;
        return;
    }
    if (events != NULL) 
    {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }
    err = clEnqueueAcquireGLObjects(m_cl_command_queue, 1, cl_mem_ids, eventsLength, cl_event_wait_lists, &cl_event_id);
    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueAcquireGLObjects\n");
        switch (err) 
        {
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_GL_OBJECT:
                printf("Error: CL_INVALID_GL_OBJECT\n");
                ec = WebCLException::INVALID_GL_OBJECT;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    } 
    else 
    {
        return;
    }
    return;
}

void WebCLCommandQueue::enqueueReleaseGLObjects(WebCLMemoryObject *mem_objects, WebCLEventList* events,
        WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem cl_mem_ids = NULL;
    cl_int err = 0;
    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;
    int eventsLength = 0;

    if (m_cl_command_queue == NULL) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }
    if ((mem_objects != NULL) && (mem_objects->isShared())) {
        cl_mem_ids = mem_objects->getCLMemoryObject();
        if (cl_mem_ids == NULL) {
            printf("Error: cl_mem_ids null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }
    if (events != NULL) {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }
    if (event != NULL) {
        cl_event_id = event->getCLEvent();
    }

    err = clEnqueueReleaseGLObjects(m_cl_command_queue, 1 ,&cl_mem_ids, 
            eventsLength, cl_event_wait_lists, &cl_event_id);
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_GL_OBJECT:
                printf("Error: CL_INVALID_GL_OBJECT\n");
                ec = WebCLException::INVALID_GL_OBJECT;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    } 
    else 
    {
        unsigned int i;
        for (i = 0; i < m_mem_list.size(); i++)
        {
            if (( m_mem_list[i].get())->getCLMemoryObject() == cl_mem_ids) 
            {
                m_mem_list.remove(i);
                m_num_mems = m_mem_list.size();
                break;
            }
        }   
        return;
    }
}
void WebCLCommandQueue::enqueueCopyImage(WebCLImage* src_image, WebCLImage* dst_image,Int32Array* src_origin,Int32Array* dst_origin,Int32Array* region,
        WebCLEventList* events, WebCLEvent* event,ExceptionCode& ec)
{
    cl_mem cl_src_image_id = NULL;
    cl_mem cl_dst_image_id = NULL;
    cl_int err = 0;

    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;
    int eventsLength = 0;

    size_t *src_origin_array = NULL;
    size_t *dst_origin_array = NULL;
    size_t *region_array = NULL;

    if (m_cl_command_queue == NULL) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }
    if (src_image != NULL) {
        cl_src_image_id = src_image->getCLImage();
        if (cl_src_image_id == NULL) {
            printf("Error: cl_src_image_id null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }
    if (dst_image != NULL) {
        cl_dst_image_id = dst_image->getCLImage();
        if (cl_dst_image_id == NULL) {
            printf("Error: cl_dst_image_id null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    unsigned src_origin_array_length = src_origin->length();
    src_origin_array = (size_t*)malloc(src_origin_array_length*sizeof(size_t));
    if(NULL == src_origin_array){
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < src_origin_array_length; i++) {
        src_origin_array[i] = src_origin->item(i);
    }

    unsigned dst_origin_array_length = dst_origin->length();
    dst_origin_array = (size_t*)malloc(dst_origin_array_length*sizeof(size_t));
    if(NULL == dst_origin_array){
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < dst_origin_array_length; i++) {
        dst_origin_array[i] = dst_origin->item(i);
    }

    unsigned  region_array_length = region->length();
    region_array = (size_t*)malloc(region_array_length*sizeof(size_t));
    if(NULL == region_array ){
        printf("Error: Error allocating memorry");
        return;
    }
    for (unsigned int i = 0; i < region_array_length; i++) {
        region_array[i] = region->item(i);
    }


    if (events != NULL) {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }
    if (event != NULL) {
        cl_event_id = event->getCLEvent();
    }

    err = clEnqueueCopyImage(m_cl_command_queue, cl_src_image_id, cl_dst_image_id,
            src_origin_array, dst_origin_array, region_array, eventsLength, cl_event_wait_lists, &cl_event_id);

    if (err != CL_SUCCESS) {
        printf("Error: clEnqueueCopyImage\n");
        switch (err) {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_MEM_COPY_OVERLAP:
                printf("Error: CL_MEM_COPY_OVERLAP\n");
                ec = WebCLException::MEM_COPY_OVERLAP;
                break;
                //CHECK(won.jeon) - defined in 1.1?
                //case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                //printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
                //break;

            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    } 
    else 
    {
        return;
    }
    return;
}


void WebCLCommandQueue::enqueueCopyImageToBuffer( WebCLImage *srcImage, WebCLBuffer *dstBuffer,Int32Array* srcOrigin,Int32Array* region,int dstOffset,
        WebCLEventList* eventWaitList,WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem cl_src_image_id= NULL;
    cl_mem cl_dst_buffer_id = NULL;
    cl_int err = 0;
    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;
    int eventsLength = 0;
    size_t *src_origin_array = NULL;
    size_t *region_array = NULL;

    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }
    if (srcImage != NULL) 
    {
        cl_src_image_id = srcImage->getCLImage();
        if (cl_src_image_id == NULL) 
        {
            printf("Error: cl_src_image_id null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }
    if (dstBuffer != NULL) 
    {
        cl_dst_buffer_id = dstBuffer->getCLBuffer();
        if (cl_dst_buffer_id == NULL) 
        {
            printf("Error: cl_dst_buffer_id null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    unsigned srcOrigin_array_length = srcOrigin->length();
    src_origin_array = (size_t*)malloc(srcOrigin_array_length*sizeof(size_t));
    if(NULL == src_origin_array)
    {
        printf("Error: Error allocating memorry");
        return;
    }

    for (unsigned int i = 0; i < srcOrigin_array_length; i++) 
    {
        src_origin_array[i] = srcOrigin->item(i);
    }

    unsigned  region_array_length = region->length();
    region_array = (size_t*)malloc(region_array_length*sizeof(size_t));
    if(NULL == region_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }

    for (unsigned int i = 0; i < region_array_length; i++) 
    {
        region_array[i] = region->item(i);
    }


    if (eventWaitList != NULL) 
    {
        cl_event_wait_lists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }

    err = clEnqueueCopyImageToBuffer(m_cl_command_queue, cl_src_image_id , cl_dst_buffer_id,
            src_origin_array,region_array, dstOffset, eventsLength ,cl_event_wait_lists, &cl_event_id);

    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueCopyImageToBuffer \n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET \n");
                ec = WebCLException::MISALIGNED_SUB_BUFFER_OFFSET;
                break;
            case CL_INVALID_IMAGE_SIZE:
                printf("Error: CL_INVALID_IMAGE_SIZE \n");
                ec = WebCLException::INVALID_IMAGE_SIZE;
                break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE \n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_INVALID_OPERATION:
                printf("Error: CL_INVALID_OPERATION\n");
                ec = WebCLException::INVALID_OPERATION;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    }// If !CS_SUCCESS
    return;
}

void WebCLCommandQueue::enqueueCopyBufferToImage( WebCLBuffer *srcBuffer, WebCLImage *dstImage,int srcOffset,Int32Array* dstOrigin,Int32Array* region,
        WebCLEventList* eventWaitList,WebCLEvent* event, ExceptionCode& ec)
{
    cl_mem cl_src_buffer_id = NULL;
    cl_mem cl_dst_image_id = NULL;
    cl_int err = 0;
    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;
    int eventsLength = 0;
    size_t *dst_origin_array = NULL;
    size_t *region_array = NULL;

    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }
    if (srcBuffer != NULL) 
    {
        cl_src_buffer_id = srcBuffer->getCLBuffer();
        if (cl_src_buffer_id == NULL) 
        {
            printf("Error:  cl_src_buffer_id null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }
    if (dstImage != NULL) 
    {
        cl_dst_image_id = dstImage->getCLImage();
        if (cl_dst_image_id == NULL) 
        {
            printf("Error:  cl_dst_image_id null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    unsigned dstOrigin_array_length = dstOrigin->length();
    dst_origin_array = (size_t*)malloc(dstOrigin_array_length*sizeof(size_t));
    if(NULL == dst_origin_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }

    for (unsigned int i = 0; i < dstOrigin_array_length; i++) 
    {
        dst_origin_array[i] = dstOrigin->item(i);
    }

    unsigned  region_array_length = region->length();
    region_array = (size_t*)malloc(region_array_length*sizeof(size_t));
    if(NULL == region_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }

    for (unsigned int i = 0; i < region_array_length; i++) 
    {
        region_array[i] = region->item(i);
    }


    if (eventWaitList != NULL) 
    {
        cl_event_wait_lists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }

    err = clEnqueueCopyBufferToImage(m_cl_command_queue,cl_src_buffer_id,cl_dst_image_id,srcOffset,
            dst_origin_array,region_array,eventsLength ,cl_event_wait_lists, &cl_event_id);

    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueCopyBufferToImage \n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET \n");
                ec = WebCLException::MISALIGNED_SUB_BUFFER_OFFSET;
                break;
            case CL_INVALID_IMAGE_SIZE:
                printf("Error: CL_INVALID_IMAGE_SIZE \n");
                ec = WebCLException::INVALID_IMAGE_SIZE;
                break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE \n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_INVALID_OPERATION:
                printf("Error: CL_INVALID_OPERATION\n");
                ec = WebCLException::INVALID_OPERATION;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    }// If !CS_SUCCESS
    return;
}

void WebCLCommandQueue::enqueueCopyBuffer(WebCLBuffer* src_buffer, WebCLBuffer* dst_buffer, int cb, ExceptionCode& ec)
{
    cl_mem cl_src_buffer_id = NULL;
    cl_mem cl_dst_buffer_id = NULL;
    cl_int err = 0;

    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }
    if (src_buffer != NULL) 
    {
        cl_src_buffer_id = src_buffer->getCLBuffer();
        if (cl_src_buffer_id == NULL) 
        {
            printf("Error: cl_src_buffer_id null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }
    if (dst_buffer != NULL) 
    {
        cl_dst_buffer_id = dst_buffer->getCLBuffer();
        if (cl_dst_buffer_id == NULL) 
        {
            printf("Error: cl_dst_buffer_id null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }
    err = clEnqueueCopyBuffer(m_cl_command_queue, cl_src_buffer_id, cl_dst_buffer_id,0, 0, cb, 0, NULL, NULL);
    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueCopyBuffer\n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_MEM_COPY_OVERLAP:
                printf("Error: CL_MEM_COPY_OVERLAP\n");
                ec = WebCLException::MEM_COPY_OVERLAP;
                break;
                //CHECK(won.jeon) - defined in 1.1?
                //case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                //printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
                //break;

            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    } 
    else 
    {
        return;
    }
    return;
}

void WebCLCommandQueue::enqueueCopyBuffer(WebCLBuffer* src_buffer, WebCLBuffer* dst_buffer,int srcOffset,int dstOffset,int sizeInBytes, WebCLEventList* events,
        WebCLEvent* event,ExceptionCode& ec)
{
    cl_mem cl_src_buffer_id = NULL;
    cl_mem cl_dst_buffer_id = NULL;
    cl_int err = 0;
    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;
    int eventsLength = 0;

    if (m_cl_command_queue == NULL)
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }
    if (src_buffer != NULL) 
    {
        cl_src_buffer_id = src_buffer->getCLBuffer();
        if (cl_src_buffer_id == NULL) 
        {
            printf("Error: cl_src_buffer_id null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }
    if (dst_buffer != NULL) 
    {
        cl_dst_buffer_id = dst_buffer->getCLBuffer();
        if (cl_dst_buffer_id == NULL) 
        {
            printf("Error: cl_dst_buffer_id null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }

    if (events != NULL) 
    {
        cl_event_wait_lists = events->getCLEvents();
        eventsLength = events->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }

    err = clEnqueueCopyBuffer(m_cl_command_queue, cl_src_buffer_id, cl_dst_buffer_id,
            srcOffset, dstOffset,sizeInBytes, eventsLength,cl_event_wait_lists, &cl_event_id);
    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueCopyBuffer\n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_MEM_COPY_OVERLAP:
                printf("Error: CL_MEM_COPY_OVERLAP\n");
                ec = WebCLException::MEM_COPY_OVERLAP;
                break;
                /* CHECK(won.jeon) - defined in 1.1?
                   case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                   printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
                   break;
                 */
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    } 
    else 
    {
        return;
    }
    return;
}
void  WebCLCommandQueue::enqueueCopyBufferRect( WebCLBuffer* src_buffer,WebCLBuffer* dst_buffer,Int32Array* srcOrigin,Int32Array* dstOrigin,Int32Array* region,int srcRowPitch,int srcSlicePitch,int dstRowPitch,int dstSlicePitch,WebCLEventList* eventWaitList,WebCLEvent* event,ExceptionCode& ec )
{
    cl_mem cl_src_buffer_id = NULL;
    cl_mem cl_dst_buffer_id = NULL;
    cl_int err = 0;
    cl_event *cl_event_wait_lists = NULL;
    cl_event cl_event_id = NULL;
    int eventsLength = 0;
    size_t *srcOrigin_array = NULL;
    size_t *dstOrigin_array = NULL;
    size_t *region_array = NULL;

    if (m_cl_command_queue == NULL) 
    {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }
    if (src_buffer != NULL) 
    {
        cl_src_buffer_id = src_buffer->getCLBuffer();
        if (cl_src_buffer_id == NULL) 
        {
            printf("Error: cl_src_buffer_id null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }
    if (dst_buffer != NULL)
    {
        cl_dst_buffer_id = dst_buffer->getCLBuffer();
        if (cl_dst_buffer_id == NULL) 
        {
            printf("Error: cl_dst_buffer_id null\n");
            ec = WebCLException::FAILURE;
            return;
        }
    }
    unsigned srcOrigin_array_length = srcOrigin->length();
    srcOrigin_array = (size_t*)malloc(srcOrigin_array_length*sizeof(size_t));
    if(NULL == srcOrigin_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }

    for (unsigned int i = 0; i < srcOrigin_array_length; i++) 
    {
        srcOrigin_array[i] = srcOrigin->item(i);
    }
    unsigned dstOrigin_array_length = dstOrigin->length();
    dstOrigin_array = (size_t*)malloc(dstOrigin_array_length*sizeof(size_t));
    if(NULL == dstOrigin_array )
    {
        printf("Error: Error allocating memorry");
        return;
    }

    for (unsigned int i = 0; i < dstOrigin_array_length; i++) {
        dstOrigin_array[i] = dstOrigin->item(i);
    }

    unsigned  region_array_length = region->length();
    region_array = (size_t*)malloc(region_array_length*sizeof(size_t));
    if(NULL == region_array ){
        printf("Error: Error allocating memorry");
        return;
    }

    for (unsigned int i = 0; i < region_array_length; i++) 
    {
        region_array[i] = region->item(i);
    }

    if (eventWaitList!= NULL) 
    {
        cl_event_wait_lists = eventWaitList->getCLEvents();
        eventsLength = eventWaitList->length();
    }
    if (event != NULL) 
    {
        cl_event_id = event->getCLEvent();
    }

    err = clEnqueueCopyBufferRect(m_cl_command_queue, cl_src_buffer_id, cl_dst_buffer_id,
            srcOrigin_array,dstOrigin_array,region_array,srcRowPitch,srcSlicePitch,dstRowPitch,dstSlicePitch, 
            eventsLength,cl_event_wait_lists, &cl_event_id);
    if (err != CL_SUCCESS) 
    {
        printf("Error: clEnqueueCopyBufferRect\n");
        switch (err) 
        {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_MEM_COPY_OVERLAP:
                printf("Error: CL_MEM_COPY_OVERLAP\n");
                ec = WebCLException::MEM_COPY_OVERLAP;
                break;
                /* CHECK(won.jeon) - defined in 1.1?
                   case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                   printf("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n");
                   break;
                 */
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    }// if !CL_SUCCESS
    else
    {
        return;
    }
}

void WebCLCommandQueue::enqueueBarrier(WebCLEventList* eventsWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    eventsWaitList = 0;
    event = 0;

    if (!m_cl_command_queue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    cl_int error = 0;
    error = clEnqueueBarrier(m_cl_command_queue);

    if (error != CL_SUCCESS) {
        switch (error) {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                ec = WebCLException::FAILURE;
                printf("Error: Invaild Error Type\n");
                break;
        }
    }
}

void WebCLCommandQueue::enqueueMarker(WebCLEventList* eventsWaitList, WebCLEvent* event, ExceptionCode& ec)
{
    eventsWaitList = 0;
    cl_event clEventID = 0;

    if (!m_cl_command_queue) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    if (event) {
        clEventID = event->getCLEvent();
        if (!clEventID) {
            printf("cl_event_id null\n");
            ec = WebCLException::INVALID_EVENT;
            return;
        }
    }

    cl_int error = 0;
    error = clEnqueueMarker(m_cl_command_queue, &clEventID);

    if (error != CL_SUCCESS) {
        switch (error) {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                ec = WebCLException::FAILURE;
                printf("Error: Invaild Error Type\n");
                break;
        }
    }
}

PassRefPtr<WebCLEvent> WebCLCommandQueue::enqueueTask(WebCLKernel* kernel, 
        int event_wait_list, ExceptionCode& ec)
{

    cl_kernel cl_kernel_id = NULL;
    cl_int err = 0;
    cl_event cl_event_id = NULL;

    if (m_cl_command_queue == NULL) {
        printf("Error: Invalid Command Queue\n");
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return NULL;
    }
    if (kernel != NULL) {
        cl_kernel_id = kernel->getCLKernel();
        if (cl_kernel_id == NULL) {
            printf("Error: cl_kernel_id null\n");
            //TODO (siba samal) Handle enqueueTask  API
            printf("WebCLCommandQueue::enqueueTask event_wait_list=%d\n",
                    event_wait_list);
            ec = WebCLException::INVALID_KERNEL;
            return NULL;
        }
    }

    err = clEnqueueTask(m_cl_command_queue, cl_kernel_id, 0, NULL,&cl_event_id); 

    if (err != CL_SUCCESS) {
        printf("Error: clEnqueueTask\n");
        switch (err) {
            case CL_INVALID_COMMAND_QUEUE:
                printf("Error: CL_INVALID_COMMAND_QUEUE\n");
                ec = WebCLException::INVALID_COMMAND_QUEUE;
                break;
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT\n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_PROGRAM_EXECUTABLE :
                printf("Error: CL_INVALID_PROGRAM_EXECUTABLE \n");
                ec = WebCLException::INVALID_PROGRAM_EXECUTABLE;
                break;				
            case CL_INVALID_KERNEL :
                printf("Error: CL_INVALID_KERNEL \n");
                ec = WebCLException::INVALID_KERNEL;
                break;
            case CL_INVALID_EVENT_WAIT_LIST:
                printf("Error: CL_INVALID_EVENT_WAIT_LIST\n");
                ec = WebCLException::INVALID_EVENT_WAIT_LIST;
                break;
            case CL_INVALID_KERNEL_ARGS :
                printf("Error: CL_INVALID_KERNEL_ARGS \n");
                ec = WebCLException::INVALID_KERNEL_ARGS;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            case CL_INVALID_WORK_GROUP_SIZE :
                printf("Error: CL_INVALID_WORK_GROUP_SIZE \n");
                ec = WebCLException::FAILURE;
                break;		
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES  \n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;			
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE  \n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;	
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }

    } else {
        RefPtr<WebCLEvent> o = WebCLEvent::create(m_context, cl_event_id);
        m_event_list.append(o);
        m_num_events++;
        return o;
    }
    return NULL;
}

cl_command_queue WebCLCommandQueue::getCLCommandQueue()
{
    return m_cl_command_queue;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
