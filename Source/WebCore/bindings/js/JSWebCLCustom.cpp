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
#include "JSWebCLDevice.h"
#include "JSWebCLGL.h"
#include "JSWebCLGLContextProperties.h"
#include "JSWebCLImageDescriptor.h"
#include "JSWebCLPlatform.h"
#include "JSWebCLProgram.h"

using namespace JSC;

namespace WebCore {

class WebCLGetInfo;
class WebCLExtension;

JSValue JSWebCL::createContext(ExecState* exec)
{
    return parsePropertiesAndCreateContext<JSWebCL, WebCLContext, WebCLContextProperties>(exec, globalObject(), this, false /*CLGLSharing*/);
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

JSValue JSWebCL::getExtension(ExecState* exec)
{
    if (exec->argumentCount() < 1)
        return throwError(exec, createNotEnoughArgumentsError(exec));

    WebCL* webCL = static_cast<WebCL*>(impl());
    const String name = exec->argument(0).toString(exec)->value(exec);
    if (exec->hadException())
        return jsUndefined();
    WebCLExtension* extension = webCL->getExtension(name);
    return toJS(exec, globalObject(), extension);
}

JSValue toJS(ExecState* exec, JSDOMGlobalObject* globalObject, WebCLExtension* extension)
{
    if (!extension)
        return jsNull();
    switch (extension->getName()) {
    case WebCLExtension::KhrGLSharingName:
#if ENABLE(WEBGL)
        return toJS(exec, globalObject, static_cast<WebCLGL*>(extension));
#else
        UNUSED_PARAM(exec);
        UNUSED_PARAM(globalObject);
        break;
#endif
    }
    ASSERT_NOT_REACHED();
    return jsNull();
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
        if (info.getWebCLContextProperties()->isGLCapable())
            return toJS(exec, globalObject, static_cast<WebCLGLContextProperties*>(info.getWebCLContextProperties().get()));
        return toJS(exec, globalObject, info.getWebCLContextProperties());
    default:
        return jsUndefined();
    }
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
