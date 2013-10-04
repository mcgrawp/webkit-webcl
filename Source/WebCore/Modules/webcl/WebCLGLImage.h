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

#ifndef WebCLGLImage_h
#define WebCLGLImage_h

#if ENABLE(WEBCL) && ENABLE(WEBGL)

#include "WebCLImage.h"

namespace WebCore {

class WebCLGLContext;
class WebCLGLObjectInfo;
class WebGLObject;
class WebGLRenderbuffer;
class WebGLTexture;

class WebCLGLImage : public WebCLImage {
public:
    virtual ~WebCLGLImage() { }

    static PassRefPtr<WebCLGLImage> create(WebCLGLContext*, int flags, WebGLRenderbuffer* webGLRenderbuffer, ExceptionCode&);
    static PassRefPtr<WebCLGLImage> create(WebCLGLContext*, int flags, int textureTarget, int miplevel, WebGLTexture*, ExceptionCode&);

    WebCLGLObjectInfo* getGLObjectInfo(ExceptionCode&);
    int getGLTextureInfo(int paramName, ExceptionCode&);
    virtual bool isShared() const { return true; }

private:
    WebCLGLImage(WebCLGLContext*, PlatformComputeObject image, int width, int height, const CCImageFormat&);
    void cacheGLObjectInfo(int type, WebGLObject*);
    RefPtr<WebCLGLObjectInfo> m_objectInfo;
};

} // namespace WebCore

#endif
#endif // WebCLGLImage_h
