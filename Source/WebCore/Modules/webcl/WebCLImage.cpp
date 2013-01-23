/*
* Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
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

PassRefPtr<WebCLImage> WebCLImage::create(WebCL* context, PlatformComputeObject image, bool isShared = false)
{
    return adoptRef(new WebCLImage(context, image, isShared));
}

WebCLImage::WebCLImage(WebCL* context, PlatformComputeObject image, bool isShared)
    : WebCLMemoryObject(context, image, isShared)
{
}

PlatformComputeObject WebCLImage::getCLImage()
{
	return m_cl_mem;
}

int WebCLImage::getGLtextureInfo(int paramNameobj, ExceptionCode& ec)
{
    CCint err = 0;
    CCint intUnits = 0;
    if (!m_CCMemoryObject) {
        printf("Error: Invalid CL Memory Object \n");
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }
    switch (paramNameobj) {
    case WebCL::TEXTURE_TARGET:
        err = clGetGLTextureInfo(m_CCMemoryObject, CL_GL_TEXTURE_TARGET, sizeof(CCint), &intUnits, 0);
        if (err == CL_SUCCESS)
            return ((int)intUnits);
        break;
    case WebCL::MIPMAP_LEVEL:
        err = clGetGLTextureInfo(m_CCMemoryObject, CL_GL_MIPMAP_LEVEL, sizeof(CCint), &intUnits, 0);
        if (err == CL_SUCCESS)
            return ((int)intUnits);
        break;
    default:
        printf("Error: Unsupported paramName Info type = %d ", paramNameobj);
        return 0;
    }
    if (err != CL_SUCCESS) {
        switch (err) {
        case CL_INVALID_MEM_OBJECT:
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error: CL_INVALID_MEM_OBJECT  \n");
            break;
        case CL_INVALID_GL_OBJECT:
            ec = WebCLException::INVALID_GL_OBJECT;
            printf("Error: CL_INVALID_GL_OBJECT \n");
            break;
        case CL_INVALID_VALUE:
            ec = WebCLException::INVALID_VALUE;
            printf("Error: CL_INVALID_VALUE \n");
            break;
        case CL_OUT_OF_RESOURCES:
            ec = WebCLException::OUT_OF_RESOURCES;
            printf("Error: CL_OUT_OF_RESOURCES \n");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            ec = WebCLException::OUT_OF_HOST_MEMORY;
            printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
            break;
        default:
            ec = WebCLException::FAILURE;
            printf("Invaild Error Type\n");
            break;
        }
    }
    return 0;
}

PassRefPtr<WebCLImageDescriptor> WebCLImage::getInfo(ExceptionCode& ec)
{
    CCerror err = 0;
    CCint intUnits = 0;
    long channelOrder = 0;
    long channelType = 0;
    long width = 0;
    long height = 0;
    long rowPitch = 0;
    int iflag = 0;

    if (!m_CCMemoryObject) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_MEM_OBJECT;
        return 0;
    }
    err = clGetImageInfo(m_CCMemoryObject, CL_IMAGE_FORMAT, sizeof(CCImageFormat), &intUnits, 0);
    if (err == CL_SUCCESS) {
        channelOrder = (long)intUnits;
        iflag = 1;
    }
    err = clGetImageInfo(m_CCMemoryObject, CL_IMAGE_ELEMENT_SIZE, sizeof(size_t), &intUnits, 0);
    if (err == CL_SUCCESS && iflag ==1)
        channelType = (long)intUnits;
    else
        iflag = 0;

    err = clGetImageInfo(m_CCMemoryObject, CL_IMAGE_WIDTH, sizeof(size_t), &intUnits, 0);
    if (err == CL_SUCCESS && iflag ==1)
        width = (long)intUnits;
    else
        iflag = 0;

    err = clGetImageInfo(m_CCMemoryObject, CL_IMAGE_HEIGHT, sizeof(size_t), &intUnits, 0);
    if (err == CL_SUCCESS && iflag ==1)
        height = (long)intUnits;
    else
        iflag = 0;

    err = clGetImageInfo(m_CCMemoryObject, CL_IMAGE_ROW_PITCH, sizeof(size_t), &intUnits, 0);
    if (err == CL_SUCCESS && iflag ==1)
        rowPitch = (long)intUnits;
    else
        iflag = 0;

    if (err == CL_SUCCESS && iflag == 1) {
        RefPtr<WebCLImageDescriptor> objectWebCLImageDescriptor = WebCLImageDescriptor::create();
        objectWebCLImageDescriptor->setChannelOrder(channelOrder);
        objectWebCLImageDescriptor->setChannelType(channelType);
        objectWebCLImageDescriptor->setWidth(width);
        objectWebCLImageDescriptor->setHeight(height);
        objectWebCLImageDescriptor->setRowPitch(rowPitch);
        // FIXME :: Returning null as its failing by mem issue. Need to Fix asap
        return objectWebCLImageDescriptor.release();
    }
    switch (err) {
    case CL_INVALID_MEM_OBJECT:
        ec = WebCLException::INVALID_MEM_OBJECT;
        printf("Error: CL_INVALID_MEM_OBJECT  \n");
        break;
    case CL_INVALID_GL_OBJECT:
        ec = WebCLException::INVALID_GL_OBJECT;
        printf("Error: CL_INVALID_GL_OBJECT \n");
        break;
    case CL_INVALID_VALUE:
        ec = WebCLException::INVALID_VALUE;
        printf("Error: CL_INVALID_VALUE \n");
        break;
    case CL_OUT_OF_RESOURCES:
        ec = WebCLException::OUT_OF_RESOURCES;
        printf("Error: CL_OUT_OF_RESOURCES \n");
        break;
    case CL_OUT_OF_HOST_MEMORY:
        ec = WebCLException::OUT_OF_HOST_MEMORY;
        printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
        break;
    default:
        ec = WebCLException::FAILURE;
        printf("Invaild Error Type\n");
        break;
    }
    return 0;
}
} // namespace WebCore

#endif // ENABLE(WEBCL)
