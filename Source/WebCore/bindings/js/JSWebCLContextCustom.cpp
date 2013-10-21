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

#include "JSWebCLContext.h"

#include "JSArrayBuffer.h"
#include "JSArrayBufferView.h"
#include "JSHTMLCanvasElement.h"
#include "JSHTMLImageElement.h"
#include "JSHTMLVideoElement.h"
#include "JSImageData.h"
#include "JSWebCLCustom.h"

using namespace JSC;
using namespace std;

namespace WebCore {

class WebCLGetInfo;

JSValue JSWebCLContext::getInfo(JSC::ExecState* exec)
{
    if (exec->argumentCount() != 1)
        return throwSyntaxError(exec);

    ExceptionCode ec = 0;
    WebCLContext* context = static_cast<WebCLContext*>(impl());
    if (exec->hadException())
        return jsUndefined();
    unsigned contextInfo  = exec->argument(0).toInt32(exec);
    if (exec->hadException())
        return jsUndefined();
    WebCLGetInfo info = context->getInfo(contextInfo, ec);
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }
    return toJS(exec, globalObject(), info);
}

// JSDictionary helper functions
static inline void setChannelOrder(WebCLImageDescriptor* descriptor, const long& channelOrder)
{
    descriptor->setChannelOrder(channelOrder);
}

static inline void setChannelType(WebCLImageDescriptor* descriptor, const long& channelType)
{
    descriptor->setChannelType(channelType);
}

static inline void setWidth(WebCLImageDescriptor* descriptor, const unsigned long& width)
{
    descriptor->setWidth(width);
}

static inline void setHeight(WebCLImageDescriptor* descriptor, const unsigned long& height)
{
    descriptor->setHeight(height);
}

static inline void setRowPitch(WebCLImageDescriptor* descriptor, const unsigned long& rowPitch)
{
    descriptor->setRowPitch(rowPitch);
}

JSValue JSWebCLContext::createImage(JSC::ExecState* exec)
{
    if (!(exec->argumentCount() == 2 || exec->argumentCount() == 3))
        return throwSyntaxError(exec);

    unsigned flags = exec->argument(0).toInt32(exec);

    ExceptionCode ec = 0;
    RefPtr<WebCLImage> webCLImage;

    JSValue inputData = exec->argument(1);
    if (inputData.inherits(&JSHTMLVideoElement::s_info)) {
        RefPtr<HTMLVideoElement> videoElement = toHTMLVideoElement(exec->argument(1));
        webCLImage = m_impl->createImage(flags, videoElement.get(), ec);
    } else if (inputData.inherits(&JSHTMLImageElement::s_info)) {
        RefPtr<HTMLImageElement> imageElement = toHTMLImageElement(exec->argument(1));
        webCLImage = m_impl->createImage(flags, imageElement.get(), ec);
    } else if (inputData.inherits(&JSHTMLCanvasElement::s_info)) {
        RefPtr<HTMLCanvasElement> canvasElement = toHTMLCanvasElement(exec->argument(1));
        webCLImage = m_impl->createImage(flags, canvasElement.get(), ec);
    } else if (inputData.inherits(&JSImageData::s_info)) {
        RefPtr<ImageData> imageData = toImageData(exec->argument(1));
        webCLImage = m_impl->createImage(flags, imageData.get(), ec);
    } else {
        if (!inputData.isObject())
            return throwSyntaxError(exec);

        RefPtr<WebCLImageDescriptor> webCLImageDescriptor = WebCLImageDescriptor::create();

        // Given the above test, this will always yield an object.
        JSObject* object = inputData.toObject(exec);

        // Create the dictionary wrapper from the initializer object.
        JSDictionary dictionary(exec, object);
        if (!dictionary.tryGetProperty("channelOrder", webCLImageDescriptor.get(), setChannelOrder))
            return throwSyntaxError(exec);
        if (!dictionary.tryGetProperty("channelType", webCLImageDescriptor.get(), setChannelType))
            return throwSyntaxError(exec);
        if (!dictionary.tryGetProperty("width", webCLImageDescriptor.get(), setWidth))
            return throwSyntaxError(exec);
        if (!dictionary.tryGetProperty("height", webCLImageDescriptor.get(), setHeight))
            return throwSyntaxError(exec);
        if (!dictionary.tryGetProperty("rowPitch", webCLImageDescriptor.get(), setRowPitch))
            return throwSyntaxError(exec);

        RefPtr<ArrayBufferView> buffer;
        // FIXME: Spec has changed to ArrayBufferView - Issue #243.
        if (exec->argumentCount() == 3) {
            if (!exec->argument(2).inherits(&JSArrayBufferView::s_info))
                return throwSyntaxError(exec);

            buffer = toArrayBufferView(exec->argument(2));
        }

        webCLImage = m_impl->createImage(flags, webCLImageDescriptor.get(), buffer.get(), ec);
    }

    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }

    return toJS(exec, globalObject(), webCLImage.get());
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
