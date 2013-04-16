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
        if (err == ComputeContext::SUCCESS)
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
        err = ComputeContext::getWorkGroupInfo(platformObject(), ccDevice, paramName, &kernelInfo);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(kernelInfo));
        break;
    }
    case ComputeContext::KERNEL_COMPILE_WORK_GROUP_SIZE: {
        Vector<size_t> workGroupSize;
        err = ComputeContext::getWorkGroupInfo(platformObject(), ccDevice, paramName, &workGroupSize);
        if (err == ComputeContext::SUCCESS) {
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
        err = ComputeContext::getWorkGroupInfo(platformObject(), ccDevice, paramName, &localMemSize);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned long>(localMemSize));
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

void WebCLKernel::setArg(unsigned argIndex, unsigned argSize, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    CCerror err = ComputeContext::setKernelArg(platformObject(), argIndex, argSize, 0);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(unsigned argIndex, PassRefPtr<WebCLKernelTypeValue> kernelObject, unsigned argType, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    if (!kernelObject) {
        ec = WebCLException::INVALID_ARG_VALUE;
        return;
    }

    CCerror err = 0;
    RefPtr<WebCLKernelTypeVector> vectorValueArgument = 0;
    float FloatValueArgument = 0;
    switch (argType) {
    case ComputeContext::CHAR: {
        char charValueArgument = 0;
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, charValueArgument, argIndex, sizeof(CCchar));
        break;
    }
    case ComputeContext::UCHAR: {
        unsigned char ucharValueArgument = 0;
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, ucharValueArgument, argIndex, sizeof(CCuchar));
        break;
    }
    case ComputeContext::SHORT: {
        short shortValueArgument = 0;
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, shortValueArgument, argIndex, sizeof(CCshort));
        break;
    }
    case ComputeContext::USHORT: {
        unsigned short ushortValueArgument = 0;
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, ushortValueArgument, argIndex, sizeof(CCushort));
        break;
    }
    case ComputeContext::INT: {
        int intValueArgument = 0;
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, intValueArgument, argIndex, sizeof(CCint));
        break;
    }
    case ComputeContext::UINT: {
        unsigned uintValueArgument = 0;
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, uintValueArgument, argIndex, sizeof(CCuint));
        break;
    }
    case ComputeContext::LONG: {
        long longValueArgument = 0;
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, longValueArgument, argIndex, sizeof(CClong));
        break;
    }
    case ComputeContext::ULONG: {
        long ulongValueArgument = 0;
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, ulongValueArgument, argIndex, sizeof(CCulong));
        break;
    }
    case ComputeContext::FLOAT_KERNEL_ARG:
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, FloatValueArgument, argIndex, sizeof(CCfloat));
        break;
    case ComputeContext::HALF:
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, FloatValueArgument, argIndex, sizeof(CChalf));
        break;
    case ComputeContext::DOUBLE:
        err = setKernelArgPrimitiveType(platformObject(), kernelObject, FloatValueArgument, argIndex, sizeof(CCdouble));
        break;
    case ComputeContext::LOCAL_MEMORY_SIZE: {
        float localArgSize = 0;
        kernelObject->asNumber(&localArgSize);
        err = ComputeContext::setKernelArg(platformObject(), argIndex, localArgSize, 0);
        break;
    }
    case ComputeContext::CHAR | ComputeContext::VEC2:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCchar2), 2);
        break;
    case ComputeContext::UCHAR | ComputeContext::VEC2:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCuchar2), 2);
        break;
    case ComputeContext::SHORT | ComputeContext::VEC2:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCshort2), 2);
        break;
    case ComputeContext::USHORT | ComputeContext::VEC2:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCushort2), 2);
        break;
    case ComputeContext::INT | ComputeContext::VEC2:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCint2), 2);
        break;
    case ComputeContext::UINT | ComputeContext::VEC2:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCuint2), 2);
        break;
    case ComputeContext::LONG | ComputeContext::VEC2:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CClong2), 2);
        break;
    case ComputeContext::ULONG | ComputeContext::VEC2:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCulong2), 2);
        break;
    case ComputeContext::FLOAT_KERNEL_ARG | ComputeContext::VEC2:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCfloat2), 2);
        break;
    case ComputeContext::CHAR | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCchar4), 4);
        break;
    case ComputeContext::UCHAR | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCuchar4), 4);
        break;
    case ComputeContext::SHORT | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCshort4), 4);
        break;
    case ComputeContext::USHORT | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCushort4), 4);
        break;
    case ComputeContext::INT | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCint4), 4);
        break;
    case ComputeContext::UINT | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCuint4), 4);
        break;
    case ComputeContext::LONG | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CClong4), 4);
        break;
    case ComputeContext::ULONG | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCulong4), 4);
        break;
    case ComputeContext::FLOAT_KERNEL_ARG | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCfloat4), 4);
        break;
    case ComputeContext::DOUBLE | ComputeContext::VEC4:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCdouble4), 4);
        break;
    case ComputeContext::CHAR | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCchar8), 8);
        break;
    case ComputeContext::UCHAR | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCuchar4), 4);
        break;
    case ComputeContext::SHORT | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCshort8), 8);
        break;
    case ComputeContext::USHORT | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCushort8), 8);
        break;
    case ComputeContext::INT | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCint8), 8);
        break;
    case ComputeContext::UINT | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCuint8), 8);
        break;
    case ComputeContext::LONG | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CClong8), 8);
        break;
    case ComputeContext::ULONG | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCulong8), 8);
        break;
    case ComputeContext::FLOAT_KERNEL_ARG | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCfloat8), 8);
        break;
    case ComputeContext::DOUBLE | ComputeContext::VEC8:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCdouble8), 8);
        break;
    case ComputeContext::CHAR | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCchar8), 8);
        break;
    case ComputeContext::UCHAR | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCuchar16), 16);
        break;
    case ComputeContext::SHORT | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCshort16), 16);
        break;
    case ComputeContext::USHORT | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCushort16), 16);
        break;
    case ComputeContext::INT | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCint16), 16);
        break;
    case ComputeContext::UINT | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCuint16), 16);
        break;
    case ComputeContext::LONG | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CClong16), 16);
        break;
    case ComputeContext::ULONG | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCulong16), 16);
        break;
    case ComputeContext::FLOAT_KERNEL_ARG | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCfloat16), 16);
        break;
    case ComputeContext::DOUBLE | ComputeContext::VEC16:
        err = setKernelArgVectorType(platformObject(), kernelObject, vectorValueArgument, argIndex, sizeof(CCdouble16), 16);
        break;
    default:
        ec = WebCLException::INVALID_ARG_VALUE;
        return;
    }
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(unsigned argIndex, WebCLMemoryObject* argValue, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(argValue)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    PlatformComputeObject ccMemoryObject = argValue->platformObject();

    CCerror err = 0;
    CCDeviceID ccDevice = 0;
    if (m_deviceID) {
        ccDevice = m_deviceID->platformObject();
        CCulong maxBufferSize = 0;
        err = ComputeContext::getDeviceInfo(ccDevice, ComputeContext::DEVICE_MAX_CONSTANT_BUFFER_SIZE, &maxBufferSize);
        CCuint maxArgs = 0;
        err = ComputeContext::getDeviceInfo(ccDevice, ComputeContext::DEVICE_MAX_CONSTANT_ARGS, &maxArgs);
        // Check for __constant qualifier restrictions. Memory object size must be less than CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
        // and index must be lesser than CL_DEVICE_MAX_CONSTANT_ARGS supported by device.
        if (sizeof(PlatformComputeObject) > maxBufferSize && argIndex > maxArgs) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
    }
    // FIXME :: Need to check if we must do above check with default device.
    err = ComputeContext::setKernelArg(platformObject(), argIndex, sizeof(PlatformComputeObject), &ccMemoryObject);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(unsigned argIndex, unsigned argValue, unsigned argSize, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    CCerror err = ComputeContext::setKernelArg(platformObject(), argIndex, argSize, &argValue);
    if (err != ComputeContext::SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

template<class T>
inline CCerror WebCLKernel::setKernelArgPrimitiveType(CCKernel ccKernel,
    PassRefPtr<WebCLKernelTypeValue> kernelObject, T nodeId, unsigned argIndex, int size)
{
    if (!kernelObject->asNumber(&nodeId))
        return ComputeContext::INVALID_ARG_VALUE;
    CCerror err =  ComputeContext::setKernelArg(ccKernel, argIndex, size, &nodeId);
    return err;
}

inline CCerror WebCLKernel::setKernelArgVectorType(CCKernel ccKernel, PassRefPtr<WebCLKernelTypeValue>
    kernelObject, PassRefPtr<WebCLKernelTypeVector> array, unsigned argIndex, int size, unsigned length)
{
    if ((!kernelObject->asVector(array)) || (length != array->length()))
        return ComputeContext::INVALID_ARG_VALUE;
    CCerror err = ComputeContext::setKernelArg(ccKernel, argIndex, size, &array);
    return err;
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

} // namespace WebCore

#endif // ENABLE(WEBCL)
