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

#ifndef WebCL_h
#define WebCL_h

#if ENABLE(WEBCL)

#include "WebCLContextProperties.h"
#include "WebCLException.h"
#include "WebCLExtension.h"
#include "WebCLInputChecker.h"
#include "WebCLPlatform.h"

namespace WebCore {

class WebCLContextProperties;
class WebCLContext;
class WebCLEvent;

class WebCL : public RefCounted<WebCL> , public WebCLExtensionsAccessor<> {
public:
    static PassRefPtr<WebCL> create();
    virtual ~WebCL() { }

    Vector<RefPtr<WebCLPlatform> > getPlatforms(ExceptionCode&);

    void waitForEvents(const Vector<RefPtr<WebCLEvent> >&, ExceptionCode&);

    PassRefPtr<WebCLContext> createContext(PassRefPtr<WebCLContextProperties>, ExceptionCode&);

protected:
    WebCL();

    template <class T>
    void inline createContextBase(PassRefPtr<WebCLContextProperties> properties, ExceptionCode& ec, RefPtr<T>& out);

private:
    RefPtr<WebCLContextProperties>& defaultProperties(ExceptionCode&);
    RefPtr<WebCLContextProperties> m_defaultProperties;
    Vector<RefPtr<WebCLPlatform> > m_platforms;
};

template <class T>
inline void WebCL::createContextBase(PassRefPtr<WebCLContextProperties> properties, ExceptionCode& ec, RefPtr<T>& outValue)
{
    outValue = 0;

    RefPtr<WebCLContextProperties> refProperties = properties;
    if (refProperties) {
        // If properties has devices, neglect deviceType and set platform based on devices.
        if (refProperties->devices().size()) {
            if (refProperties->platform()) {
                // Ensure devices are from same platform as sent in properties.
                RefPtr<WebCLPlatform> devicePlatform = refProperties->platform();
                for (size_t i = 0; i < refProperties->devices().size(); i++)
                    if (refProperties->devices()[0]->platform() != devicePlatform) {
                        ec = WebCLException::INVALID_DEVICE;
                        return;
                    }
            } else {
                // Check if all devices are belonging to same platform.
                // if yes set that platform object as propreties.platform.
                const WebCLPlatform* devicePlatform = refProperties->devices()[0]->platform();
                for (size_t i = 1; i < refProperties->devices().size(); i++)
                    if (refProperties->devices()[i]->platform() != devicePlatform) {
                        ec = WebCLException::INVALID_DEVICE;
                        return;
                    }
                refProperties->setPlatform(const_cast<WebCLPlatform*>(devicePlatform));
            }
        } else {
            // No devices sent by user, check if deviceType was sent. Else use default.
            unsigned long deviceType = ComputeContext::DEVICE_TYPE_DEFAULT;
            if (refProperties->deviceType()) {
                deviceType = refProperties->deviceType();
                if (!WebCLInputChecker::isValidDeviceType(deviceType)) {
                    ec = WebCLException::INVALID_VALUE;
                    return;
                }
            }
            // Get devices of "deviceType".
            if (refProperties->platform()) {
                Vector<RefPtr<WebCLDevice> > webCLDevices = refProperties->platform()->getDevices(deviceType, ec);
                if (ec != WebCLException::SUCCESS)
                    return;
                refProperties->setDevices(webCLDevices);
            } else {
                // No Platform nor device sent in Properties.
                RefPtr<WebCLPlatform> defaultPlatform = getPlatforms(ec)[0];
                Vector<RefPtr<WebCLDevice> > webCLDevices = defaultPlatform->getDevices(deviceType, ec);
                if (ec != WebCLException::SUCCESS)
                    return;
                refProperties->setPlatform(defaultPlatform);
                refProperties->setDevices(webCLDevices);
            }
        }
    } else {
        refProperties = defaultProperties(ec);
        if (ec != WebCLException::SUCCESS)
            return;
    }

    CCerror error = ComputeContext::SUCCESS;
    outValue = T::create(refProperties.release(), error);
    if (!outValue) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
    }
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCL_h
