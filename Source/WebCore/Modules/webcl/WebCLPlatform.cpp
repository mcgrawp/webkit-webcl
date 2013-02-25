/*
* Copyright (C) 2011, 2012, 2013 Samsung Electronics Corporation.
* All rights reserved.
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

#include "WebCLPlatform.h"
#include "WebCL.h"
#include "WebCLException.h"

namespace WebCore {

WebCLPlatform::~WebCLPlatform()
{
}

PassRefPtr<WebCLPlatform> WebCLPlatform::create(cl_platform_id platform_id)
{
    return adoptRef(new WebCLPlatform(platform_id));
}

WebCLPlatform::WebCLPlatform(cl_platform_id platform_id)
 : m_cl_platform_id(platform_id)
{
}

WebCLGetInfo WebCLPlatform::getInfo(int platform_info, ExceptionCode& ec)
{
    if (!m_cl_platform_id) {
        ec = WebCLException::INVALID_PLATFORM;
        return WebCLGetInfo();
    }

    switch(platform_info) {
    case ComputeContext::PLATFORM_PROFILE:
        return WebCLGetInfo(String("WEBCL_PROFILE"));
    case ComputeContext::PLATFORM_VERSION:
        return WebCLGetInfo(String("WebCL 1.0"));
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT_NOT_REACHED();
    return WebCLGetInfo();
}

Vector<RefPtr<WebCLDevice> > WebCLPlatform::getDevices(int deviceType, ExceptionCode& ec) const
{
    Vector<RefPtr<WebCLDevice> > devices;
    if (!m_cl_platform_id) {
        ec = WebCLException::INVALID_PLATFORM;
        return devices;
    }

    deviceType = ComputeContext::DEVICE_TYPE_DEFAULT;
    Vector<CCDeviceID> ccDevices;
    CCerror error = ComputeContext::getDeviceIDs(m_cl_platform_id, deviceType, ccDevices);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return devices;
    }

    toWebCLDeviceArray(ccDevices, devices);
    return devices;
}

Vector<String> WebCLPlatform::getSupportedExtensions(ExceptionCode&)
{
    // FIXME: Needs a proper implementation.
    return Vector<String>();
}

cl_platform_id WebCLPlatform::getCLPlatform()
{
    return m_cl_platform_id;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
