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

#include "WebCLHTMLInterop.h"

#include "CachedImage.h"
#include "HTMLCanvasElement.h"
#include "HTMLImageElement.h"
#include "ImageBuffer.h"
#include "ImageData.h"
#include "SharedBuffer.h"
#include <runtime/Uint8ClampedArray.h>

namespace WebCore {

void WebCLHTMLInterop::extractDataFromCanvas(HTMLCanvasElement* canvas, void*& hostPtr, size_t& canvasSize, ExceptionCode& ec)
{
    if (!canvas) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }
    Vector<uint8_t> data;
    CCerror error = ComputeContext::CCPackImageData(canvas->copiedImage(), GraphicsContext3D::HtmlDomCanvas, canvas->width(), canvas->height(), data);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return;
    }
    hostPtr = data.data();
    canvasSize = data.size();
    if (!hostPtr || !canvasSize) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }
}

void WebCLHTMLInterop::extractDataFromImage(HTMLImageElement* image, void*& hostPtr, size_t& imageSize, ExceptionCode& ec)
{
    if (!image || !image->cachedImage()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }
    Vector<uint8_t> data;
    CCerror error = ComputeContext::CCPackImageData(image->cachedImage()->image(), GraphicsContext3D::HtmlDomImage, image->width(), image->height(), data);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return;
    }
    hostPtr = data.data();
    imageSize = data.size();
    if (!hostPtr || !imageSize) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }
}

void WebCLHTMLInterop::extractDataFromImageData(ImageData* srcPixels, void*& hostPtr, size_t& pixelSize, ExceptionCode& ec)
{
    if (!srcPixels && !srcPixels->data() && !srcPixels->data()->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }
    pixelSize = srcPixels->data()->length();
    hostPtr = static_cast<void*>(srcPixels->data()->data());
    if (!hostPtr || !pixelSize) {
        ec = WebCLException::INVALID_HOST_PTR;
        return;
    }
}
}

#endif
