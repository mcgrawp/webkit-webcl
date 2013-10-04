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

#include "WebCLImage.h"

#include "WebCLContext.h"
#include "WebCLImageDescriptor.h"

namespace WebCore {

WebCLImage::~WebCLImage()
{
}

PassRefPtr<WebCLImage> WebCLImage::create(WebCLContext* context, CCenum flags, CCuint width, CCuint height, CCuint rowPitch, const CCImageFormat& format, void* data, ExceptionCode& ec)
{
    CCerror createImage2DError;
    PlatformComputeObject ccMemoryImage = context->computeContext()->createImage2D(flags, width, height, rowPitch, format, data, createImage2DError);
    if (createImage2DError != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(createImage2DError);
        return 0;
    }

    return adoptRef(new WebCLImage(context, ccMemoryImage, width, height, format));
}

WebCLImage::WebCLImage(WebCLContext* context, PlatformComputeObject image, CCuint width, CCuint height, const CCImageFormat& format)
    : WebCLMemoryObject(context, image)
    , m_width(width)
    , m_height(height)
    , m_format(format)
{
}

PassRefPtr<WebCLImageDescriptor> WebCLImage::getInfo(ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }

    RefPtr<WebCLImageDescriptor> imageDescriptor = WebCLImageDescriptor::create(m_format);
    imageDescriptor->setWidth(m_width);
    imageDescriptor->setHeight(m_height);

    return imageDescriptor.release();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
