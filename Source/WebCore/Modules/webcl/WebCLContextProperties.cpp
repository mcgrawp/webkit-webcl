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

#if ENABLE(WEBCL)

#include "WebCLContextProperties.h"

namespace WebCore {

PassRefPtr<WebCLContextProperties> WebCLContextProperties::create()
{
    return adoptRef(new WebCLContextProperties());
}

PassRefPtr<WebCLContextProperties> WebCLContextProperties::create(PassRefPtr<WebCLPlatform> platform, const Vector<RefPtr<WebCLDevice> >& devices, CCenum deviceType, PassRefPtr<WebGLRenderingContext> webGLRenderingContext)
{
    return adoptRef(new WebCLContextProperties(platform, devices, deviceType, webGLRenderingContext));
}

WebCLContextProperties::WebCLContextProperties()
    : m_deviceType(ComputeContext::DEVICE_TYPE_DEFAULT)
{
}

WebCLContextProperties::WebCLContextProperties(PassRefPtr<WebCLPlatform> platform, const Vector<RefPtr<WebCLDevice> >& devices, CCenum deviceType, PassRefPtr<WebGLRenderingContext> webGLRenderingContext)
    : m_platform(platform)
    , m_devices(devices)
    , m_deviceType(deviceType)
#if ENABLE(WEBGL)
    , m_webGLRenderingContext(webGLRenderingContext)
#endif
{
}

WebCLPlatform* WebCLContextProperties::platform()
{
    return m_platform.get();
}

void WebCLContextProperties::setPlatform(PassRefPtr<WebCLPlatform> platform)
{
    m_platform = platform;
    m_ccProperties.clear();
}

const Vector<RefPtr<WebCLDevice> >& WebCLContextProperties::devices() const
{
    return m_devices;
}

void WebCLContextProperties::setDevices(const Vector<RefPtr<WebCLDevice> >& devices)
{
    m_devices = devices;
    m_ccProperties.clear();
}

CCenum WebCLContextProperties::deviceType() const
{
    return m_deviceType;
}

void WebCLContextProperties::setDeviceType(CCenum type)
{
    m_deviceType = type;
    m_ccProperties.clear();
}

#if ENABLE(WEBGL)
WebGLRenderingContext* WebCLContextProperties::glContext()
{
    return m_webGLRenderingContext.get();
}

void WebCLContextProperties::setGLContext(PassRefPtr<WebGLRenderingContext> webGLRenderingContext)
{
    m_webGLRenderingContext = webGLRenderingContext;
    m_ccProperties.clear();
}
#endif

bool WebCLContextProperties::isGLCapable() const
{
#if ENABLE(WEBGL)
    return !!m_webGLRenderingContext.get();
#else
    return false;
#endif
}

Vector<CCContextProperties>& WebCLContextProperties::computeContextProperties()
{
    if (m_ccProperties.size())
        return m_ccProperties;

    if (m_platform) {
        m_ccProperties.append(ComputeContext::CONTEXT_PLATFORM);
        m_ccProperties.append(reinterpret_cast<CCContextProperties>(platform()->platformObject()));
    }

#if ENABLE(WEBGL)
    if (m_webGLRenderingContext)
        ComputeContext::populatePropertiesForInteroperabilityWithGL(m_ccProperties, m_webGLRenderingContext->graphicsContext3D()->platformGraphicsContext3D());
#endif

    m_ccProperties.append(0);
    return m_ccProperties;
}

}
#endif // ENABLE(WEBCL)
