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

#include "WebCL.h"
#include "WebCLException.h"

namespace WebCore {

WebCLKernel::~WebCLKernel()
{
    ASSERT(m_clKernel);
    CCerror computeContextErrorCode = m_context->computeContext()->releaseKernel(m_clKernel);
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
    m_clKernel = 0;
}

PassRefPtr<WebCLKernel> WebCLKernel::create(WebCLContext* context, cl_kernel kernel)
{
    return adoptRef(new WebCLKernel(context, kernel));
}

WebCLKernel::WebCLKernel(WebCLContext* context, cl_kernel kernel)
    : m_context(context)
    , m_clKernel(kernel)
{
}

WebCLGetInfo WebCLKernel::getInfo(int kernelInfo, ExceptionCode& ec)
{
    if (!m_clKernel) {
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    CCerror err = 0;
    switch (kernelInfo) {
    case ComputeContext::KERNEL_FUNCTION_NAME: {
        char functionName[WebCL::CHAR_BUFFER_SIZE] = {""};
        err = ComputeContext::getKernelInfo(m_clKernel, kernelInfo, sizeof(functionName), &functionName);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(String(functionName));
        break;
    }
    case ComputeContext::KERNEL_NUM_ARGS: {
        CCuint numberOfArgs = 0;
        err = ComputeContext::getKernelInfo(m_clKernel, kernelInfo, sizeof(CCuint), &numberOfArgs);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(numberOfArgs));
        break;
    }
    case ComputeContext::KERNEL_PROGRAM: {
        CCProgram ccProgramID = 0;
        err = ComputeContext::getKernelInfo(m_clKernel, kernelInfo, sizeof(CCProgram), &ccProgramID);
        if (err == CL_SUCCESS) {
            RefPtr<WebCLProgram> programObj = WebCLProgram::create(m_context, ccProgramID);
            return WebCLGetInfo(programObj.release());
        }
        break;
    }
    /* FIXME: we should not create a context here
    case ComputeContext::KERNEL_CONTEXT: {
        CCContext ccContextID = 0;
        err = ComputeContext::getKernelInfo(m_clKernel, kernelInfo, sizeof(CCContext), &ccContextID);
        if (err == CL_SUCCESS) {
            RefPtr<WebCLContext> contextObj = WebCLContext::create(m_context, ccContextID);
            return WebCLGetInfo(contextObj.release());
        }
        break;
    }
    */
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
    if (!m_clKernel) {
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    CCDeviceID ccDevice = 0;
    // FIXME: s/getCLDevice/getComputeContextDevice
    if (device)
        ccDevice = device->getCLDevice();
    if (!ccDevice) {
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }
    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::KERNEL_WORK_GROUP_SIZE:
    case ComputeContext::KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: {
        size_t kernelInfo = 0;
        err = ComputeContext::getWorkGroupInfo(m_clKernel, ccDevice, paramName, sizeof(size_t), &kernelInfo);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(kernelInfo));
        break;
    }
    case ComputeContext::KERNEL_COMPILE_WORK_GROUP_SIZE: {
        size_t workGroupSize[3] = {0, 0, 0};
        err = ComputeContext::getWorkGroupInfo(m_clKernel, ccDevice, paramName, sizeof(size_t) * 3, workGroupSize);
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
        err = ComputeContext::getWorkGroupInfo(m_clKernel, ccDevice, paramName, sizeof(CCulong), &localMemSize);
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

void WebCLKernel::setArg(unsigned argIndex, unsigned argSize, ExceptionCode& ec)
{
    if (!m_clKernel) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    CCerror err = clSetKernelArg(m_clKernel, argIndex, argSize, 0);
    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(unsigned argIndex, PassRefPtr<WebCLKernelTypeValue> kernelObject, unsigned argType, ExceptionCode& ec)
{
    if (!m_clKernel) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    if (!kernelObject) {
        ec = WebCLException::INVALID_ARG_VALUE;
        return;
    }

    CCerror err = 0;
    RefPtr<WebCLKernelTypeVector> array = 0;
    char nodeIdCh = 0;
    unsigned char nodeIduCh = 0;
    short nodeIdSh = 0;
    unsigned short nodeIduSh = 0;
    int nodeIdInt = 0;
    unsigned nodeIduInt = 0;
    long nodeIdLong = 0;
    long nodeIduLong = 0;
    float nodeIdFloat = 0;

    switch (argType) {
    case ComputeContext::CHAR:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdCh, argIndex, sizeof(cl_char));
        break;
    case ComputeContext::UCHAR:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIduCh, argIndex, sizeof(cl_uchar));
        break;
    case ComputeContext::SHORT:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdSh, argIndex, sizeof(cl_short));
        break;
    case ComputeContext::USHORT:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIduSh, argIndex, sizeof(cl_ushort));
        break;
    case ComputeContext::INT:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdInt, argIndex, sizeof(cl_int));
        break;
    case ComputeContext::UINT:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIduInt, argIndex, sizeof(cl_uint));
        break;
    case ComputeContext::LONG:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdLong, argIndex, sizeof(cl_long));
        break;
    case ComputeContext::ULONG:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIduLong, argIndex, sizeof(cl_ulong));
        break;
    case ComputeContext::FLOAT_KERNEL_ARG:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdFloat, argIndex, sizeof(cl_float));
        break;
    case ComputeContext::HALF:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdFloat, argIndex, sizeof(cl_half));
        break;
    case ComputeContext::DOUBLE:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdFloat, argIndex, sizeof(cl_double));
        break;
    case ComputeContext::CHAR | ComputeContext::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_char2), 2);
        break;
    case ComputeContext::UCHAR | ComputeContext::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uchar2), 2);
        break;
    case ComputeContext::SHORT | ComputeContext::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_short2), 2);
        break;
    case ComputeContext::USHORT | ComputeContext::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ushort2), 2);
        break;
    case ComputeContext::INT | ComputeContext::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_int2), 2);
        break;
    case ComputeContext::UINT | ComputeContext::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uint2), 2);
        break;
    case ComputeContext::LONG | ComputeContext::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_long2), 2);
        break;
    case ComputeContext::ULONG | ComputeContext::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ulong2), 2);
        break;
    case ComputeContext::FLOAT_KERNEL_ARG | ComputeContext::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_float2), 2);
        break;
    case ComputeContext::CHAR | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_char4), 4);
        break;
    case ComputeContext::UCHAR | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uchar4), 4);
        break;
    case ComputeContext::SHORT | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_short4), 4);
        break;
    case ComputeContext::USHORT | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ushort4), 4);
        break;
    case ComputeContext::INT | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_int4), 4);
        break;
    case ComputeContext::UINT | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uint4), 4);
        break;
    case ComputeContext::LONG | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_long4), 4);
        break;
    case ComputeContext::ULONG | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ulong4), 4);
        break;
    case ComputeContext::FLOAT_KERNEL_ARG | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_float4), 4);
        break;
    case ComputeContext::DOUBLE | ComputeContext::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_double4), 4);
        break;
    case ComputeContext::CHAR | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_char8), 8);
        break;
    case ComputeContext::UCHAR | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uchar4), 4);
        break;
    case ComputeContext::SHORT | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_short8), 8);
        break;
    case ComputeContext::USHORT | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ushort8), 8);
        break;
    case ComputeContext::INT | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_int8), 8);
        break;
    case ComputeContext::UINT | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uint8), 8);
        break;
    case ComputeContext::LONG | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_long8), 8);
        break;
    case ComputeContext::ULONG | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ulong8), 8);
        break;
    case ComputeContext::FLOAT_KERNEL_ARG | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_float8), 8);
        break;
    case ComputeContext::DOUBLE | ComputeContext::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_double8), 8);
        break;
    case ComputeContext::CHAR | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_char8), 8);
        break;
    case ComputeContext::UCHAR | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uchar16), 16);
        break;
    case ComputeContext::SHORT | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_short16), 16);
        break;
    case ComputeContext::USHORT | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ushort16), 16);
        break;
    case ComputeContext::INT | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_int16), 16);
        break;
    case ComputeContext::UINT | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uint16), 16);
        break;
    case ComputeContext::LONG | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_long16), 16);
        break;
    case ComputeContext::ULONG | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ulong16), 16);
        break;
    case ComputeContext::FLOAT_KERNEL_ARG | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_float16), 16);
        break;
    case ComputeContext::DOUBLE | ComputeContext::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_double16), 16);
        break;
    default:
        ec = WebCLException::INVALID_ARG_VALUE;
        printf("Error: Invaild Kernel Argument Type\n");
        return;
    }
    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(unsigned argIndex, WebCLMemoryObject* argValue, ExceptionCode& ec)
{
    CCerror err = 0;
    cl_mem memID = 0;
    cl_device_id clDevice = 0;

    if (!m_clKernel) {
        ec = WebCLException::INVALID_KERNEL;
        printf("Error: Invalid kernel\n");
        return;
    }
    if (argValue)
        memID = argValue->getCLMemoryObject();
    if (!memID) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        printf("Error: cl_mem_id null\n");
        return;
    }
    if (m_deviceID) {
        clDevice = m_deviceID->getCLDevice();
        cl_ulong maxBufferSize = 0;
        clGetDeviceInfo(clDevice, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &maxBufferSize, 0);
        cl_uint maxArgs = 0;
        clGetDeviceInfo(clDevice, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &maxArgs, 0);
        // Check for __constant qualifier restrictions. Memory object size must be less than CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
        // and index must be lesser than CL_DEVICE_MAX_CONSTANT_ARGS supported by device.
        if (sizeof(cl_mem) > maxBufferSize && argIndex > maxArgs) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error::setArg::WebCLMemoryObject is not valid to call with __constant restriction\n");
            return;
        }
    }
    // FIXME :: Need to check if we must do above check with default device.
    // Memory Object sent is valid.
    err = clSetKernelArg(m_clKernel, argIndex, sizeof(cl_mem), &memID);

    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(unsigned argIndex, unsigned argValue, unsigned argSize, ExceptionCode& ec)
{
    CCerror err = 0;
    if (!m_clKernel) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    err = clSetKernelArg(m_clKernel, argIndex, argSize, &argValue);
    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

template<class T> inline unsigned WebCLKernel::clSetKernelArgPrimitiveType(cl_kernel kernelID,
    RefPtr<WebCLKernelTypeValue> kernelObject, T nodeId, unsigned argIndex, int size)
{
    if (!kernelObject->asNumber(&nodeId)) {
        printf("Error: WebCL Kernel Type Value Not Proper\n");
        return -1;
    }
    CCerror err =  clSetKernelArg(kernelID, argIndex, size, &nodeId);
    return err;
}

inline unsigned WebCLKernel::clSetKernelArgVectorType(cl_kernel kernelID, RefPtr<WebCLKernelTypeValue>
    kernelObject, RefPtr<WebCLKernelTypeVector> array, unsigned argIndex, int size, unsigned length)
{
    if ((!kernelObject->asVector(&array)) || (length != array->length())) {
        printf("Error: Invalid WebCL Kernel Type %d\n", array->length());
        return -1;
    }
    CCerror err = clSetKernelArg(kernelID, argIndex, size, &array);
    return err;
}

void WebCLKernel::setDevice(PassRefPtr<WebCLDevice> deviceID)
{
    m_deviceID = deviceID;
}

cl_kernel WebCLKernel::getCLKernel()
{
    return m_clKernel;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
