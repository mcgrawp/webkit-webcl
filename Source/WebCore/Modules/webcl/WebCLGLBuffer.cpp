/*
 * Copyright (C) 2013 Samsung Electronics Corporation. All rights reserved.
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

#if ENABLE(WEBCL) && ENABLE(WEBGL)

#include "WebCLGLBuffer.h"

#include "WebCLContext.h"
#include "WebCLGLObjectInfo.h"
#include "WebGLBuffer.h"

namespace WebCore {

PassRefPtr<WebCLGLBuffer> WebCLGLBuffer::create(WebCLContext* context, CCenum memoryFlags, WebGLBuffer* webGLBuffer, ExceptionCode& ec)
{
    Platform3DObject platform3DObject = webGLBuffer->object();
    ASSERT(platform3DObject);
    CCerror error = ComputeContext::SUCCESS;
    PlatformComputeObject buffer = context->computeContext()->createFromGLBuffer(memoryFlags, platform3DObject, error);
    if (!buffer) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLGLBuffer> clglBuffer = adoptRef(new WebCLGLBuffer(context, buffer));
    clglBuffer->cacheGLObjectInfo(webGLBuffer);
    return clglBuffer.release();
}

WebCLGLBuffer::WebCLGLBuffer(WebCLContext* context, PlatformComputeObject object)
    : WebCLBuffer(context, object)
{
}

void WebCLGLBuffer::cacheGLObjectInfo(WebGLBuffer* webGLBuffer)
{
    m_objectInfo = WebCLGLObjectInfo::create(ComputeContext::GL_OBJECT_BUFFER, webGLBuffer);
}

WebCLGLObjectInfo* WebCLGLBuffer::getGLObjectInfo(ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }

    return m_objectInfo.get();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
