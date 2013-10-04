/*
 * Copyright (C) 2013 Samsung Electronics Corporation. All rights reserved.
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

#include "WebCLGLContextProperties.h"

namespace WebCore {

PassRefPtr<WebCLGLContextProperties> WebCLGLContextProperties::create()
{
    return adoptRef(new WebCLGLContextProperties());
}

PassRefPtr<WebCLGLContextProperties> WebCLGLContextProperties::create(PassRefPtr<WebCLPlatform> platform, const Vector<RefPtr<WebCLDevice> >& devices, unsigned long deviceType, SharedContextResolutionPolicy contextPolicy, PassRefPtr<WebGLRenderingContext> webGLRenderingContext)
{
    if (contextPolicy == UseGLContextProvided)
        ASSERT(webGLRenderingContext);
    else
        ASSERT(!webGLRenderingContext);
    return adoptRef(new WebCLGLContextProperties(platform, devices, deviceType, contextPolicy, webGLRenderingContext));
}

WebCLGLContextProperties::WebCLGLContextProperties()
    : WebCLContextProperties()
{
    m_contextPolicy = NoSharedGLContext;
}

WebCLGLContextProperties::WebCLGLContextProperties(PassRefPtr<WebCLPlatform> platform, const Vector<RefPtr<WebCLDevice> >& devices, unsigned long deviceType, SharedContextResolutionPolicy contextPolicy, PassRefPtr<WebGLRenderingContext> webGLRenderingContext)
    : WebCLContextProperties(platform, devices, deviceType)
{
    m_contextPolicy = contextPolicy;
    m_webGLRenderingContext = webGLRenderingContext;
}

RefPtr<WebGLRenderingContext>& WebCLGLContextProperties::sharedContext()
{
    return m_webGLRenderingContext;
}

void WebCLGLContextProperties::setSharedContext(PassRefPtr<WebGLRenderingContext> webGLRenderingContext)
{
    m_contextPolicy = webGLRenderingContext ? UseGLContextProvided : GetCurrentGLContext;
    m_webGLRenderingContext = webGLRenderingContext;
    m_ccProperties.clear();
}

void WebCLGLContextProperties::computeContextPropertiesInternal()
{
    ASSERT(m_contextPolicy != NoSharedGLContext);

    WebCLContextProperties::computeContextPropertiesInternal();

    if (m_contextPolicy == UseGLContextProvided)
        ComputeContext::populatePropertiesForInteroperabilityWithGL(m_ccProperties, m_webGLRenderingContext->graphicsContext3D()->platformGraphicsContext3D());
    else // if GetCurrentGLContext
        ComputeContext::populatePropertiesForInteroperabilityWithGL(m_ccProperties, 0);
}

}
#endif // ENABLE(WEBCL)
