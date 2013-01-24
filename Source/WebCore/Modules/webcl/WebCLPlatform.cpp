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
        printf("Error: Invalid Platform ID\n");
        return WebCLGetInfo();
    }

    CCerror err = 0;
    switch(platform_info) {
    case WebCL::PLATFORM_PROFILE:
    case WebCL::PLATFORM_VERSION:
    {
        char platform_string[1024];
        err = ComputeContext::getPlatformInfo(m_cl_platform_id, platform_info, sizeof(platform_string), &platform_string);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(String(platform_string));
        break;
    }
    default:
        printf("Error: Unsupported Platform Info type = %d ", platform_info);
        ec = WebCLException::FAILURE;
        return WebCLGetInfo();
    }

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

PassRefPtr<WebCLDeviceList> WebCLPlatform::getDevices(int device_type, ExceptionCode& ec)
{
    if (!m_cl_platform_id) {
        ec = WebCLException::INVALID_PLATFORM;
        return 0;
    }

    if (!device_type)
        device_type = WebCL::DEVICE_TYPE_DEFAULT;

    CCerror error;
    RefPtr<WebCLDeviceList> webCLDeviceList = WebCLDeviceList::create(m_cl_platform_id, device_type, error);
    if (!webCLDeviceList) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    return webCLDeviceList;
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
