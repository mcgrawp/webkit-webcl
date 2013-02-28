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

#if ENABLE (WEBCL)

#include "WebCLBuffer.h"

#include "WebCL.h"

namespace WebCore {

WebCLBuffer::~WebCLBuffer()
{
}

PassRefPtr<WebCLBuffer> WebCLBuffer::create(WebCLContext* context, CCenum memoryFlags, int size, void* data, CCerror& error)
{
    PlatformComputeObject buffer = context->computeContext()->createBuffer(memoryFlags, size, data, error);
    if (!buffer) {
        ASSERT(error != ComputeContext::SUCCESS);
        return 0;
    }

    return adoptRef(new WebCLBuffer(context, buffer, false));
}

PassRefPtr<WebCLBuffer> WebCLBuffer::createFromGLBuffer(WebCLContext* context, CCenum memoryFlags, const Platform3DObject& platform3DObject, CCerror& error)
{
    PlatformComputeObject buffer = context->computeContext()->createFromGLBuffer(memoryFlags, platform3DObject, error);
    if (!buffer) {
        ASSERT(error != ComputeContext::SUCCESS);
        return 0;
    }

    return adoptRef(new WebCLBuffer(context, buffer, true));
}

WebCLBuffer::WebCLBuffer(WebCLContext* context, PlatformComputeObject buffer, bool isShared)
    : WebCLMemoryObject(context, buffer, isShared)
{
}

PlatformComputeObject WebCLBuffer::getCLBuffer()
{
    return WebCLMemoryObject::getCLMemoryObject();
}

PassRefPtr<WebCLBuffer>  WebCLBuffer::createSubBuffer(int flags, int origin, int size, ExceptionCode& ec)
{
    PlatformComputeObject buffer = getCLBuffer();
    if (!buffer) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        printf("Error:: getCLBuffer() returned NULL.\n");
        return 0;
    }

    // FIXME: This leaks!
    CCBufferRegion* bufferCreateInfo = new CCBufferRegion();
    if (!bufferCreateInfo) {
        // FIXME: FAILURE is not the best error here.
        ec = WebCLException::FAILURE;
        printf(" Error:: CCBufferRegion creation failed.\n");
        return 0;
    }

    bufferCreateInfo->origin = origin;
    bufferCreateInfo->size = size;

    CCerror error = 0;
    PlatformComputeObject resultSubBuffer = 0;

    switch (flags) {
    case ComputeContext::MEM_READ_ONLY:
    case ComputeContext::MEM_WRITE_ONLY:
    case ComputeContext::MEM_READ_WRITE:
    case 0: // FIXME: Use a named constant here, instead of 0.
        resultSubBuffer = clCreateSubBuffer(buffer, flags, ComputeContext::BUFFER_CREATE_TYPE_REGION, bufferCreateInfo, &error);
        break;
    default:
        ec = WebCLException::INVALID_VALUE;
        free(bufferCreateInfo);
        break;
    }

    if (error != CL_SUCCESS) {
        printf("Error: clCreateBuffer\n");
        switch (error) {
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
        }
    } else {
        RefPtr<WebCLBuffer> resultMemPtr = adoptRef(new WebCLBuffer(m_context, resultSubBuffer, false));
        return resultMemPtr;
    }
    return 0;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
