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
    CCerror err = 0;

    ASSERT(m_clKernel);

    err = clReleaseKernel(m_clKernel);
    if (err != CL_SUCCESS) {
        switch (err) {
        case CL_INVALID_KERNEL:
            printf("Error: CL_INVALID_KERNEL \n");
            break;
        case CL_OUT_OF_RESOURCES:
            printf("Error: CL_OUT_OF_RESOURCES  \n");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
            break;
        default:
            printf("Error: Invaild Error Type\n");
            break;
        }
    } else {
        for (int i = 0; i < m_numKernels; i++) {
            if ((m_kernelList[i].get())->getCLKernel() == m_clKernel) {
                m_kernelList.remove(i);
                m_numKernels = m_kernelList.size();
                break;
            }
        }
    }
}

PassRefPtr<WebCLKernel> WebCLKernel::create(WebCLContext* context, cl_kernel kernel)
{
    return adoptRef(new WebCLKernel(context, kernel));
}

WebCLKernel::WebCLKernel(WebCLContext* context, cl_kernel kernel)
    : m_context(context)
    , m_clKernel(kernel)
    , m_numKernels(0)
{
}

WebCLGetInfo WebCLKernel::getInfo(int kernelInfo, ExceptionCode& ec)
{
    CCerror err = 0;
    char functionName[WebCL::CHAR_BUFFER_SIZE] = {""};
    cl_uint uintUnits = 0;
    cl_program clProgramID = 0;
    // cl_context clContextID = 0;
    RefPtr<WebCLProgram> programObj = 0;
    RefPtr<WebCLContext> contextObj = 0;

    if (!m_clKernel) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    switch (kernelInfo) {
    case WebCL::KERNEL_FUNCTION_NAME:
        err = clGetKernelInfo(m_clKernel, CL_KERNEL_FUNCTION_NAME, sizeof(functionName), &functionName, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(String(functionName));
        break;
    case WebCL::KERNEL_NUM_ARGS:
        err = clGetKernelInfo(m_clKernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &uintUnits, 0);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned>(uintUnits));
        break;
    case WebCL::KERNEL_REFERENCE_COUNT:
        err = clGetKernelInfo(m_clKernel, CL_KERNEL_REFERENCE_COUNT, sizeof(cl_uint), &uintUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uintUnits));
        break;
    case WebCL::KERNEL_PROGRAM:
        {
            err = clGetKernelInfo(m_clKernel, CL_KERNEL_PROGRAM, sizeof(clProgramID), &clProgramID, 0);
            if (err == CL_SUCCESS) {
                programObj = WebCLProgram::create(m_context, clProgramID);
                return WebCLGetInfo(PassRefPtr<WebCLProgram>(programObj));
            }
        }
        break;
    /* FIXME: we should not create a context here
    case WebCL::KERNEL_CONTEXT:
        {
            err = clGetKernelInfo(m_clKernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &clContextID, 0);
            if (err == CL_SUCCESS) {
                contextObj = WebCLContext::create(m_context, clContextID);
                return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
            }
        }
        break;
    */
    default:
        ec = WebCLException::FAILURE;
        printf("Error: Invaild Kernel_info \n");
        return WebCLGetInfo();
    }

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

WebCLGetInfo WebCLKernel::getWorkGroupInfo(WebCLDevice* device, int paramName, ExceptionCode& ec)
{
    CCerror err = 0;
    cl_device_id clDevice = 0;
    size_t sizetUnits = 0;
    cl_ulong ulongUnits = 0;

    if (!m_clKernel) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    if (device) {
        // FIXME: s/getCLDevice/getComputeContextDevice
        clDevice = device->getCLDevice();
    }
    if (!clDevice) {
        printf("Error: cl_device null\n");
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }
    switch (paramName) {
    case WebCL::KERNEL_WORK_GROUP_SIZE:
        err = clGetKernelWorkGroupInfo(m_clKernel, clDevice, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &sizetUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(sizetUnits));
        break;
    case WebCL::KERNEL_COMPILE_WORK_GROUP_SIZE:
        err = clGetKernelWorkGroupInfo(m_clKernel, clDevice, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t), &sizetUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(sizetUnits));
        break;
    case WebCL::KERNEL_LOCAL_MEM_SIZE:
        err = clGetKernelWorkGroupInfo(m_clKernel, clDevice, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &ulongUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned long>(ulongUnits));
        break;
    default:
        printf("Error: Unsupported Kernrl Info type\n");
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
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    CCerror err = clSetKernelArg(m_clKernel, argIndex, argSize, 0);
    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(unsigned argIndex, PassRefPtr<WebCLKernelTypeValue> kernelObject, unsigned argType, ExceptionCode& ec)
{
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

    if (!m_clKernel) {
        ec = WebCLException::INVALID_KERNEL;
        printf("Error: Invalid kernel\n");
        return;
    }
    if (!kernelObject) {
        ec = WebCLException::INVALID_ARG_VALUE;
        printf("Error: WebCLKernelTypeValue is null\n");
        return;
    }
    switch (argType) {
    case WebCL::CHAR:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdCh, argIndex, sizeof(cl_char));
        break;
    case WebCL::UCHAR:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIduCh, argIndex, sizeof(cl_uchar));
        break;
    case WebCL::SHORT:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdSh, argIndex, sizeof(cl_short));
        break;
    case WebCL::USHORT:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIduSh, argIndex, sizeof(cl_ushort));
        break;
    case WebCL::INT:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdInt, argIndex, sizeof(CCerror));
        break;
    case WebCL::UINT:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIduInt, argIndex, sizeof(cl_uint));
        break;
    case WebCL::LONG:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdLong, argIndex, sizeof(cl_long));
        break;
    case WebCL::ULONG:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIduLong, argIndex, sizeof(cl_ulong));
        break;
    case WebCL::FLOAT_KERNEL_ARG:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdFloat, argIndex, sizeof(cl_float));
        break;
    case WebCL::HALF:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdFloat, argIndex, sizeof(cl_half));
        break;
    case WebCL::DOUBLE:
        err = clSetKernelArgPrimitiveType(m_clKernel, kernelObject, nodeIdFloat, argIndex, sizeof(cl_double));
        break;
    case WebCL::CHAR | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_char2), 2);
        break;
    case WebCL::UCHAR | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uchar2), 2);
        break;
    case WebCL::SHORT | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_short2), 2);
        break;
    case WebCL::USHORT | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ushort2), 2);
        break;
    case WebCL::INT | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_int2), 2);
        break;
    case WebCL::UINT | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uint2), 2);
        break;
    case WebCL::LONG | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_long2), 2);
        break;
    case WebCL::ULONG | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ulong2), 2);
        break;
    case WebCL::FLOAT_KERNEL_ARG | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_float2), 2);
        break;
    case WebCL::CHAR | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_char4), 4);
        break;
    case WebCL::UCHAR | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uchar4), 4);
        break;
    case WebCL::SHORT | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_short4), 4);
        break;
    case WebCL::USHORT | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ushort4), 4);
        break;
    case WebCL::INT | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_int4), 4);
        break;
    case WebCL::UINT | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uint4), 4);
        break;
    case WebCL::LONG | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_long4), 4);
        break;
    case WebCL::ULONG | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ulong4), 4);
        break;
    case WebCL::FLOAT_KERNEL_ARG | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_float4), 4);
        break;
    case WebCL::DOUBLE | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_double4), 4);
        break;
    case WebCL::CHAR | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_char8), 8);
        break;
    case WebCL::UCHAR | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uchar4), 4);
        break;
    case WebCL::SHORT | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_short8), 8);
        break;
    case WebCL::USHORT | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ushort8), 8);
        break;
    case WebCL::INT | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_int8), 8);
        break;
    case WebCL::UINT | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uint8), 8);
        break;
    case WebCL::LONG | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_long8), 8);
        break;
    case WebCL::ULONG | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ulong8), 8);
        break;
    case WebCL::FLOAT_KERNEL_ARG | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_float8), 8);
        break;
    case WebCL::DOUBLE | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_double8), 8);
        break;
    case WebCL::CHAR | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_char8), 8);
        break;
    case WebCL::UCHAR | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uchar16), 16);
        break;
    case WebCL::SHORT | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_short16), 16);
        break;
    case WebCL::USHORT | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ushort16), 16);
        break;
    case WebCL::INT | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_int16), 16);
        break;
    case WebCL::UINT | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_uint16), 16);
        break;
    case WebCL::LONG | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_long16), 16);
        break;
    case WebCL::ULONG | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_ulong16), 16);
        break;
    case WebCL::FLOAT_KERNEL_ARG | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_float16), 16);
        break;
    case WebCL::DOUBLE | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_clKernel, kernelObject, array, argIndex, sizeof(cl_double16), 16);
        break;
    case WebCL::KERNEL_ARG_SAMPLER:
        printf("CL_KERNEL_ARG_SAMPLER - Not Handled\n");
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
        printf("Error: Invalid kernel\n");
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
