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
    CCerror computeContextErrorCode = m_context->computeContext()->releaseMemoryObject(m_CCMemoryObject);
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
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
        return WebCLGetInfo();
    }

    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::MEM_TYPE: {
        CCMemoryObjectype memoryType = 0;
        err = ComputeContext::getMemoryObjectInfo(m_CCMemoryObject, paramName, sizeof(CCMemoryObjectype), &memoryType);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(memoryType));
        break;
        }
    case ComputeContext::MEM_FLAGS: {
        CCMemoryFlags memoryFlags = 0;
        err = ComputeContext::getMemoryObjectInfo(m_CCMemoryObject, paramName, sizeof(CCMemoryFlags), &memoryFlags);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(memoryFlags));
        break;
        }
    case ComputeContext::MEM_SIZE: {
        size_t memorySizeValue = 0;
        err = ComputeContext::getMemoryObjectInfo(m_CCMemoryObject, paramName, sizeof(size_t), &memorySizeValue);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<size_t>(memorySizeValue));
        break;
        }
    /* FIXME:: Context must not be created here.
    case ComputeContext::MEM_CONTEXT: {
        CCContext ccContextID = 0;
        err = ComputeContext::getMemoryObjectInfo(m_platformComputeObject, paramName, sizeof(CCContext), &ccContextID);
        if (err == CL_SUCCESS) {
            RefPtr<WebCLContext> contextObject = WebCLContext::create(m_context, ccContextID);
            if(contextObject)
                return WebCLGetInfo(contextObject.release());
        }
        ec = WebCLException::INVALID_CONTEXT;
        return WebCLGetInfo();
        break;
        }
    */
    case ComputeContext::MEM_ASSOCIATED_MEMOBJECT: {
        PlatformComputeObject associatedMemoryObject = 0;
        err = ComputeContext::getMemoryObjectInfo(m_CCMemoryObject, paramName, sizeof(PlatformComputeObject), &associatedMemoryObject);
        if (err == CL_SUCCESS && associatedMemoryObject) {
            RefPtr<WebCLMemoryObject> memoryObj = WebCLMemoryObject::create(m_context, associatedMemoryObject);
            if (memoryObj)
                return WebCLGetInfo(memoryObj.release());
        }
        ec = WebCLException::INVALID_MEM_OBJECT;
        return WebCLGetInfo();
        break;
        }
    case ComputeContext::MEM_OFFSET: {
        size_t memoryOffsetValue = 0;
        err = ComputeContext::getMemoryObjectInfo(m_CCMemoryObject, paramName, sizeof(size_t), &memoryOffsetValue);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<size_t>(memoryOffsetValue));
        break;
        }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }
    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

PlatformComputeObject WebCLMemoryObject::getCLMemoryObject()
{
    return m_CCMemoryObject;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
