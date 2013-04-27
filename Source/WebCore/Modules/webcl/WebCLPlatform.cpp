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

#include "ComputeExtensions.h"
#include "WebCL.h"
#include "WebCLException.h"
#include "WebCLInputChecker.h"

namespace WebCore {

WebCLPlatform::~WebCLPlatform()
{
}

PassRefPtr<WebCLPlatform> WebCLPlatform::create(CCPlatformID platformID)
{
    return adoptRef(new WebCLPlatform(platformID));
}

WebCLPlatform::WebCLPlatform(CCPlatformID platformID)
    : WebCLObject(platformID)
{
}

WebCLGetInfo WebCLPlatform::getInfo(int platform_info, ExceptionCode& ec)
{
    if (!platformObject()) {
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
    if (!platformObject()) {
        ec = WebCLException::INVALID_PLATFORM;
        return devices;
    }

    if (deviceType && !WebCLInputChecker::isValidDeviceType(deviceType)) {
        ec = WebCLException::INVALID_VALUE;
        return devices;
    }

    if (!deviceType)
        deviceType = ComputeContext::DEVICE_TYPE_DEFAULT;

    Vector<CCDeviceID> ccDevices;
    CCerror error = ComputeContext::getDeviceIDs(platformObject(), deviceType, ccDevices);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return devices;
    }

    toWebCLDeviceArray(ccDevices, devices);
    return devices;
}

Vector<String> WebCLPlatform::getSupportedExtensions(ExceptionCode&)
{
    Vector<String> result;
    if (ComputeExtensions::get().supports("cl_khr_gl_sharing", platformObject()))
        result.append("KHR_GL_SHARING");

    return result;
}

CCerror getPlatforms(Vector<RefPtr<WebCLPlatform> >& platforms)
{
    Vector<CCPlatformID> ccPlatforms;

    CCerror error = ComputeContext::getPlatformIDs(ccPlatforms);
    if (error != ComputeContext::SUCCESS)
        return error;

    for (size_t i = 0; i < ccPlatforms.size(); ++i)
        platforms.append(WebCLPlatform::create(ccPlatforms[i]));

    return error;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
