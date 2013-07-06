/*
 * Copyright (C) 2011, 2013 Samsung Electronics Corporation. All rights reserved.
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

#include "WebCLContext.h"
#include "WebCLInputChecker.h"

namespace WebCore {

WebCLBuffer::~WebCLBuffer()
{
}

PassRefPtr<WebCLBuffer> WebCLBuffer::create(WebCLContext* context, CCenum memoryFlags, int size, void* data, ExceptionCode& ec)
{
    CCerror error = ComputeContext::SUCCESS;
    PlatformComputeObject buffer = context->computeContext()->createBuffer(memoryFlags, size, data, error);
    if (!buffer) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    return adoptRef(new WebCLBuffer(context, buffer));
}

WebCLBuffer::WebCLBuffer(WebCLContext* context, PlatformComputeObject buffer)
    : WebCLMemoryObject(context, buffer)
{
}

PassRefPtr<WebCLBuffer> WebCLBuffer::createSubBuffer(int memoryFlags, int origin, int size, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }
    if (!WebCLInputChecker::isValidMemoryObjectFlag(memoryFlags)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    CCBufferRegion* bufferCreateInfo = new CCBufferRegion();
    if (!bufferCreateInfo) {
        ec = WebCLException::OUT_OF_HOST_MEMORY;
        return 0;
    }

    bufferCreateInfo->origin = origin;
    bufferCreateInfo->size = size;

    CCerror error = 0;
    PlatformComputeObject ccSubBuffer = m_context->computeContext()->createSubBuffer(platformObject(), memoryFlags,
        ComputeContext::BUFFER_CREATE_TYPE_REGION, bufferCreateInfo, error);

    delete(bufferCreateInfo);
    bufferCreateInfo = 0;

    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLBuffer> subBuffer = adoptRef(new WebCLBuffer(m_context, ccSubBuffer));
    return subBuffer.release();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
