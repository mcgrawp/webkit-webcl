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

#include "WebCLKernel.h"

#include "WebCLContext.h"
#include "WebCLException.h"
#include "WebCLInputChecker.h"

namespace WebCore {

WebCLKernel::~WebCLKernel()
{
    releasePlatformObject();
}

PassRefPtr<WebCLKernel> WebCLKernel::create(WebCLContext* context, WebCLProgram* program, const String& kernelName, ExceptionCode& ec)
{
    CCerror error = ComputeContext::SUCCESS;
    CCKernel computeContextKernel = context->computeContext()->createKernel(program->platformObject(), kernelName, error);
    if (!computeContextKernel) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    return adoptRef(new WebCLKernel(context, program, computeContextKernel, kernelName));
}

Vector<RefPtr<WebCLKernel> > WebCLKernel::createKernelsInProgram(WebCLContext* context, WebCLProgram* program, ExceptionCode& ec)
{
    CCerror error = ComputeContext::SUCCESS;
    Vector<CCKernel> computeContextKernels = context->computeContext()->createKernelsInProgram(program->platformObject(), error);
    Vector<RefPtr<WebCLKernel> > kernels;
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return kernels;
    }

    Vector<char> functionName;
    for (size_t i = 0 ; i < computeContextKernels.size(); i++) {
        error = ComputeContext::getKernelInfo(computeContextKernels[i], ComputeContext::KERNEL_FUNCTION_NAME, functionName.data());
        if (error != ComputeContext::SUCCESS) {
            ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
            kernels.clear();
            return kernels;
        }

        kernels.append(adoptRef(new WebCLKernel(context, program, computeContextKernels[i], String(functionName.data()))));
    }

    return kernels;
}

WebCLKernel::WebCLKernel(WebCLContext* context, WebCLProgram* program, CCKernel kernel, const String& kernelName)
    : WebCLObject(kernel)
    , m_context(context)
    , m_program(program)
    , m_kernelName(kernelName)
{
}

WebCLGetInfo WebCLKernel::getInfo(int kernelInfo, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    CCerror err = ComputeContext::SUCCESS;
    switch (kernelInfo) {
    case ComputeContext::KERNEL_FUNCTION_NAME:
        return WebCLGetInfo(m_kernelName);
    case ComputeContext::KERNEL_NUM_ARGS: {
        CCuint numberOfArgs = 0;
        err = ComputeContext::getKernelInfo(platformObject(), kernelInfo, &numberOfArgs);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(numberOfArgs));
        break;
    }
    case ComputeContext::KERNEL_PROGRAM:
        return WebCLGetInfo(m_program);
    case ComputeContext::KERNEL_CONTEXT:
        return WebCLGetInfo(m_context);
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

WebCLGetInfo WebCLKernel::getWorkGroupInfo(WebCLDevice* device, int paramName, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    if (!WebCLInputChecker::validateWebCLObject(device)) {
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }

    CCDeviceID ccDevice = device->platformObject();

    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::KERNEL_WORK_GROUP_SIZE:
    case ComputeContext::KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: {
        size_t kernelInfo = 0;
        err = ComputeContext::getWorkGroupInfo(platformObject(), ccDevice, paramName, sizeof(size_t), &kernelInfo);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(kernelInfo));
        break;
    }
    case ComputeContext::KERNEL_COMPILE_WORK_GROUP_SIZE: {
        size_t workGroupSize[3] = {0, 0, 0};
        err = ComputeContext::getWorkGroupInfo(platformObject(), ccDevice, paramName, sizeof(size_t) * 3, workGroupSize);
        if (err == CL_SUCCESS) {
            RefPtr<Int32Array> values = Int32Array::create(3);
            for (int i = 0; i < 3; i++)
                values->set(i, workGroupSize[i]);
            return WebCLGetInfo(values.release());
        }
        break;
    }
    case ComputeContext::KERNEL_PRIVATE_MEM_SIZE:
    case ComputeContext::KERNEL_LOCAL_MEM_SIZE: {
        CCulong localMemSize = 0;
        err = ComputeContext::getWorkGroupInfo(platformObject(), ccDevice, paramName, sizeof(CCulong), &localMemSize);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned long>(localMemSize));
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

void WebCLKernel::setDevice(PassRefPtr<WebCLDevice> deviceID)
{
    m_deviceID = deviceID;
}

void WebCLKernel::releasePlatformObjectImpl()
{
    CCerror computeContextErrorCode = m_context->computeContext()->releaseKernel(platformObject());
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
}

template<typename T>
static void setArgHelper(WebCLKernel* kernel, unsigned argIndex, ScriptState* state, const ScriptValue& jsArgValue, ExceptionCode& ec)
{
    Vector<T> argValue;
    size_t argSize = 0;
    if (jsDecodeKernelArgValue(state, jsArgValue.jsValue(), argSize, argValue)) {
        ec = WebCLException::INVALID_KERNEL_ARGS;
        return;
    }
    CCerror err = ComputeContext::setKernelArg(kernel->platformObject(), argIndex, argSize, argValue.data());
    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(ScriptState* state, unsigned argIndex, const ScriptValue& value, unsigned argType, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    if (value.jsValue().isObject() && !argType)
        argType = ComputeContext::ARG_MEMORY_OBJECT;
    switch (argType) {
    case ComputeContext::INT:
        setArgHelper<int>(this, argIndex, state, value, ec);
        break;
    case ComputeContext::UINT:
        setArgHelper<unsigned>(this, argIndex, state, value, ec);
        break;
    case ComputeContext::CHAR:
        setArgHelper<char>(this, argIndex, state, value, ec);
        break;
    case ComputeContext::UCHAR:
        setArgHelper<unsigned char>(this, argIndex, state, value, ec);
        break;
    case ComputeContext::SHORT:
        setArgHelper<short>(this, argIndex, state, value, ec);
        break;
    case ComputeContext::USHORT:
        setArgHelper<unsigned short>(this, argIndex, state, value, ec);
        break;
    case ComputeContext::LONG:
        setArgHelper<long>(this, argIndex, state, value, ec);
        break;
    case ComputeContext::ULONG:
        setArgHelper<unsigned long>(this, argIndex, state, value, ec);
        break;
    case ComputeContext::FLOAT_KERNEL_ARG:
        setArgHelper<float>(this, argIndex, state, value, ec);
        break;
    case ComputeContext::ARG_MEMORY_OBJECT: {
        size_t argSize = 0;
        RefPtr<WebCLMemoryObject> argValue = jsDecodeKernelArgValue(state, value.jsValue(), argSize);
        if (!argValue) {
            ec = WebCLException::INVALID_KERNEL_ARGS;
            return;
        }
        if (!WebCLInputChecker::validateWebCLObject(argValue.get())) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
        PlatformComputeObject ccMemoryObject = argValue->platformObject();
        if (!ccMemoryObject) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
        CCerror err = ComputeContext::setKernelArg(platformObject(), argIndex, sizeof(PlatformComputeObject), &ccMemoryObject);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
        return;
    }
    default:
        ec = WebCLException::INVALID_KERNEL_ARGS;
        return;
    }
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
