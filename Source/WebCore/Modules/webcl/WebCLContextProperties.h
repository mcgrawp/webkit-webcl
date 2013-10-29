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

#ifndef WebCLContextProperties_h
#define WebCLContextProperties_h

#if ENABLE(WEBCL)

#include "ComputeTypes.h"
#include "WebCLPlatform.h"
#include "WebGLRenderingContext.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class WebCLDevice;
class WebCLPlatform;

class WebCLContextProperties : public RefCounted<WebCLContextProperties>
{
public:
    enum SharedContextResolutionPolicy {
        NoSharedGLContext = 0,
        GetCurrentGLContext,
        UseGLContextProvided
    };

    static PassRefPtr<WebCLContextProperties> create();
    static PassRefPtr<WebCLContextProperties> create(PassRefPtr<WebCLPlatform>, const Vector<RefPtr<WebCLDevice> >&, CCenum deviceType);

    virtual ~WebCLContextProperties() { }

    Vector<RefPtr<WebCLDevice> >& devices();
    void setDevices(Vector<RefPtr<WebCLDevice> >);

    RefPtr<WebCLPlatform>& platform();
    void setPlatform(PassRefPtr<WebCLPlatform>);

    CCenum deviceType() const;
    void setDeviceType(CCenum type);

    virtual RefPtr<WebGLRenderingContext>& sharedContext();
    virtual void setSharedContext(PassRefPtr<WebGLRenderingContext>);

    virtual Vector<CCContextProperties>& computeContextProperties();

    virtual bool isGLCapable() const { return false; }

protected:
    WebCLContextProperties();
    WebCLContextProperties(PassRefPtr<WebCLPlatform>, const Vector<RefPtr<WebCLDevice> >&, CCenum deviceType);

    virtual void computeContextPropertiesInternal();

    RefPtr<WebCLPlatform> m_platform;
    Vector<RefPtr<WebCLDevice> > m_devices;
    CCenum m_deviceType;
    Vector<CCContextProperties> m_ccProperties;

    enum SharedContextResolutionPolicy m_contextPolicy;
    RefPtr<WebGLRenderingContext> m_webGLRenderingContext;
};

}// namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCLContextProperties_h
