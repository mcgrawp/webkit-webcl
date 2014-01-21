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

#include "WebCL.h"

#include "ComputeEvent.h"
#include "WebCLContext.h"
#include "WebCLDevice.h"
#include "WebCLEvent.h"

using namespace JSC;

namespace WebCore {

PassRefPtr<WebCL> WebCL::create()
{
    return adoptRef(new WebCL);
}

WebCL::WebCL()
    : WebCLExtensionsAccessor(0)
{
}

Vector<RefPtr<WebCLPlatform> > WebCL::getPlatforms(ExceptionCode& ec)
{
    if (m_platforms.size())
        return m_platforms;

    CCerror error = WebCore::getPlatforms(m_platforms);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        m_platforms.clear();
        return m_platforms;
    }
    return m_platforms;
}

void WebCL::waitForEvents(const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
{
    Vector<ComputeEvent*> computeEvents;

    for (size_t i = 0; i < events.size(); ++i) {
        // FIXME: We currently do not support the asynchronous variant of this method.
        // So it the event being waited on has not been initialized (1) or is an user
        // event (2), we would hand the browser.
        // 1 - http://www.khronos.org/bugzilla/show_bug.cgi?id=1092
        // 2 - http://www.khronos.org/bugzilla/show_bug.cgi?id=1086
        if (events[i]->isPlatformObjectNeutralized()
            || !events[i]->holdsValidCLObject()
            || events[i]->isUserEvent()) {
            ec = WebCLException::INVALID_EVENT;
            return;
        }

        computeEvents.append(events[i]->platformObject());
    }

    CCerror error = ComputeContext::waitForEvents(computeEvents);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
}

RefPtr<WebCLContextProperties>& WebCL::defaultProperties(ExceptionCode& ec)
{
    if (m_defaultProperties)
        return m_defaultProperties;

    Vector<RefPtr<WebCLPlatform> > webCLPlatforms = getPlatforms(ec);
    if (ec != WebCLException::SUCCESS)
        return m_defaultProperties;

    Vector<RefPtr<WebCLDevice> > webCLDevices = webCLPlatforms[0]->getDevices(ComputeContext::DEVICE_TYPE_DEFAULT, ec);
    if (ec != WebCLException::SUCCESS)
        return m_defaultProperties;

    m_defaultProperties = WebCLContextProperties::create(webCLPlatforms[0], webCLDevices, ComputeContext::DEVICE_TYPE_DEFAULT);
    return m_defaultProperties;
}

PassRefPtr<WebCLContext> WebCL::createContext(PassRefPtr<WebCLContextProperties> properties, ExceptionCode& ec)
{
    RefPtr<WebCLContextProperties> refProperties = properties;
    if (refProperties) {
        // If properties has devices, neglect deviceType and set platform based on devices.
        // FIXME: Handle "If the array is not null, it must not be empty".
        if (refProperties->devices().size()) {
            // FIXME: Spec says "This field (platform) is ignored if devices is non-null."
            if (refProperties->platform()) {
                // Ensure devices are from same platform as sent in properties.
                RefPtr<WebCLPlatform> devicePlatform = refProperties->platform();
                for (size_t i = 0; i < refProperties->devices().size(); i++) {
                    if (refProperties->devices()[0]->platform() != devicePlatform) {
                        ec = WebCLException::INVALID_DEVICE;
                        return 0;
                    }
                }
            } else {
                // Check if all devices are belonging to same platform.
                // if yes set that platform object as propreties.platform.
                const WebCLPlatform* devicePlatform = refProperties->devices()[0]->platform();
                for (size_t i = 1; i < refProperties->devices().size(); i++) {
                    if (refProperties->devices()[i]->platform() != devicePlatform) {
                        ec = WebCLException::INVALID_DEVICE;
                        return 0;
                    }
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
                    return 0;
                }
            }
            // Get devices of "deviceType".
            if (refProperties->platform()) {
                Vector<RefPtr<WebCLDevice> > webCLDevices = refProperties->platform()->getDevices(deviceType, ec);
                if (ec != WebCLException::SUCCESS)
                    return 0;
                refProperties->setDevices(webCLDevices);
            } else {
                // No Platform nor device sent in Properties.
                RefPtr<WebCLPlatform> defaultPlatform = getPlatforms(ec)[0];
                Vector<RefPtr<WebCLDevice> > webCLDevices = defaultPlatform->getDevices(deviceType, ec);
                if (ec != WebCLException::SUCCESS)
                    return 0;
                refProperties->setPlatform(defaultPlatform);
                refProperties->setDevices(webCLDevices);
            }
        }
    } else {
        refProperties = defaultProperties(ec);
        if (ec != WebCLException::SUCCESS)
            return 0;
    }

    RefPtr<WebCLContext> context = WebCLContext::create(this, refProperties.release(), ec);
    if (!context) {
        ASSERT(ec != WebCLException::SUCCESS);
        return 0;
    }
    return context.release();
}

void WebCL::trackReleaseableWebCLObject(WeakPtr<WebCLObject> object)
{
    m_descendantWebCLObjects.append(object);
}

void WebCL::releaseAll()
{
    for (size_t i = 0; i < m_descendantWebCLObjects.size(); ++i) {
        WebCLObject* object = m_descendantWebCLObjects.at(i).get();
        if (!object)
            continue;

        WebCLContext* context = static_cast<WebCLContext*>(object);
        context->releaseAll();
    }
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
