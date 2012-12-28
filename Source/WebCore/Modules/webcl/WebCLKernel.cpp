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
    ASSERT(m_cl_kernel);

    CCerror err = clReleaseKernel(m_cl_kernel);
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
        for (int i = 0; i < m_num_kernels; i++) {
            if ((m_kernel_list[i].get())->getCLKernel() == m_cl_kernel) {
                m_kernel_list.remove(i);
                m_num_kernels = m_kernel_list.size();
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
    , m_cl_kernel(kernel)
{
    m_num_kernels = 0;
}

WebCLGetInfo WebCLKernel::getInfo(int kernel_info, ExceptionCode& ec)
{
    CCerror err = 0;
    char function_name[1024];
    cl_uint uint_units = 0;
    cl_program cl_program_id = 0;
    RefPtr<WebCLProgram> programObj = 0;

    if (!m_cl_kernel) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    switch (kernel_info) {
    case WebCL::KERNEL_FUNCTION_NAME:
        err = clGetKernelInfo(m_cl_kernel, CL_KERNEL_FUNCTION_NAME, sizeof(function_name), &function_name, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(String(function_name));
        break;
    case WebCL::KERNEL_NUM_ARGS:
        err = clGetKernelInfo(m_cl_kernel, CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &uint_units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uint_units));
        break;
    case WebCL::KERNEL_REFERENCE_COUNT:
        err = clGetKernelInfo(m_cl_kernel, CL_KERNEL_REFERENCE_COUNT, sizeof(cl_uint), &uint_units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uint_units));
        break;
    case WebCL::KERNEL_PROGRAM:
        err = clGetKernelInfo(m_cl_kernel, CL_KERNEL_PROGRAM, sizeof(cl_program_id), &cl_program_id, 0);
        programObj = WebCLProgram::create(m_context, cl_program_id);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(PassRefPtr<WebCLProgram>(programObj));
        break;
    // FIXME: we should not create a context here
    /*
    case WebCL::KERNEL_CONTEXT:
        err = clGetKernelInfo(m_cl_kernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &cl_context_id, 0);
        contextObj = WebCLContext::create(m_context, cl_context_id);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
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

WebCLGetInfo WebCLKernel::getWorkGroupInfo(WebCLDevice* device, int param_name, ExceptionCode& ec)
{
    CCerror err = 0;
    cl_device_id cl_device = 0;
    size_t sizet_units = 0;
    cl_ulong ulong_units = 0;

    if (!m_cl_kernel) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    if (device) {
        // FIXME: s/getCLDevice/getComputeContextDevice
        cl_device = device->getCLDevice();
        if (!cl_device) {
            printf("Error: cl_device null\n");
            ec = WebCLException::INVALID_DEVICE;
            return WebCLGetInfo();
        }
    }
    switch (param_name) {
    case WebCL::KERNEL_WORK_GROUP_SIZE:
        err = clGetKernelWorkGroupInfo(m_cl_kernel, cl_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(sizet_units));
        break;
    case WebCL::KERNEL_COMPILE_WORK_GROUP_SIZE:
        err = clGetKernelWorkGroupInfo(m_cl_kernel, cl_device, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(sizet_units));
        break;
    case WebCL::KERNEL_LOCAL_MEM_SIZE:
        err = clGetKernelWorkGroupInfo(m_cl_kernel, cl_device, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &ulong_units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
        break;
    default:
        printf("Error: Unsupported Kernrl Info type\n");
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    printf("Error: clGetKerelWorkGroupInfo\n");
    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

void WebCLKernel::setArg(unsigned argIndex, unsigned argSize, ExceptionCode& ec)
{
    if (!m_cl_kernel) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    CCerror err = clSetKernelArg(m_cl_kernel, argIndex, argSize, 0);
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

    if (!m_cl_kernel) {
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
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdCh, argIndex, sizeof(cl_char));
        break;
    case WebCL::UCHAR:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduCh, argIndex, sizeof(cl_uchar));
        break;
    case WebCL::SHORT:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdSh, argIndex, sizeof(cl_short));
        break;
    case WebCL::USHORT:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduSh, argIndex, sizeof(cl_ushort));
        break;
    case WebCL::INT:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdInt, argIndex, sizeof(cl_int));
        break;
    case WebCL::UINT:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduInt, argIndex, sizeof(cl_uint));
        break;
    case WebCL::LONG:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdLong, argIndex, sizeof(cl_long));
        break;
    case WebCL::ULONG:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduLong, argIndex, sizeof(cl_ulong));
        break;
    case WebCL::FLOAT_KERNEL_ARG:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdFloat, argIndex, sizeof(cl_float));
        break;
    case WebCL::HALF:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdFloat, argIndex, sizeof(cl_half));
        break;
    case WebCL::DOUBLE:
        err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdFloat, argIndex, sizeof(cl_double));
        break;
    case WebCL::CHAR | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_char2), 2);
        break;
    case WebCL::UCHAR | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_uchar2), 2);
        break;
    case WebCL::SHORT | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_short2), 2);
        break;
    case WebCL::USHORT | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_ushort2), 2);
        break;
    case WebCL::INT | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_int2), 2);
        break;
    case WebCL::UINT | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_uint2), 2);
        break;
    case WebCL::LONG | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_long2), 2);
        break;
    case WebCL::ULONG | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_ulong2), 2);
        break;
    case WebCL::FLOAT_KERNEL_ARG | WebCL::VEC2:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_float2), 2);
        break;
    case WebCL::CHAR | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_char4), 4);
        break;
    case WebCL::UCHAR | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_uchar4), 4);
        break;
    case WebCL::SHORT | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_short4), 4);
        break;
    case WebCL::USHORT | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_ushort4), 4);
        break;
    case WebCL::INT | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_int4), 4);
        break;
    case WebCL::UINT | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_uint4), 4);
        break;
    case WebCL::LONG | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_long4), 4);
        break;
    case WebCL::ULONG | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_ulong4), 4);
        break;
    case WebCL::FLOAT_KERNEL_ARG | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_float4), 4);
        break;
    case WebCL::DOUBLE | WebCL::VEC4:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_double4), 4);
        break;
    case WebCL::CHAR | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_char8), 8);
        break;
    case WebCL::UCHAR | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_uchar4), 4);
        break;
    case WebCL::SHORT | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_short8), 8);
        break;
    case WebCL::USHORT | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_ushort8), 8);
        break;
    case WebCL::INT | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_int8), 8);
        break;
    case WebCL::UINT | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_uint8), 8);
        break;
    case WebCL::LONG | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_long8), 8);
        break;
    case WebCL::ULONG | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_ulong8), 8);
        break;
    case WebCL::FLOAT_KERNEL_ARG | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_float8), 8);
        break;
    case WebCL::DOUBLE | WebCL::VEC8:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_double8), 8);
        break;
    case WebCL::CHAR | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_char8), 8);
        break;
    case WebCL::UCHAR | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_uchar16), 16);
        break;
    case WebCL::SHORT | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_short16), 16);
        break;
    case WebCL::USHORT | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_ushort16), 16);
        break;
    case WebCL::INT | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_int16), 16);
        break;
    case WebCL::UINT | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_uint16), 16);
        break;
    case WebCL::LONG | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_long16), 16);
        break;
    case WebCL::ULONG | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_ulong16), 16);
        break;
    case WebCL::FLOAT_KERNEL_ARG | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_float16), 16);
        break;
    case WebCL::DOUBLE | WebCL::VEC16:
        err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array, argIndex, sizeof(cl_double16), 16);
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
    cl_mem cl_mem_id = 0;
    cl_device_id cl_device = 0;

    if (!m_cl_kernel) {
        ec = WebCLException::INVALID_KERNEL;
        printf("Error: Invalid kernel\n");
        return;
    }
    if (argValue) {
        cl_mem_id = argValue->getCLMemoryObject();
        if (!cl_mem_id) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error: cl_mem_id null\n");
            return;
        }
    }
    if (m_device_id) {
        cl_device = m_device_id->getCLDevice();
        cl_ulong max_buffer_size = 0;
        clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &max_buffer_size, 0);
        cl_uint max_args = 0;
        clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &max_args, 0);
        // Check for __constant qualifier restrictions
        if (!(sizeof(cl_mem) <= max_buffer_size) && (argIndex <= max_args)) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error::setArg::WebCLMemoryObject is not valid to call with __constant restriction\n");
            return;
        }
    }
    // No device or valid memory object.
    err = clSetKernelArg(m_cl_kernel, argIndex, sizeof(cl_mem), &cl_mem_id);

    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(unsigned argIndex, unsigned argValue, unsigned argSize, ExceptionCode& ec)
{
    CCerror err = 0;
    if (!m_cl_kernel) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    err = clSetKernelArg(m_cl_kernel, argIndex, argSize, &argValue);
    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

/*
// TODO (siba samal) Is this API is needed??
unsigned long WebCLKernel::getKernelWorkGroupInfo(WebCLDeviceList* devices, int param_name)
{
    cl_int err = 0;
    cl_device_id cl_device = 0;
    size_t ret = 0;

    if (!m_cl_kernel) {
        printf("Error: Invalid kernel\n");
        return 0;
    }

    if (devices) {
        cl_device = devices->getCLDevices();
        if (!cl_device) {
            printf("Error: cl_device null\n");
            return 0;
        }
    }
    switch (param_name) {
    case KERNEL_WORK_GROUP_SIZE:
        err = clGetKernelWorkGroupInfo(m_cl_kernel, cl_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(ret), &ret, 0);
        if (err != CL_SUCCESS) {
            switch (err) {
            case CL_INVALID_DEVICE:
                printf("Error: CL_INVALID_DEVICE\n");
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE\n");
                break;
            case CL_INVALID_KERNEL:
                printf("Error: CL_INVALID_KERNEL\n");
                break;
            default:
                printf("Error: Invaild Error Type\n");
                break;
            }
        } else
            return ret;
        break;
    default:
        break;
    }

    return 0;
}
*/

template<class T> inline unsigned WebCLKernel::clSetKernelArgPrimitiveType(cl_kernel cl_kernel_id,
    PassRefPtr<WebCLKernelTypeValue> kernelObject,
    T nodeId, unsigned argIndex, int size)
{
    if (!kernelObject->asNumber(&nodeId)) {
        printf("Error: WebCL Kernel Type Value Not Proper\n");
        return -1;
    }

    CCerror err = clSetKernelArg(cl_kernel_id, argIndex, size, &nodeId);
    return err;
}

inline unsigned WebCLKernel::clSetKernelArgVectorType(cl_kernel cl_kernel_id,
    PassRefPtr<WebCLKernelTypeValue> kernelObject,
    RefPtr<WebCLKernelTypeVector> array, unsigned argIndex,
    int size, unsigned length)
{
    if ((!kernelObject->asVector(&array)) || (length != array->length())) {
        printf("Error: Invalid WebCL Kernel Type %d\n", array->length());
        return -1;
    }

    CCerror err = clSetKernelArg(cl_kernel_id, argIndex, size, &array);
    return err;
}

void WebCLKernel::setDevice(RefPtr<WebCLDevice> m_device_id_)
{
    m_device_id = m_device_id_;
}

cl_kernel WebCLKernel::getCLKernel()
{
    return m_cl_kernel;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
