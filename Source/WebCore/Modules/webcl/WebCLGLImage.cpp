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

#include "WebCLGLImage.h"

#include "WebCLGLContext.h"
#include "WebCLGLObjectInfo.h"
#include "WebGLRenderbuffer.h"
#include "WebGLTexture.h"

namespace WebCore {

PassRefPtr<WebCLGLImage> WebCLGLImage::create(WebCLGLContext* context, int flags, WebGLRenderbuffer* webGLRenderbuffer, ExceptionCode& ec)
{
    ASSERT(webGLRenderbuffer);

    GLuint renderbufferID = webGLRenderbuffer->object();
    if (!renderbufferID) {
        ec = WebCLException::INVALID_GL_OBJECT;
        return 0;
    }

    GC3Dsizei width = webGLRenderbuffer->getWidth();
    GC3Dsizei height = webGLRenderbuffer->getHeight();

    // FIXME: Format is wrong here. It should have been gotten from WebGLRenderbuffer as well.
    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};

    CCerror error;
    PlatformComputeObject ccMemoryID = context->computeContext()->createFromGLRenderbuffer(flags, renderbufferID, error);
    if (!ccMemoryID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLGLImage> imageObject = adoptRef(new WebCLGLImage(context, ccMemoryID, width, height, imageFormat));
    imageObject->cacheGLObjectInfo(ComputeContext::GL_OBJECT_RENDERBUFFER, webGLRenderbuffer);
    return imageObject.release();
}

PassRefPtr<WebCLGLImage> WebCLGLImage::create(WebCLGLContext* context, int flags, int textureTarget, int miplevel,
                                              WebGLTexture* webGLTexture, ExceptionCode& ec)
{
    ASSERT(webGLTexture);
    GLuint textureID = webGLTexture->object();
    if (!textureID) {
        ec = WebCLException::INVALID_GL_OBJECT;
        return 0;
    }

    GC3Dsizei width = webGLTexture->getWidth(textureTarget, miplevel);
    GC3Dsizei height = webGLTexture->getHeight(textureTarget, miplevel);

    // FIXME: Format is wrong here. It should have been gotten from WebGLTexture as well.
    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};

    CCerror error;
    PlatformComputeObject ccMemoryID = context->computeContext()->createFromGLTexture2D(flags, textureTarget, miplevel, textureID, error);
    if (!ccMemoryID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLGLImage> imageObject = adoptRef(new WebCLGLImage(context, ccMemoryID, width, height, imageFormat));
    imageObject->cacheGLObjectInfo(ComputeContext::GL_OBJECT_TEXTURE2D, webGLTexture);
    return imageObject.release();
}

WebCLGLImage::WebCLGLImage(WebCLGLContext* context, PlatformComputeObject image, int width, int height, const CCImageFormat& format)
    : WebCLImage(context, image, width, height, format)
{
}

int WebCLGLImage::getGLTextureInfo(int textureInfoType, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }

    if (!WebCLInputChecker::isValidGLTextureInfo(textureInfoType)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    // FIXME: Is a WebCLGLImage was created from a WebGLRenderbuffer instead of WebGLTexture,
    // what should happen here?
    // http://www.khronos.org/bugzilla/show_bug.cgi?id=940

    CCint err = 0;
    switch (textureInfoType) {
    case ComputeContext::GL_TEXTURE_TARGET:
    case ComputeContext::GL_MIPMAP_LEVEL: {
        CCint glTextureInfo = 0;
        err = ComputeContext::getGLTextureInfo(platformObject(), textureInfoType, &glTextureInfo);
        if (err == ComputeContext::SUCCESS)
            return glTextureInfo;
        break;
    }
    }

    ASSERT(err != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return 0;
}

void WebCLGLImage::cacheGLObjectInfo(int type, WebGLObject* glObject)
{
    m_objectInfo = WebCLGLObjectInfo::create(type, glObject);
}

WebCLGLObjectInfo* WebCLGLImage::getGLObjectInfo(ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }

    return m_objectInfo.get();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
