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

#include "WebCLBuffer.h"
#include "WebCL.h"
namespace WebCore {

WebCLBuffer::~WebCLBuffer()
{
}

PassRefPtr<WebCLBuffer> WebCLBuffer::create(WebCL* compute_context,cl_mem buffer, bool is_shared = false)
{
	return adoptRef(new WebCLBuffer(compute_context, buffer, is_shared));
}

WebCLBuffer::WebCLBuffer(WebCL* compute_context, cl_mem buffer, bool is_shared) 
		:WebCLMemoryObject(compute_context,buffer,is_shared)
{
}

cl_mem WebCLBuffer::getCLBuffer()
{
	return m_cl_mem;
}

PassRefPtr<WebCLBuffer>  WebCLBuffer::createSubBuffer(int flags,int origin,int size, ExceptionCode& ec)
{
    cl_mem buffer = getCLBuffer();
    if( NULL == buffer)
    {
        printf(" Error : getCLBuffer() returned NULL..\n");
        return NULL;
    }

    cl_buffer_region *buffer_create_info = new cl_buffer_region();
    buffer_create_info->origin = origin;
    buffer_create_info->size = size;

    int errcode_ret = 0 ;
    cl_mem result_sub_buffer = NULL;

    switch (flags)
    {
        case WebCL::MEM_READ_ONLY:
            result_sub_buffer = clCreateSubBuffer(buffer,CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, buffer_create_info , &errcode_ret);
            break;
        case WebCL::MEM_WRITE_ONLY:
            result_sub_buffer = clCreateSubBuffer(buffer, CL_MEM_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, buffer_create_info , &errcode_ret);
            break;
        case WebCL::MEM_READ_WRITE:
            result_sub_buffer = clCreateSubBuffer(buffer, CL_MEM_READ_WRITE, CL_BUFFER_CREATE_TYPE_REGION, buffer_create_info , &errcode_ret);
            break;
        case WebCL::MEM_USE_HOST_PTR:
            result_sub_buffer = clCreateSubBuffer(buffer, CL_MEM_USE_HOST_PTR, CL_BUFFER_CREATE_TYPE_REGION, buffer_create_info , &errcode_ret);
            break;
        case WebCL::MEM_ALLOC_HOST_PTR:
            result_sub_buffer = clCreateSubBuffer(buffer, CL_MEM_ALLOC_HOST_PTR, CL_BUFFER_CREATE_TYPE_REGION, buffer_create_info , &errcode_ret);
            break;
        case WebCL::MEM_COPY_HOST_PTR:
            result_sub_buffer = clCreateSubBuffer(buffer, CL_MEM_COPY_HOST_PTR, CL_BUFFER_CREATE_TYPE_REGION, buffer_create_info , &errcode_ret);
            break;
            /*        case WebCL::MEM_HOST_WRITE_ONLY:
                      result_sub_buffer = clCreateSubBuffer(buffer, CL_MEM_HOST_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION, buffer_create_info , &errcode_ret);
                      break;
                      case WebCL::MEM_HOST_READ_ONLY:
                      result_sub_buffer = clCreateSubBuffer(buffer, CL_MEM_HOST_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, buffer_create_info , &errcode_ret);
                      break;
                      case WebCL::MEM_HOST_NO_ACCESS:
                      result_sub_buffer = clCreateSubBuffer(buffer, CL_MEM_HOST_NO_ACCESS, CL_BUFFER_CREATE_TYPE_REGION, buffer_create_info , &errcode_ret);
                      break;*/
        case 0: 
            result_sub_buffer = clCreateSubBuffer( buffer,0,CL_BUFFER_CREATE_TYPE_REGION,buffer_create_info,&errcode_ret);
            break;
        default:
            printf("Error: Unsupported Mem Flag\n");
            ec = WebCLException::INVALID_CONTEXT;
            break;
    }
    if (errcode_ret != CL_SUCCESS) {
        printf("Error: clCreateBuffer\n");
        switch(errcode_ret){
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT \n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_BUFFER_SIZE:
                printf("Error: CL_INVALID_BUFFER_SIZE\n");
                ec = WebCLException::INVALID_BUFFER_SIZE;
                break;
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                printf("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n");
                ec = WebCLException::MEM_OBJECT_ALLOCATION_FAILURE;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }// Switch
    }
    else
    {
        //Successful SubBuffer created
        RefPtr<WebCLBuffer> result_mem_ptr = WebCLBuffer::create(m_context, result_sub_buffer , false);
        return result_mem_ptr;
    }
    return NULL;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
