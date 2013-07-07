/*
 * Copyright (C) 2011, 2012, 2013 Samsung Electronics Corporation. All rights reserved.
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

#include "WebCLContext.h"

namespace WebCore {

WebCLMemoryObject::~WebCLMemoryObject()
{
    releasePlatformObject();
}

PassRefPtr<WebCLMemoryObject> WebCLMemoryObject::create(WebCLContext* context, PlatformComputeObject CCmem)
{
    return adoptRef(new WebCLMemoryObject(context, CCmem));
}

WebCLMemoryObject::WebCLMemoryObject(WebCLContext* context, PlatformComputeObject CCmem)
    : WebCLObject(CCmem)
    , m_context(context)
{
}

WebCLGetInfo WebCLMemoryObject::getInfo(int paramName, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return WebCLGetInfo();
    }

    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::MEM_TYPE: {
        CCMemoryObjectype memoryType = 0;
        err = ComputeContext::getMemoryObjectInfo(platformObject(), paramName, &memoryType);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(memoryType));
        break;
        }
    case ComputeContext::MEM_FLAGS: {
        CCMemoryFlags memoryFlags = 0;
        err = ComputeContext::getMemoryObjectInfo(platformObject(), paramName, &memoryFlags);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(memoryFlags));
        break;
        }
    case ComputeContext::MEM_SIZE: {
        size_t memorySizeValue = 0;
        err = ComputeContext::getMemoryObjectInfo(platformObject(), paramName, &memorySizeValue);
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
        err = ComputeContext::getMemoryObjectInfo(platformObject(), paramName, &associatedMemoryObject);
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
        err = ComputeContext::getMemoryObjectInfo(platformObject(), paramName, &memoryOffsetValue);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<size_t>(memoryOffsetValue));
        break;
        }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    // FIXME: Avoid accessing OpenCL constants directly.
    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

void WebCLMemoryObject::releasePlatformObjectImpl()
{
    CCerror computeContextErrorCode = m_context->computeContext()->releaseMemoryObject(platformObject());
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
