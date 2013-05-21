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

#if ENABLE(WEBCL)

#include "WebCLGLContext.h"
#include "WebGLBuffer.h"
#include "WebGLRenderbuffer.h"

namespace WebCore {

PassRefPtr<WebCLGLContext> WebCLGLContext::create(PassRefPtr<WebCLContextProperties> properties, CCerror& error)
{
    RefPtr<WebCLGLContext> context;
    createBase(properties, error, context);
    return context.release();
}

WebCLGLContext::WebCLGLContext(ComputeContext* computeContext, PassRefPtr<WebCLContextProperties> properties)
    : WebCLContext(computeContext, properties)
{
}

PassRefPtr<WebCLGLCommandQueue> WebCLGLContext::createCommandQueue(WebCLDevice* device, int commandQueueProperty, ExceptionCode& ec)
{
    RefPtr<WebCLGLCommandQueue> queue;
    createCommandQueueBase(device, commandQueueProperty, ec, queue);
    return queue.release();
}

PassRefPtr<WebCLGLBuffer> WebCLGLContext::createFromGLBuffer(int flags, WebGLBuffer* webGLBuffer, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::FAILURE;
        return 0;
    }

    if (!webGLBuffer || !webGLBuffer->object()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    Platform3DObject bufferID = webGLBuffer->object();
    return WebCLGLBuffer::create(this, flags, bufferID, ec);
}

PassRefPtr<WebCLGLImage> WebCLGLContext::createFromGLRenderBuffer(int flags, WebGLRenderbuffer* renderBuffer, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    GLuint renderBufferID = 0;
    if (renderBuffer)
        renderBufferID = renderBuffer->getInternalFormat();

    if (!renderBufferID) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    GC3Dsizei width = renderBuffer->getWidth();
    GC3Dsizei height = renderBuffer->getHeight();

    // FIXME: Format is wrong here. It should have been gotten from WebGLRenderbuffer as well.
    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return WebCLGLImage::create(this, flags, width, height, imageFormat, renderBufferID, ec);
}

PassRefPtr<WebCLMemoryObject> WebCLGLContext::createFromGLTexture2D(int flags, GC3Denum textureTarget, GC3Dint miplevel, GC3Duint texture, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    CCerror error;
    PlatformComputeObject ccMemoryImage = platformObject()->createFromGLTexture2D(flags, textureTarget, miplevel, texture, error);
    if (!ccMemoryImage) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLMemoryObject> memoryObject = WebCLMemoryObject::create(this, ccMemoryImage);
    return memoryObject.release();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
