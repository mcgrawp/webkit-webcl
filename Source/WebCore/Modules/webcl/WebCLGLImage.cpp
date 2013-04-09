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

#include "WebCLGLImage.h"

#include "WebCLContext.h"

namespace WebCore {

PassRefPtr<WebCLGLImage> WebCLGLImage::create(WebCLContext* context, int flags, GC3Dsizei width, GC3Dsizei height, const CCImageFormat& format, GLuint renderBufferID, ExceptionCode& ec)
{
    CCerror error;
    PlatformComputeObject ccMemoryID = context->computeContext()->createFromGLRenderbuffer(flags, renderBufferID, error);
    if (!ccMemoryID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLGLImage> imageObject = adoptRef(new WebCLGLImage(context, ccMemoryID, width, height, format));
    return imageObject.release();
}

WebCLGLImage::WebCLGLImage(WebCLContext* context, PlatformComputeObject image, int width, int height, const CCImageFormat& format)
    : WebCLImage(context, image, width, height, format)
{
}

int WebCLGLImage::getGLTextureInfo(int textureInfoType, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }

    CCint err = 0;
    switch (textureInfoType) {
    case ComputeContext::GL_TEXTURE_TARGET:
    case ComputeContext::GL_MIPMAP_LEVEL: {
        CCint glTextureInfo = 0;
        err = ComputeContext::getGLTextureInfo(platformObject(), textureInfoType, &glTextureInfo);
        if (err == CL_SUCCESS)
            return ((int)glTextureInfo);
        break;
    }
    // FIXME: InputChecker class should perform this validation.
    default:
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return 0;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
