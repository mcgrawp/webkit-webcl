/*
 * Copyright (C) 2011, 2012, 2013 Samsung Electronics Corporation. All rights reserved.
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
#include "WebCLContextProperties.h"

#include "WebCLDevice.h"

#include <OpenGL/OpenGL.h>

#if ENABLE(WEBCL)

namespace WebCore {

PassRefPtr<WebCLContextProperties> WebCLContextProperties::create(PassRefPtr<WebCLPlatform> platform, const Vector<RefPtr<WebCLDevice> >& devices, int deviceType, int shareGroup, const String& hint)
{
    return adoptRef(new WebCLContextProperties(platform, devices, deviceType, shareGroup, hint));
}

WebCLContextProperties::WebCLContextProperties(PassRefPtr<WebCLPlatform> platform, const Vector<RefPtr<WebCLDevice> >& devices, int deviceType, int shareGroup, const String& hint)
    : objplatform(platform)
    , objdevices(devices)
    , objdeviceType(deviceType)
    , objshareGroup(shareGroup)
    , objhint(hint)
{
}

RefPtr<WebCLPlatform>& WebCLContextProperties::platform()
{
    return objplatform;
}

void WebCLContextProperties::setPlatform(PassRefPtr<WebCLPlatform> platform)
{
    objplatform = platform;
    m_ccProperties.clear();
}

Vector<RefPtr<WebCLDevice> >& WebCLContextProperties::devices()
{
    return objdevices;
}

int WebCLContextProperties::deviceType() const
{
    return objdeviceType;
}

void WebCLContextProperties::setDeviceType(int type)
{
    objdeviceType = type;
    m_ccProperties.clear();
}

int WebCLContextProperties::shareGroup() const
{
    return objshareGroup;
}

void WebCLContextProperties::setShareGroup(int shareGroup)
{
    objshareGroup = shareGroup;
    m_ccProperties.clear();
}

String WebCLContextProperties::hint() const
{
    return objhint;
}

void WebCLContextProperties::setHint(const String& hint)
{
    objhint = hint;
}

Vector<CCContextProperties>& WebCLContextProperties::computeContextProperties()
{
    if (m_ccProperties.size())
        return m_ccProperties;

    if (objplatform) {
        m_ccProperties.append(ComputeContext::CONTEXT_PLATFORM);
        m_ccProperties.append(reinterpret_cast<CCContextProperties>(platform()->getCLPlatform()));
    }

    if (objshareGroup) {
        CGLContextObj kCGLContext = CGLGetCurrentContext();
        CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
        m_ccProperties.append(CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE);
        m_ccProperties.append(reinterpret_cast<CCContextProperties>(kCGLShareGroup));
    }

    m_ccProperties.append(0);

    return m_ccProperties;
}

}
#endif // ENABLE(WEBCL)
