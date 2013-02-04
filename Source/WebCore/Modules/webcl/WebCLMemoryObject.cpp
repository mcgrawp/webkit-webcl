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

#include "WebCLMemoryObject.h"

#include "WebCL.h"
#include "WebCLGLObjectInfo.h"

namespace WebCore {

WebCLMemoryObject::~WebCLMemoryObject()
{
    ASSERT(m_CCMemoryObject);
    CCerror err = clReleaseMemObject(m_CCMemoryObject);
    if (err != CL_SUCCESS) {
        switch (err) {
        case CL_INVALID_MEM_OBJECT:
            printf("Error: CL_INVALID_MEM_OBJECT \n");
            break;
        case CL_OUT_OF_RESOURCES:
            printf("Error: CL_OUT_OF_RESOURCES  \n");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
            break;
        default:
            printf("Error: Invaild Error Type\n");
            break;
        }
    }
    ASSERT_UNUSED(err, err == ComputeContext::SUCCESS);
    m_CCMemoryObject = 0;
}

PassRefPtr<WebCLMemoryObject> WebCLMemoryObject::create(WebCLContext* context, PlatformComputeObject CCmem, bool isShared = false)
{
    return adoptRef(new WebCLMemoryObject(context, CCmem, isShared));
}

WebCLMemoryObject::WebCLMemoryObject(WebCLContext* context, PlatformComputeObject CCmem, bool isShared)
    : m_context(context)
    , m_CCMemoryObject(CCmem)
    , m_shared(isShared)
{
}

bool WebCLMemoryObject::isShared()
{
    return m_shared;
}

PassRefPtr<WebCLGLObjectInfo> WebCLMemoryObject::getGLObjectInfo(ExceptionCode& ec)
{
    CCerror err = -1;
    unsigned* glObjectType = 0;
    unsigned* glObjectName = 0;
    if (!m_CCMemoryObject) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        printf("Error: Invalid MEM_OBJECT\n");
        return 0;
    }
    err = clGetGLObjectInfo(m_CCMemoryObject, glObjectType, glObjectName);
    if (err != CL_SUCCESS) {
        switch (err) {
        case CL_INVALID_MEM_OBJECT:
            printf("Error: CL_INVALID_MEM_OBJECT\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            break;
        case CL_INVALID_GL_OBJECT:
            printf("Error: CL_INVALID_GL_OBJECT\n");
            ec = WebCLException::INVALID_GL_OBJECT;
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
            printf("Error: Invalid ERROR Type\n");
            ec = WebCLException::FAILURE;
            break;
        }
    } else {
        RefPtr<WebCLGLObjectInfo> CLGLObjectInfo = WebCLGLObjectInfo::create(glObjectType, glObjectName);
        if (CLGLObjectInfo)
            return CLGLObjectInfo;
        printf("Error:: Error creating WebCLGLObjectInfo object\n");
    }
    return 0;
}

WebCLGetInfo WebCLMemoryObject::getInfo(int paramName, ExceptionCode& ec)
{
    if (!m_CCMemoryObject) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        printf("Error: Invalid MEM_OBJECT.\n");
        return WebCLGetInfo();
    }

    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::MEM_TYPE: {
        CCMemObjectype memType = 0;
        err = clGetMemObjectInfo(m_CCMemoryObject, CL_MEM_TYPE, sizeof(CCMemObjectype), &memType, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(memType));
        }
        break;
    case ComputeContext::MEM_FLAGS: {
        CCMemFlags memFlags = 0;
        err = clGetMemObjectInfo(m_CCMemoryObject, CL_MEM_FLAGS, sizeof(memFlags), &memFlags, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(memFlags));
        }
        break;

    case ComputeContext::MEM_SIZE: {
        size_t memSizeValue = 0;
        err = clGetMemObjectInfo(m_CCMemoryObject, CL_MEM_SIZE, sizeof(size_t), &memSizeValue, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<size_t>(memSizeValue));
        }
        break;
    /* FIXME:: Context must not be created here.
    case ComputeContext::MEM_CONTEXT: {
        RefPtr<WebCLContext> contextObj = 0;
        cl_context clContextID = 0;
        err=clGetMemObjectInfo(m_PlatformComputeObject, CL_MEM_CONTEXT, sizeof(cl_context), &clContextID, 0);
        contextObj = WebCLContext::create(m_context, clContextID);
        if(!contextObj) {
            printf("Error: CL Mem context not NULL\n");
            return WebCLGetInfo();
        }
        if (err == CL_SUCCESS)
            return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
        }
        break;
    */
    case ComputeContext::MEM_ASSOCIATED_MEMOBJECT: {
        PlatformComputeObject associatedMemObj = 0;
        err = clGetMemObjectInfo(m_CCMemoryObject, CL_MEM_ASSOCIATED_MEMOBJECT, sizeof(associatedMemObj), &associatedMemObj, 0);
        if (err == CL_SUCCESS) {
            if (associatedMemObj) {
                RefPtr<WebCLMemoryObject> memoryObj = WebCLMemoryObject::create(m_context, associatedMemObj);
                if (!memoryObj) {
                    printf(" ERROR:: WebCLMemoryObject::getInfo WebCLMemoryObject not created\n");
                    return WebCLGetInfo();
                }
                return WebCLGetInfo(memoryObj.release());
            }
            printf("ERROR:: clGetMemObjectInfo with CL_MEM_ASSOCIATED_MEMOBJECT returned null\n");
            return WebCLGetInfo();
        }
        printf(" ERROR:: clGetMemObjectInfo:: CL_MEM_ASSOCIATED_MEMOBJECT failed\n");
        }
        break;
    case ComputeContext::MEM_OFFSET: {
        size_t memOffsetValue = 0;
        err = clGetMemObjectInfo(m_CCMemoryObject, CL_MEM_OFFSET, sizeof(size_t), &memOffsetValue, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<size_t>(memOffsetValue));
        }
        break;
    default:
        printf("Error: Unsupported Mem Info type\n");
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }
    switch (err) {
    case CL_INVALID_VALUE:
        ec = WebCLException::INVALID_VALUE;
        printf("Error: CL_INVALID_VALUE   \n");
        break;
    case CL_INVALID_MEM_OBJECT:
        ec = WebCLException::INVALID_MEM_OBJECT;
        printf("Error: CL_INVALID_MEM_OBJECT    \n");
        break;
    case CL_OUT_OF_RESOURCES:
        ec = WebCLException::OUT_OF_RESOURCES;
        printf("Error: CL_OUT_OF_RESOURCES   \n");
        break;
    case CL_OUT_OF_HOST_MEMORY:
        ec = WebCLException::OUT_OF_HOST_MEMORY;
        printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
        break;
    default:
        ec = WebCLException::FAILURE;
        printf("Error: Invaild Error Type\n");
        break;
    }
    return WebCLGetInfo();
}

PlatformComputeObject WebCLMemoryObject::getCLMemoryObject()
{
    return m_CCMemoryObject;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
