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

#ifndef WebCLGLContext_h
#define WebCLGLContext_h

#if ENABLE(WEBCL)

#include "WebCLContext.h"

#include "WebCLGLBuffer.h"
#include "WebCLGLCommandQueue.h"
#include "WebCLGLImage.h"

namespace WebCore {

class WebGLTexture;

class WebCLGLContext : public WebCLContext {
public:
    virtual ~WebCLGLContext() { }
    static PassRefPtr<WebCLGLContext> create(PassRefPtr<WebCLContextProperties> properties, CCerror& error);

    PassRefPtr<WebCLGLCommandQueue> createCommandQueue(WebCLDevice*, int, ExceptionCode&);
    PassRefPtr<WebCLGLCommandQueue> createCommandQueue(WebCLDevice* devices, ExceptionCode& ec)
    {
        return createCommandQueue(devices, 0, ec);
    }
    PassRefPtr<WebCLGLCommandQueue> createCommandQueue(ExceptionCode& ec)
    {
        return createCommandQueue(0, 0, ec);
    }
    PassRefPtr<WebCLGLCommandQueue> createCommandQueue(int buildProp, ExceptionCode& ec)
    {
        return createCommandQueue(0, buildProp, ec);
    }

    PassRefPtr<WebCLGLBuffer> createFromGLBuffer(int, WebGLBuffer*, ExceptionCode&);
    PassRefPtr<WebCLGLImage> createFromGLRenderbuffer(int, WebGLRenderbuffer*, ExceptionCode&);
    PassRefPtr<WebCLGLImage> createFromGLTexture(int memoryFlags, int textureTarget, GC3Dint miplevel, WebGLTexture*, ExceptionCode&);

private:
    WebCLGLContext(ComputeContext*, PassRefPtr<WebCLContextProperties>);

    friend class WebCLContext;
};

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCLContext_h
