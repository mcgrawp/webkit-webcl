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

#ifndef JSWebCLKernelCustom_h
#define JSWebCLKernelCustom_h

#if ENABLE(WEBCL)

#include "JSWebCLMemoryObject.h"
#include "ScriptObject.h"
#include "ScriptState.h"
#include "WebCLKernelArgumentTypes.h"
#include <wtf/MathExtras.h>

using namespace JSC;
using namespace WebCore;

namespace WebCore {

inline void determineType(const ScriptValue& value, unsigned& argType)
{
    JSValue jsValue = value.jsValue();
    bool isArray = isJSArray(jsValue);
    // The type parameter is ignored if value is an instance of WebCLMemoryObject.
    if (jsValue.isObject() && !isArray && jsValue.inherits(&JSWebCLMemoryObject::s_info)) {
        argType = WebCLKernelArgumentTypes::ARG_MEMORY_OBJECT;
        return;
    }
    // if value is a number or array and argType is null, its type is taken to be FLOAT by default
    if (argType == WebCLKernelArgumentTypes::KERNEL_NULL_TYPE && (jsValue.isNumber() || isArray)) {
        argType = WebCLKernelArgumentTypes::FLOAT;
        return;
    }

    // Mask to remove the higher bytes reserved for Vectors notation.
    argType = argType & 0X00FF;
}

template <typename T>
inline int jsDecodeKernelArgValue(ScriptState* state, const JSValue& jsValue, Vector<T>& argValue)
{
    T jsArgValue;
    if (jsValue.isNumber())
        jsArgValue = static_cast<T>(jsValue.toNumber(state));
    else if (jsValue.isString())
        jsArgValue = static_cast<T>(jsValue.toWTFString(state)[0]);
    else
        return -1;
    if (state->hadException())
        return -1;
    argValue.append(jsArgValue);
    return 0;
}

// For WebCLMemoryObject
inline PassRefPtr<WebCLMemoryObject> jsDecodeKernelArgValue(ScriptState* state, const JSValue& jsValue)
{
    RefPtr<WebCLMemoryObject> argValue = toWebCLMemoryObject(jsValue.toObject(state));
    if (state->hadException())
        return 0;
    return argValue;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif
