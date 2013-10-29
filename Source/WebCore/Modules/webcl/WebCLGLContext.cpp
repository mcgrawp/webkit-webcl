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

#include "WebCLGLContext.h"

#include "WebCLGLBuffer.h"
#include "WebCLGLCommandQueue.h"
#include "WebCLGLImage.h"
#include "WebCLImageDescriptor.h"
#include "WebCLProgram.h"
#include "WebGLBuffer.h"
#include "WebGLRenderbuffer.h"
#include "WebGLTexture.h"

namespace WebCore {

PassRefPtr<WebCLGLContext> WebCLGLContext::create(PassRefPtr<WebCLContextProperties> properties, ExceptionCode& ec)
{
    RefPtr<WebCLGLContext> context;
    createBase(properties, ec, context);
    return context.release();
}

WebCLGLContext::WebCLGLContext(ComputeContext* computeContext, PassRefPtr<WebCLContextProperties> properties)
    : WebCLContext(computeContext, properties)
{
}

PassRefPtr<WebCLGLCommandQueue> WebCLGLContext::createCommandQueue(WebCLDevice* device, CCenum commandQueueProperty, ExceptionCode& ec)
{
    RefPtr<WebCLGLCommandQueue> queue;
    createCommandQueueBase(device, commandQueueProperty, ec, queue);
    return queue.release();
}

PassRefPtr<WebCLGLBuffer> WebCLGLContext::createFromGLBuffer(CCenum flags, WebGLBuffer* webGLBuffer, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!webGLBuffer || !webGLBuffer->object()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    return WebCLGLBuffer::create(this, flags, webGLBuffer, ec);
}

PassRefPtr<WebCLGLImage> WebCLGLContext::createFromGLRenderbuffer(CCenum flags, WebGLRenderbuffer* renderbuffer, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!renderbuffer) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    return WebCLGLImage::create(this, flags, renderbuffer, ec);
}

PassRefPtr<WebCLGLImage> WebCLGLContext::createFromGLTexture(CCenum flags, CCenum textureTarget, GC3Dint miplevel, WebGLTexture* texture, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!texture) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    return WebCLGLImage::create(this, flags, textureTarget, miplevel, texture, ec);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
