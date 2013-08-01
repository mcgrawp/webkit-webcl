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

#ifndef JSWebCLCustom_h
#define JSWebCLCustom_h

#if ENABLE(WEBCL)

#include "JSWebCLContext.h"
#include "JSWebCLDevice.h"
#include "JSWebCLPlatform.h"
#include "JSWebCLGLContext.h"
#include "JSWebGLRenderingContext.h"

#include <runtime/JSFunction.h>

using namespace JSC;

namespace WebCore {

class WebCLExtension;

JSValue toJS(ExecState*, JSDOMGlobalObject*, const WebCLGetInfo&);
JSValue toJS(ExecState*, JSDOMGlobalObject*, WebCLExtension*);

template <class T, class U>
inline JSValue parsePropertiesAndCreateContext(ExecState* exec, JSDOMGlobalObject* globalObject, T* jsCustomObject, bool clGLSharing)
{
    unsigned argumentCount = exec->argumentCount();
    if (!(!argumentCount || argumentCount == 1))
        return throwSyntaxError(exec);

    ExceptionCode ec = 0;
    RefPtr<U> webCLContext;
    if (!argumentCount) {
        webCLContext = jsCustomObject->impl()->createContext(0, ec);
        if (ec) {
            setDOMException(exec, ec);
            return jsUndefined();
        }
        return toJS(exec, globalObject, webCLContext.get());
    }

    ASSERT(exec->argumentCount() == 1);
    if (!exec->argument(0).isObject())
        return throwSyntaxError(exec);

    JSObject* jsAttrs = exec->argument(0).getObject();

    WebCLPlatform* platform = 0;
    Identifier platformIdentifier(exec, "platform");
    if (jsAttrs->hasProperty(exec, platformIdentifier))
        platform = toWebCLPlatform(jsAttrs->get(exec, platformIdentifier));

    Vector<RefPtr<WebCLDevice> > devices;
    Identifier devicesIdentifier(exec, "devices");
    if (jsAttrs->hasProperty(exec, devicesIdentifier))
        devices = toRefPtrNativeArray<WebCLDevice, JSWebCLDevice>(exec, jsAttrs->get(exec, devicesIdentifier), &toWebCLDevice);

    int deviceType = ComputeContext::DEVICE_TYPE_DEFAULT;
    Identifier deviceTypeIdentifier(exec, "deviceType");
    if (jsAttrs->hasProperty(exec, deviceTypeIdentifier))
        deviceType = jsAttrs->get(exec, deviceTypeIdentifier).toInt32(exec);

    WebGLRenderingContext* webGLRenderingContext = 0;
    WebCLContextProperties::SharedContextResolutionPolicy contextPolicy = clGLSharing ?
        WebCLContextProperties::GetCurrentGLContext : WebCLContextProperties::NoSharedGLContext;
    Identifier sharedContextIdentifier(exec, "sharedContext");
    if (clGLSharing && jsAttrs->hasProperty(exec, sharedContextIdentifier)) {
        webGLRenderingContext = toWebGLRenderingContext(jsAttrs->get(exec, sharedContextIdentifier));
        contextPolicy = webGLRenderingContext ? WebCLContextProperties::UseGLContextProvided : WebCLContextProperties::GetCurrentGLContext;
    }

    RefPtr<WebCore::WebCLContextProperties> webCLContextProperties = WebCLContextProperties::create(platform, devices, deviceType, contextPolicy, webGLRenderingContext);

    webCLContext = jsCustomObject->impl()->createContext(webCLContextProperties.get(), ec);
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }

    return toJS(exec, globalObject, webCLContext.get());
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif
