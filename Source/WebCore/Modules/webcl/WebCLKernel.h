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

#ifndef WebCLKernel_h
#define WebCLKernel_h

#if ENABLE(WEBCL)

#include "JSWebCLKernelCustom.h"
#include "WebCLInputChecker.h"
#include "WebCLMemoryObject.h"
#include "WebCLObject.h"

namespace WTF {
class ArrayBufferView;
}

namespace WebCore {

class WebCLGetInfo;
class WebCLContext;
class WebCLDevice;
class WebCLProgram;
class WebCLMemoryObject;
class WebCLSampler;

class WebCLKernel : public WebCLObject<CCKernel> {
public:
    virtual ~WebCLKernel();
    static PassRefPtr<WebCLKernel> create(WebCLContext*, WebCLProgram*, const String&, ExceptionCode&);
    static Vector<RefPtr<WebCLKernel> > createKernelsInProgram(WebCLContext*, WebCLProgram*, ExceptionCode&);
    WebCLGetInfo getInfo(int, ExceptionCode&);
    WebCLGetInfo getWorkGroupInfo(WebCLDevice*, int, ExceptionCode&);

    void setArg(unsigned index, WebCLMemoryObject*, ExceptionCode&);
    void setArg(unsigned index, WebCLSampler*, ExceptionCode&);
    void setArg(unsigned index, ArrayBufferView*, ExceptionCode&);

    void setArg(ScriptState*, unsigned index, const ScriptValue&, unsigned type, ExceptionCode&);
    // FIXME:: Issue with default value. When user doesnt send argType using [Default=undefined] in IDL sends 0 as argType.
    // Need to set default value as KERNEL_NULL_TYPE, since 0 also represents WebCLKernelArgumentTypes.CHAR as per specification.
    void setArg(ScriptState* state, unsigned index, const ScriptValue& value, ExceptionCode& ec)
    {
        setArg(state, index, value, WebCLKernelArgumentTypes::KERNEL_NULL_TYPE, ec);
    }

    void setDevice(PassRefPtr<WebCLDevice>);
private:
    WebCLKernel(WebCLContext*, WebCLProgram*, CCKernel, const String&);

    template <class T>
    void setArgHelper(unsigned argIndex, ScriptState*, const ScriptValue& jsArgValue, unsigned vectorSize, ExceptionCode&);
    void setArgHelper(unsigned argIndex, ScriptState*, const ScriptValue& jsArgValue, ExceptionCode&);
    void setMemoryArgHelper(unsigned argIndex, ScriptState*, const ScriptValue& jsArgValue, ExceptionCode&);

    void releasePlatformObjectImpl();

    WebCLContext* m_context;
    WebCLProgram* m_program;
    String m_kernelName;
    RefPtr<WebCLDevice> m_deviceID;
};

template<typename T>
inline void WebCLKernel::setArgHelper(unsigned argIndex, ScriptState* state, const ScriptValue& jsArgValue, unsigned vectorSize, ExceptionCode& ec)
{
    Vector<T> argValue;
    if (jsDecodeKernelArgValue(state, jsArgValue.jsValue(), vectorSize, argValue)) {
        ec = WebCLException::INVALID_KERNEL_ARGS;
        return;
    }
    size_t argSize = sizeof(T) * (vectorSize ? vectorSize : 1);
    CCerror err = ComputeContext::setKernelArg(platformObject(), argIndex, argSize, argValue.data());
    ASSERT(err != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

// Helper for __local kernel variables.
inline void WebCLKernel::setArgHelper(unsigned argIndex, ScriptState* state, const ScriptValue& jsArgValue, ExceptionCode& ec)
{
    Vector<size_t> argValue;
    unsigned vectorSize = 0;
    if (jsDecodeKernelArgValue(state, jsArgValue.jsValue(), vectorSize, argValue)) {
        ec = WebCLException::INVALID_KERNEL_ARGS;
        return;
    }
    if (argValue.size() < 1) {
        ec = WebCLException::INVALID_ARG_VALUE;
        return;
    }

    CCerror err = ComputeContext::setKernelArg(platformObject(), argIndex, argValue[0], 0 /* __local kernel variables must not be initialised. */);
    ASSERT(err != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

inline void WebCLKernel::setMemoryArgHelper(unsigned argIndex, ScriptState* state, const ScriptValue& jsArgValue, ExceptionCode& ec)
{
    RefPtr<WebCLMemoryObject> argValue = jsDecodeKernelArgValue(state, jsArgValue.jsValue());
    if (!argValue) {
        ec = WebCLException::INVALID_KERNEL_ARGS;
        return;
    }
    if (!WebCLInputChecker::validateWebCLObject(argValue.get())) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }
    PlatformComputeObject ccMemoryObject = argValue->platformObject();
    CCerror err = ComputeContext::setKernelArg(platformObject(), argIndex, sizeof(PlatformComputeObject), &ccMemoryObject);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

} // namespace WebCore

#endif
#endif // WebCLKernel_h
