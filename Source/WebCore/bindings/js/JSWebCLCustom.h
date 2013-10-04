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

#include "JSDictionary.h"
#include "JSWebCLContext.h"
#include "JSWebCLGLContext.h"
#include "JSWebCLImage.h"
#include "WebCLCommandQueue.h"
#include "WebCLFinishCallback.h"
#include "WebCLGLContextProperties.h"
#include "WebCLImageDescriptor.h"
#include "WebCLProgram.h"

using namespace JSC;


namespace WebCore {

class WebCLExtension;
class WebCLGetInfo;
class DOMGlobalObject;

JSValue toJS(ExecState*, JSDOMGlobalObject*, const WebCLGetInfo&);
JSValue toJS(ExecState*, JSDOMGlobalObject*, WebCLExtension*);

// JSDictionary helper functions
static inline void setPlatform(WebCLContextProperties* properties, const RefPtr<WebCLPlatform>& platform)
{
    properties->setPlatform(platform);
}

static inline void setDevices(WebCLContextProperties* properties, const Vector<RefPtr<WebCLDevice> >& devices)
{
    properties->setDevices(devices);
}

static inline void setDeviceType(WebCLContextProperties* properties, const unsigned long& deviceType)
{
    properties->setDeviceType(deviceType);
}

#if ENABLE(WEBGL)
static inline void setSharedContext(WebCLContextProperties* properties, const RefPtr<WebGLRenderingContext>& context)
{
    properties->setSharedContext(context);
}
#endif

template <class T, class U, class V>
inline JSValue parsePropertiesAndCreateContext(ExecState* exec, JSDOMGlobalObject* globalObject, T* jsCustomObject, bool clGLSharing)
{
    unsigned argumentCount = exec->argumentCount();
    if (!(!argumentCount || argumentCount == 1))
        return throwSyntaxError(exec);

    ExceptionCode ec = 0;
    RefPtr<U> webCLContext;

    // Argument is optional (hence undefined is allowed), and null is allowed.
    JSValue value = exec->argument(0);
    if (value.isUndefinedOrNull()) {
        webCLContext = jsCustomObject->impl()->createContext(0, ec);
        if (ec) {
            setDOMException(exec, ec);
            return jsUndefined();
        }
        return toJS(exec, globalObject, webCLContext.get());
    }

    ASSERT(exec->argumentCount() == 1);
    if (!value.isObject())
        return throwSyntaxError(exec);

    // Given the above test, this will always yield an object.
    JSObject* object = value.toObject(exec);

    // Create default options.
    RefPtr<WebCLContextProperties> properties = V::create();

    // Create the dictionary wrapper from the initializer object.
    JSDictionary dictionary(exec, object);
    if (!dictionary.tryGetProperty("platform", properties.get(), setPlatform))
        return throwSyntaxError(exec);
    if (!dictionary.tryGetProperty("devices", properties.get(), setDevices))
        return throwSyntaxError(exec);
    if (!dictionary.tryGetProperty("deviceType", properties.get(), setDeviceType))
        return throwSyntaxError(exec);

#if ENABLE(WEBGL)
    if (clGLSharing) {
        // We call it with a null GL context so that internally WebCLContextProperties
        // keeps its control up to date.
        setSharedContext(properties.get(), 0 /* GL context */);
        if (!dictionary.tryGetProperty("sharedContext", properties.get(), setSharedContext))
            return throwSyntaxError(exec);
    }
#else
    ASSERT_UNUSED(clGLSharing, !clGLSharing);
#endif

    webCLContext = jsCustomObject->impl()->createContext(properties.get(), ec);
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }

    return toJS(exec, globalObject, webCLContext.get());
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif
