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

#include "JSWebCLCustom.h"

#include "JSFloat32Array.h"
#include "JSInt32Array.h"
#include "JSUint8Array.h"
#include "JSWebCL.h"
#include "JSWebCLCommandQueue.h"
#include "JSWebCLContextProperties.h"
#include "JSWebCLDevice.h"
#include "JSWebCLImageDescriptor.h"
#include "JSWebCLPlatform.h"
#include "JSWebCLProgram.h"
#include "JSWebGLRenderingContext.h"
#include "WebCLGetInfo.h"
#include "WebCLContext.h"

using namespace JSC;

namespace WebCore {

class WebCLGetInfo;
class WebCLExtension;

static void parseContextProperties(ExecState* exec, WebCLContextProperties* properties, unsigned argumentIndex, ExceptionCode& ec)
{
    JSValue value = exec->argument(argumentIndex);
    if (!value.isObject()) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    // Given the above test, this will always yield an object.
    JSObject* object = value.toObject(exec);

    Vector<RefPtr<WebCLDevice> > devices;
    Identifier devicesIdentifier(exec, "devices");
    if (object->hasProperty(exec, devicesIdentifier)) {
        JSValue value = object->get(exec, devicesIdentifier);
        if (!value.isNull()) {
            devices = toRefPtrNativeArray<WebCLDevice, JSWebCLDevice>(exec, value, &toWebCLDevice);
            if (!devices.size()) {
                ec = WebCLException::INVALID_DEVICE;
                return;
            }
            properties->setDevices(devices);
            // Finish the algorithm right way because spec says
            // the other fields are ignored if 'devices' is valid.
            return;
        }
    }

    WebCLPlatform* platform = 0;
    Identifier platformIdentifier(exec, "platform");
    if (object->hasProperty(exec, platformIdentifier)) {
        JSValue value = object->get(exec, platformIdentifier);
        if (!value.isNull()) {
            platform = toWebCLPlatform(object->get(exec, platformIdentifier));
            if (!platform) {
                ec = WebCLException::INVALID_PLATFORM;
                return;
            }
            properties->setPlatform(platform);
        }
    }

    unsigned deviceType = ComputeContext::DEVICE_TYPE_DEFAULT;
    Identifier deviceTypeIdentifier(exec, "deviceType");
    if (object->hasProperty(exec, deviceTypeIdentifier)) {
        JSValue value = object->get(exec, deviceTypeIdentifier);
        if (!value.isNull()) {
            deviceType = value.toUInt32(exec);
            if (!WebCLInputChecker::isValidDeviceType(deviceType)) {
                ec = WebCLException::INVALID_DEVICE_TYPE;
                return;
            }
            properties->setDeviceType(deviceType);
        }
    }
}

// If there is one argument, we have to figure out if it is
// a WebGLRenderingContext or a WebCLContextProperties (dictionary).
static void handleOneArgument(ExecState* exec, WebCLContextProperties* properties, ExceptionCode& ec)
{
    unsigned argumentCount = exec->argumentCount();
    ASSERT_UNUSED(argumentCount, argumentCount == 1);

    if (exec->argument(0).inherits(&JSWebGLRenderingContext::s_info)) {
        RefPtr<WebGLRenderingContext> gl = toWebGLRenderingContext(exec->argument(0));
        properties->setGLContext(gl.get());
        return;
    }

    parseContextProperties(exec, properties, 0 /*index*/, ec);
}

// If there is two arguments, the first one has to be a valid non-null WebGLRenderingContext.
// Second is a possibly-null WebCLContextProperties (dictionary).
static void handleTwoArguments(ExecState* exec, WebCLContextProperties* properties, ExceptionCode& ec)
{
    unsigned argumentCount = exec->argumentCount();
    ASSERT_UNUSED(argumentCount, argumentCount == 2);

    JSValue argument0 = exec->argument(0);
    if (!argument0.inherits(&JSWebGLRenderingContext::s_info))
        return;

    RefPtr<WebGLRenderingContext> gl = toWebGLRenderingContext(argument0);
    properties->setGLContext(gl.get());

    parseContextProperties(exec, properties, 1 /*index*/, ec);
}

JSValue JSWebCL::createContext(ExecState* exec)
{
    unsigned argumentCount = exec->argumentCount();
    if (!(!argumentCount || argumentCount == 1 || argumentCount == 2))
        return throwSyntaxError(exec);

    ExceptionCode ec = 0;
    RefPtr<WebCLContextProperties> properties = WebCLContextProperties::create();

    if (argumentCount == 1)
        handleOneArgument(exec, properties.get(), ec);
    else if (argumentCount == 2)
        handleTwoArguments(exec, properties.get(), ec);

    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }

    RefPtr<WebCLContext> webCLContext = impl()->createContext(properties.get(), ec);
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }
    return toJS(exec, globalObject(), webCLContext.get());
}

JSValue JSWebCL::getSupportedExtensions(ExecState* exec)
{
    WebCL* cl = static_cast<WebCL*>(impl());
    ExceptionCode ec = 0;
    Vector<String> value = cl->getSupportedExtensions(ec);
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }

    MarkedArgumentBuffer list;
    for (size_t i = 0; i < value.size(); ++i) {
        // FIXME: use jsStringWithCache?
        list.append(jsString(exec, value[i]));
    }
    return constructArray(exec, 0, globalObject(), list);
}

JSValue toJS(ExecState* exec, JSDOMGlobalObject* globalObject, const WebCLGetInfo& info)
{
    switch (info.getType()) {
    case WebCLGetInfo::kTypeBool:
        return jsBoolean(info.getBool());
    case WebCLGetInfo::kTypeBoolArray: {
        MarkedArgumentBuffer list;
        const Vector<bool>& value = info.getBoolArray();
        for (size_t ii = 0; ii < value.size(); ++ii)
            list.append(jsBoolean(value[ii]));
        return constructArray(exec, static_cast<ArrayAllocationProfile*>(0), list);
    }
    case WebCLGetInfo::kTypeFloat:
        return jsNumber(info.getFloat());
    case WebCLGetInfo::kTypeInt:
        return jsNumber(info.getInt());
    case WebCLGetInfo::kTypeNull:
        return jsNull();
    case WebCLGetInfo::kTypeString:
        return jsString(exec, info.getString());
    case WebCLGetInfo::kTypeUnsignedInt:
        return jsNumber(info.getUnsignedInt());
    case WebCLGetInfo::kTypeUnsignedLong:
        return jsNumber(info.getUnsignedLong());
    case WebCLGetInfo::kTypeWebCLFloatArray:
        return toJS(exec, globalObject, info.getWebCLFloatArray());
    case WebCLGetInfo::kTypeWebCLImageDescriptor:
        return toJS(exec, globalObject, info.getWebCLImageDescriptor());
    case WebCLGetInfo::kTypeWebCLIntArray:
        return toJS(exec, globalObject, info.getWebCLIntArray());
    case WebCLGetInfo::kTypeWebCLProgram:
        return toJS(exec, globalObject, info.getWebCLProgram());
    case WebCLGetInfo::kTypeWebCLContext:
        return toJS(exec, globalObject, info.getWebCLContext());
    case WebCLGetInfo::kTypeWebCLCommandQueue:
        return toJS(exec, globalObject, info.getWebCLCommandQueue());
    case WebCLGetInfo::kTypeWebCLDevice:
        return toJS(exec, globalObject, info.getWebCLDevice());
    case WebCLGetInfo::kTypeWebCLDevices:
        return jsArray(exec, globalObject, info.getWebCLDevices());
    case WebCLGetInfo::kTypeWebCLMemoryObject:
        return toJS(exec, globalObject, info.getWebCLMemoryObject());
    case WebCLGetInfo::kTypeWebCLPlatform:
        return toJS(exec, globalObject, info.getWebCLPlatform());
    case WebCLGetInfo::kTypeWebCLContextProperties:
        // FIXME: http://www.khronos.org/bugzilla/show_bug.cgi?id=1038
        // No way to recover the WebGLRenderingContext object used by cl-gl interop
        return toJS(exec, globalObject, info.getWebCLContextProperties());
    default:
        return jsUndefined();
    }
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
