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
#include <wtf/Uint8ClampedArray.h>

namespace WebCore {

void WebCLHTMLInterop::extractDataFromCanvas(HTMLCanvasElement* canvas, void** hostPtr, size_t& canvasSize)
{
    if (!canvas)
        return;
    RefPtr<ImageData> imageData;
#if USE(CG)
    // If CoreGraphics usage is enabled, use it to retrive the data.
    // try to get ImageData first, as that may avoid lossy conversions.
    imageData = canvas->getImageData();
#endif
    if (imageData) {
        *hostPtr = imageData->data()->data();
        canvasSize = imageData->data()->length();
    } else {
        ImageBuffer* buffer = canvas->buffer();
        IntRect rect(0, 0, canvas->width(), canvas->height());
        RefPtr<Uint8ClampedArray> clampedArray = buffer->getUnmultipliedImageData(rect);
        if (!clampedArray)
            return;
        *hostPtr = clampedArray->data();
        canvasSize = clampedArray->byteLength();
    }
}

void WebCLHTMLInterop::extractDataFromImage(HTMLImageElement* image, void** hostPtr, size_t& imageSize)
{
    if (!image || !image->cachedImage())
        return;

    CachedImage* cachedImage = image->cachedImage();
    if (!cachedImage || !cachedImage->image() || !cachedImage->image()->data())
        return;
    unsigned sourcePixelFormat = 4; // source pixel format is treated as 32-bit(4 byte) RGBA regardless of the source.
    *hostPtr = (void*) cachedImage->image()->data()->data();
    imageSize = image->width() * image->height() * sourcePixelFormat;
}

}

#endif
