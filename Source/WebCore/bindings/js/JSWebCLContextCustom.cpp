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
#include "JSWebCLCustom.h"
#include "NotImplemented.h"
#include "WebCLBuffer.h"
#include "WebCLContext.h"
#include "WebCLGetInfo.h"
#include "WebCLImageDescriptor.h"

using namespace JSC;
using namespace std;

namespace WebCore {

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
    WebCLGetInfo info = context->getInfo(contextInfo , ec);
    if (ec) {
        setDOMException(exec , ec);
        return jsUndefined();
    }
    return toJS(exec , globalObject() , info);
}

JSValue JSWebCLContext::createImageWithDescriptor(JSC::ExecState* exec)
{
    if (!exec->argumentCount())
        return throwSyntaxError(exec);

    ExceptionCode ec = 0;
    JSObject* jsAttrs = 0;
    RefPtr<ArrayBuffer> buffer = 0;
    RefPtr<WebCLMemoryObject> objWebCLImage;
    unsigned flag  = exec->argument(0).toInt32(exec);
    RefPtr<WebCore::WebCLImageDescriptor> objWebCLImageDescriptor = WebCLImageDescriptor::create();
    if (exec->argumentCount() >= 2 && exec->argument(1).isObject()) {
        jsAttrs = exec->argument(1).getObject();
        Identifier channelOrder(exec , "channelOrder");
        if (jsAttrs->hasProperty(exec , channelOrder))
            objWebCLImageDescriptor->setChannelOrder(jsAttrs->get(exec , channelOrder).toInt32(exec));

        Identifier channelType(exec , "channelType");
        if (jsAttrs->hasProperty(exec , channelType))
            objWebCLImageDescriptor->setChannelType(jsAttrs->get(exec , channelType).toInt32(exec));

        Identifier width(exec , "width");
        if (jsAttrs->hasProperty(exec , width))
            objWebCLImageDescriptor->setWidth(jsAttrs->get(exec , width).toInt32(exec));

        Identifier height(exec , "height");
        if (jsAttrs->hasProperty(exec , height))
            objWebCLImageDescriptor->setHeight(jsAttrs->get(exec , height).toInt32(exec));

        Identifier rowPitch(exec , "rowPitch");
        if (jsAttrs->hasProperty(exec , rowPitch))
            objWebCLImageDescriptor->setRowPitch(jsAttrs->get(exec , rowPitch).toInt32(exec));
    }

    if (exec->argumentCount() == 2)
        objWebCLImage = m_impl->createImageWithDescriptor(flag , objWebCLImageDescriptor.get() , ec);

    if (exec->argumentCount() == 3) {
        buffer = toArrayBuffer(exec->argument(2));
        objWebCLImage = m_impl->createImageWithDescriptor(flag , objWebCLImageDescriptor.get() , buffer.get() , ec);
    }

    if (ec) {
        setDOMException(exec , ec);
        return jsUndefined();
    }
    return toJS(exec , globalObject() , objWebCLImage.get());
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
