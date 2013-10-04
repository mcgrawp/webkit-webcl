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

#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLGetInfo.h"
#include "WebCLImageDescriptor.h"
#include "WebCLInputChecker.h"
#include "WebCLMemoryObject.h"
#include "WebCLPlatform.h"
#include "WebCLProgram.h"

namespace WebCore {

WebCLDevice::~WebCLDevice()
{
}

PassRefPtr<WebCLDevice> WebCLDevice::create(CCDeviceID deviceID, const WebCLPlatform* platform)
{
    return adoptRef(new WebCLDevice(deviceID, platform));
}

WebCLDevice::WebCLDevice(CCDeviceID deviceID, const WebCLPlatform* platform)
    : WebCLObject(deviceID)
    , WebCLExtensionsAccessor(deviceID)
    , m_platform(platform)
{
}

WebCLGetInfo WebCLDevice::getInfo(int infoType, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }

    if (!WebCLInputChecker::isValidDeviceInfoType(infoType)) {
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    CCerror err = 0;
    switch (infoType) {
    case ComputeContext::DEVICE_EXTENSIONS:
    case ComputeContext::DEVICE_OPENCL_C_VERSION: {
        Vector<char> deviceString;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType, &deviceString);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(String(deviceString.data()));
        }
        break;
    case ComputeContext::DEVICE_PROFILE:
        return WebCLGetInfo(String("WEBCL_PROFILE"));
    case ComputeContext::DEVICE_VERSION:
        return WebCLGetInfo(String("WebCL 1.0"));
    case ComputeContext::DEVICE_ADDRESS_BITS:
    case ComputeContext::DEVICE_MAX_CONSTANT_ARGS:
    case ComputeContext::DEVICE_MAX_READ_IMAGE_ARGS: // unsigned int
    case ComputeContext::DEVICE_MAX_WRITE_IMAGE_ARGS: // unsigned int
    case ComputeContext::DEVICE_MAX_SAMPLERS:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_INT:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_HALF: 
    case ComputeContext::DEVICE_MEM_BASE_ADDR_ALIGN: 
    case ComputeContext::DEVICE_MAX_COMPUTE_UNITS: {
        CCuint infoValue = 0;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType, &infoValue);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(infoValue));
        }
        break;
    case ComputeContext::DEVICE_IMAGE2D_MAX_HEIGHT:
    case ComputeContext::DEVICE_IMAGE2D_MAX_WIDTH:
    case ComputeContext::DEVICE_MAX_PARAMETER_SIZE:
    case ComputeContext::DEVICE_MAX_WORK_GROUP_SIZE:
    case ComputeContext::DEVICE_MAX_WORK_ITEM_DIMENSIONS: { // unsigned int
        size_t infoValue = 0;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType, &infoValue);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(infoValue));
        }
        break;
    case ComputeContext::DEVICE_MAX_WORK_ITEM_SIZES: { // size_t[]
        Vector<size_t> workItemSizes;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType, &workItemSizes);
        if (err == ComputeContext::SUCCESS) {
            int values[3] = {0, 0, 0};
            for (size_t i = 0; i < workItemSizes.size(); ++i)
                values[i] = (int) workItemSizes[i];
            return WebCLGetInfo(Int32Array::create(values, 3));
        }
        break;
    }
    case ComputeContext::DEVICE_LOCAL_MEM_SIZE:
    case ComputeContext::DEVICE_MAX_CONSTANT_BUFFER_SIZE:
    case ComputeContext::DEVICE_GLOBAL_MEM_SIZE:
    case ComputeContext::DEVICE_MAX_MEM_ALLOC_SIZE: { // unsigned long long
        CCulong infoValue = 0;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType, &infoValue);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned long>(infoValue));
        break;
    }
    case ComputeContext::DEVICE_AVAILABLE:
    case ComputeContext::DEVICE_ENDIAN_LITTLE:
    case ComputeContext::DEVICE_HOST_UNIFIED_MEMORY: 
    case ComputeContext::DEVICE_COMPILER_AVAILABLE: {
        CCbool infoValue = 0;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType, &infoValue);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<bool>(infoValue));
        break;
    }
    case ComputeContext::DEVICE_IMAGE_SUPPORT:
        return WebCLGetInfo(true);
    case ComputeContext::DEVICE_TYPE: {
        CCDeviceType type = 0;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType, &type);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(type));
        break;
    }
    case ComputeContext::DEVICE_QUEUE_PROPERTIES: {
        CCCommandQueueProperties queueProperties = 0;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType,  &queueProperties);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(queueProperties));
        break;
    }
    case ComputeContext::DEVICE_PLATFORM:
        return WebCLGetInfo(const_cast<WebCLPlatform*>(m_platform));
    case ComputeContext::DEVICE_LOCAL_MEM_TYPE: {
        CCDeviceLocalMemType localMemoryType = 0;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType, &localMemoryType);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(localMemoryType));
        break;
    }
    case ComputeContext::DEVICE_SINGLE_FP_CONFIG: {
        CCDeviceFPConfig deviceFPConfig = 0;
        err = ComputeContext::getDeviceInfo(platformObject(), infoType, &deviceFPConfig);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned long>(deviceFPConfig));
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

void toWebCLDeviceArray(const WebCLPlatform* platform, const Vector<CCDeviceID>& ccDevices, Vector<RefPtr<WebCLDevice> >& devices)
{
    for (size_t i = 0; i < ccDevices.size(); i++) {
        RefPtr<WebCLDevice> webCLDevice = WebCLDevice::create(ccDevices[i], platform);
        devices.append(webCLDevice);
    }
}

const WebCLPlatform* WebCLDevice::platform()
{
    return m_platform;
}

} // namespace WebCore
#endif // ENABLE(WEBCL)
