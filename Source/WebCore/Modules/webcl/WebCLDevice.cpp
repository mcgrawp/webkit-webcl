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

PassRefPtr<WebCLDevice> WebCLDevice::create(CCDeviceID deviceID)
{
    return adoptRef(new WebCLDevice(deviceID));
}

WebCLDevice::WebCLDevice(CCDeviceID deviceID)
    : m_ccDeviceID(deviceID)
{
}

WebCLGetInfo WebCLDevice::getInfo(int infoType, ExceptionCode& ec)
{
    CCerror err = 0;
    char deviceString[WebCL::CHAR_BUFFER_SIZE];
    CCuint uintUnits = 0;
    size_t sizetUnits = 0;
    size_t sizetArray[WebCL::CHAR_BUFFER_SIZE] = {0};
    CCulong ulongUnits = 0;
    CCbool boolUnits = false;
    CCDeviceType type = 0;
    CCCommandQueueProperties queueProperties = 0;
    CCDeviceLocalMemType localMemoryType = 0;
    CCDeviceFPConfig fpConfig = 0;
    char platformID[1024];

    if (!m_ccDeviceID) {
        printf("Error: Invalid Device ID\n");
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }

    switch (infoType) {
    case WebCL::DEVICE_EXTENSIONS:
    case WebCL::DEVICE_PROFILE:
    case WebCL::DEVICE_OPENCL_C_VERSION:
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(deviceString), &deviceString);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(String(deviceString));
        break;
    case WebCL::MEM_READ_ONLY:
    case WebCL::DEVICE_ADDRESS_BITS:
    case WebCL::DEVICE_MAX_CONSTANT_ARGS:
    case WebCL::DEVICE_MAX_READ_IMAGE_ARGS: // unsigned int
    case WebCL::DEVICE_MAX_WRITE_IMAGE_ARGS: // unsigned int
    case WebCL::DEVICE_MAX_SAMPLERS:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_INT:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
    case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(CCuint), &uintUnits);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uintUnits));
        break;
    case WebCL::DEVICE_IMAGE2D_MAX_HEIGHT:
    case WebCL::DEVICE_IMAGE2D_MAX_WIDTH:
    case WebCL::DEVICE_MAX_PARAMETER_SIZE:
    case WebCL::DEVICE_MAX_WORK_GROUP_SIZE:
    case WebCL::DEVICE_MAX_WORK_ITEM_DIMENSIONS: // unsigned int
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(size_t), &sizetUnits);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(sizetUnits));
        break;
    case WebCL::DEVICE_MAX_WORK_ITEM_SIZES: // size_t[]
        {
            err = ComputeContext::getDeviceInfo(m_ccDeviceID, WebCL::DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &sizetUnits);
            if (err == ComputeContext::SUCCESS) {
                err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, 1024, &sizetArray);
                // FIXME: Check support for SizeTArray/Int16Array in WebCLGetInfo
                if (err == ComputeContext::SUCCESS) {
                    int values[3] = {0, 0, 0};
                    for (int i = 0; i < (int)sizetUnits; ++i)
                        values[i] = (int) sizetArray[i];
                    return WebCLGetInfo(Int32Array::create(values, 3));
                }
            }
        }
        break;
    case WebCL::DEVICE_LOCAL_MEM_SIZE:
    case WebCL::DEVICE_MAX_CONSTANT_BUFFER_SIZE:
    case WebCL::DEVICE_MAX_MEM_ALLOC_SIZE: // unsigned long long
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(CCulong), &ulongUnits);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned long>(ulongUnits));
        break;
    case WebCL::DEVICE_AVAILABLE:
    case WebCL::DEVICE_ENDIAN_LITTLE:
    case WebCL::DEVICE_HOST_UNIFIED_MEMORY:
    case WebCL::DEVICE_IMAGE_SUPPORT:
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(CCbool), &boolUnits);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<bool>(boolUnits));
        break;
    case WebCL::DEVICE_TYPE:
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(type), &type);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(type));
        break;
    case WebCL::DEVICE_QUEUE_PROPERTIES:
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(CCCommandQueueProperties), &queueProperties);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(queueProperties));
        break;
    case WebCL::DEVICE_DOUBLE_FP_CONFIG:
    case WebCL::DEVICE_HALF_FP_CONFIG:
        // Part of cl_ext.h (which isn't available in Khronos).
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(CCDeviceFPConfig), &fpConfig);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(fpConfig));
        break;
    case WebCL::DEVICE_PLATFORM: // cl_platform_id
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(platformID), &platformID);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(String(platformID));
        break;
    case WebCL::DEVICE_LOCAL_MEM_TYPE:
        err = ComputeContext::getDeviceInfo(m_ccDeviceID, infoType, sizeof(CCDeviceLocalMemType), &localMemoryType);
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

CCDeviceID WebCLDevice::getCLDevice()
{
    return m_ccDeviceID;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
