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

#include "WebCL.h"
#include "WebCLContext.h"

namespace WebCore {

WebCLImage::~WebCLImage()
{
}

PassRefPtr<WebCLImage> WebCLImage::create(WebCLContext* context, PlatformComputeObject image, bool isShared = false)
{
    return adoptRef(new WebCLImage(context, image, isShared));
}

WebCLImage::WebCLImage(WebCLContext* context, PlatformComputeObject image, bool isShared)
    : WebCLMemoryObject(context, image, isShared)
{
}

PlatformComputeObject WebCLImage::getCLImage()
{
    return WebCLMemoryObject::getCLMemoryObject();
}

int WebCLImage::getGLtextureInfo(int textureInfoType, ExceptionCode& ec)
{
    if (!m_CCMemoryObject) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }

    CCint err = 0;
    switch (textureInfoType) {
    case ComputeContext::GL_TEXTURE_TARGET:
    case ComputeContext::GL_MIPMAP_LEVEL: {
        CCint glTextureInfo = 0;
        err = ComputeContext::getGLtextureInfo(m_CCMemoryObject, textureInfoType, sizeof(CCint), &glTextureInfo);
        if (err == CL_SUCCESS)
            return ((int)glTextureInfo);
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }
    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return 0;
}

PassRefPtr<WebCLImageDescriptor> WebCLImage::getInfo(ExceptionCode& ec)
{
    if (!m_CCMemoryObject) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }

    int iflag = 0;
    RefPtr<WebCLImageDescriptor> objectWebCLImageDescriptor = WebCLImageDescriptor::create();
    CCerror err = 0;
    CCImageFormat imageFormat;
    err = ComputeContext::getImageInfo(m_CCMemoryObject, ComputeContext::IMAGE_FORMAT, sizeof(CCImageFormat), &imageFormat);
    if (err == CL_SUCCESS) {
        iflag = 1;
        objectWebCLImageDescriptor->setChannelOrder(imageFormat.image_channel_order);
        objectWebCLImageDescriptor->setChannelType(imageFormat.image_channel_data_type);
    }

    size_t imageInfoValue = 0;
    err = ComputeContext::getImageInfo(m_CCMemoryObject, ComputeContext::IMAGE_WIDTH, sizeof(size_t), &imageInfoValue);
    if (err == CL_SUCCESS && iflag == 1)
        objectWebCLImageDescriptor->setWidth((long)imageInfoValue);
    else
        iflag = 0;

    err = ComputeContext::getImageInfo(m_CCMemoryObject, ComputeContext::IMAGE_HEIGHT, sizeof(size_t), &imageInfoValue);
    if (err == CL_SUCCESS && iflag == 1)
        objectWebCLImageDescriptor->setHeight((long)imageInfoValue);
    else
        iflag = 0;

    err = ComputeContext::getImageInfo(m_CCMemoryObject, ComputeContext::IMAGE_ROW_PITCH, sizeof(size_t), &imageInfoValue);
    if (err == CL_SUCCESS && iflag == 1)
        objectWebCLImageDescriptor->setRowPitch((long)imageInfoValue);
    else
        iflag = 0;

    if (err == CL_SUCCESS && iflag == 1)
        return objectWebCLImageDescriptor;

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return 0;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
