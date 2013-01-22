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

#include "WebCLDevice.h"

#include "WebCL.h"
#include "WebCLException.h"

namespace WebCore {

WebCLDevice::~WebCLDevice()
{
}

PassRefPtr<WebCLDevice> WebCLDevice::create(cl_device_id deviceID)
{
    return adoptRef(new WebCLDevice(deviceID));
}

WebCLDevice::WebCLDevice(cl_device_id deviceID)
    : m_cl_device_id(deviceID)
{
}

WebCLGetInfo WebCLDevice::getInfo(int infoType, ExceptionCode& ec)
{
    CCerror err = 0;
    char deviceString[1024];
    CCuint uint_units = 0;
    size_t sizet_units = 0;
    size_t sizet_array[1024] = {0};
    cl_ulong ulong_units = 0;
    cl_bool bool_units = false;
    cl_device_type type = 0;
    cl_device_mem_cache_type momoryCacheType = 0;
    cl_command_queue_properties queueProperties = 0;
    cl_device_exec_capabilities exec = 0;
    cl_device_local_mem_type localMemoryType = 0;
    cl_device_fp_config fpConfig = 0;
    // cl_platform_id platformID;
    char platformID[1024];
    if (!m_cl_device_id) {
        printf("Error: Invalid Device ID\n");
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }

    switch (infoType) {
    case WebCL::DEVICE_EXTENSIONS:
    case WebCL::DEVICE_NAME:
    case WebCL::DEVICE_PROFILE:
    case WebCL::DEVICE_VENDOR:
    case WebCL::DEVICE_VERSION:
    case WebCL::DEVICE_OPENCL_C_VERSION:
    case WebCL::DRIVER_VERSION:
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(deviceString), &deviceString);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(String(deviceString));
        break;
    case WebCL::MEM_READ_ONLY:
    case WebCL::DEVICE_ADDRESS_BITS:
    case WebCL::DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
    case WebCL::DEVICE_MAX_CLOCK_FREQUENCY:
    case WebCL::DEVICE_MAX_CONSTANT_ARGS:
    case WebCL::DEVICE_MAX_READ_IMAGE_ARGS: // unsigned int
    case WebCL::DEVICE_MAX_WRITE_IMAGE_ARGS: // unsigned int
    case WebCL::DEVICE_MAX_SAMPLERS:
    case WebCL::DEVICE_MEM_BASE_ADDR_ALIGN:
    case WebCL::DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
    case WebCL::DEVICE_VENDOR_ID:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_INT:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
    case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
    case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
    case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_INT:
    case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_LONG:
    case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
    case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
    case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_HALF:
    case WebCL::DEVICE_MAX_COMPUTE_UNITS:
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(cl_uint), &uint_units);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uint_units));
        break;
    case WebCL::DEVICE_IMAGE2D_MAX_HEIGHT:
    case WebCL::DEVICE_IMAGE2D_MAX_WIDTH:
    case WebCL::DEVICE_MAX_PARAMETER_SIZE:
    case WebCL::DEVICE_MAX_WORK_GROUP_SIZE:
    case WebCL::DEVICE_PROFILING_TIMER_RESOLUTION: // cl_bool
    case WebCL::DEVICE_MAX_WORK_ITEM_DIMENSIONS: // unsigned int
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(size_t), &sizet_units);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(sizet_units));
        break;
    case WebCL::DEVICE_MAX_WORK_ITEM_SIZES: // size_t[]
        err = ComputeContext::getDeviceInfo(m_cl_device_id, WebCL::DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &sizet_units);
        if (err == ComputeContext::SUCCESS) {
            err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, 1024, &sizet_array);
            // FIXME: Check support for SizeTArray/Int16Array in WebCLGetInfo
            if (err == ComputeContext::SUCCESS) {
                int values[3] = {0, 0, 0};
                for (int i = 0; i < (int) sizet_units; ++i) {
                    values[i] = (int) sizet_array[i];
                    printf("%d\n", values[i]);
                }
                return WebCLGetInfo(Int32Array::create(values, 3));
            }
        }
        break;
    case WebCL::DEVICE_LOCAL_MEM_SIZE:
    case WebCL::DEVICE_MAX_CONSTANT_BUFFER_SIZE:
    case WebCL::DEVICE_MAX_MEM_ALLOC_SIZE: // unsigned long long
    case WebCL::DEVICE_GLOBAL_MEM_CACHE_SIZE:
    case WebCL::DEVICE_GLOBAL_MEM_SIZE:
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(cl_ulong), &ulong_units);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
        break;
    case WebCL::DEVICE_AVAILABLE:
    case WebCL::DEVICE_COMPILER_AVAILABLE:
    case WebCL::DEVICE_ENDIAN_LITTLE:
    case WebCL::DEVICE_ERROR_CORRECTION_SUPPORT:
    case WebCL::DEVICE_HOST_UNIFIED_MEMORY:
    case WebCL::DEVICE_IMAGE_SUPPORT:
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(cl_bool), &bool_units);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<bool>(bool_units));
        break;
    case WebCL::DEVICE_TYPE:
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(type), &type);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(type));
        break;
    case WebCL::DEVICE_QUEUE_PROPERTIES:
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(cl_command_queue_properties), &queueProperties);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(queueProperties));
        break;
    case WebCL::DEVICE_SINGLE_FP_CONFIG:
    case WebCL::DEVICE_DOUBLE_FP_CONFIG:
    case WebCL::DEVICE_HALF_FP_CONFIG:
        // Part of cl_ext.h (which isn't available in Khronos).
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(cl_device_fp_config), &fpConfig);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(fpConfig));
        break;
    case WebCL::DEVICE_PLATFORM: // cl_platform_id
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(platformID), &platformID);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(String(platformID));
        break;
    case WebCL::DEVICE_EXECUTION_CAPABILITIES:
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(cl_device_exec_capabilities), &exec);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(exec));
        break;
    case WebCL::DEVICE_GLOBAL_MEM_CACHE_TYPE:
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(cl_device_mem_cache_type), &momoryCacheType);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(momoryCacheType));
        break;
    case WebCL::DEVICE_LOCAL_MEM_TYPE:
        err = ComputeContext::getDeviceInfo(m_cl_device_id, infoType, sizeof(cl_device_local_mem_type), &localMemoryType);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(localMemoryType));
        break;
    default:
        ec = WebCLException::FAILURE;
        ASSERT_NOT_REACHED();
        return WebCLGetInfo();
    }

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

Vector<String> WebCLDevice::getSupportedExtensions(ExceptionCode&)
{
    // FIXME: Needs a proper implementation.
    return Vector<String>();
}

cl_device_id WebCLDevice::getCLDevice()
{
    return m_cl_device_id;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
