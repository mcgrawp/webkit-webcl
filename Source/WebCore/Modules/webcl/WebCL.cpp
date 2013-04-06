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
#include "WebCL.h"

#include "ComputeContext.h"
#include "WebCLContextProperties.h"
#include "WebCLException.h"
#include <wtf/PassRefPtr.h>

using namespace JSC;

namespace WebCore {

WebCL::WebCL()
{
}

PassRefPtr<WebCL> WebCL::create()
{
    return adoptRef(new WebCL());
}

WebCL::~WebCL()
{
}

Vector<RefPtr<WebCLPlatform> > WebCL::getPlatforms(ExceptionCode& ec) const
{
    Vector<RefPtr<WebCLPlatform> > webCLPlatforms;
    CCerror error = WebCore::getPlatforms(webCLPlatforms);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return webCLPlatforms;
    }

    return webCLPlatforms;
}

void WebCL::waitForEvents(const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
{

    Vector<CCEvent> ccEvents;

    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->getCLEvent());

    CCerror error = ComputeContext::waitForEvents(ccEvents);
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

    m_defaultProperties = WebCLContextProperties::create(webCLPlatforms[0], webCLDevices, ComputeContext::DEVICE_TYPE_DEFAULT, 1 /*shareGroup*/, emptyString());
    return m_defaultProperties;
}

PassRefPtr<WebCLContext> WebCL::createContext(PassRefPtr<WebCLContextProperties> properties, ExceptionCode& ec)
{
    RefPtr<WebCLContextProperties> refProperties = properties;
    if (refProperties) {
        if (!refProperties->platform()) {
            Vector<RefPtr<WebCLPlatform> > webCLPlatforms = getPlatforms(ec);
            if (ec != WebCLException::SUCCESS)
                return 0;

            refProperties->setPlatform(webCLPlatforms[0]);
        }

        if (!refProperties->devices().size()) {
            Vector<RefPtr<WebCLDevice> > webCLDevices = refProperties->platform()->getDevices(ComputeContext::DEVICE_TYPE_DEFAULT, ec);
            if (ec != WebCLException::SUCCESS)
                return 0;

            refProperties->devices().append(webCLDevices[0]);
        }
    } else {
        refProperties = defaultProperties(ec);
        if (ec != WebCLException::SUCCESS)
            return 0;
    }

    CCerror error = ComputeContext::SUCCESS;
    RefPtr<WebCLContext> webCLContext = WebCLContext::create(this, refProperties.release(), error);
    if (!webCLContext) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    m_context.append(webCLContext);
    return webCLContext;
}

/*
* Desc:: Turns an array of extension names that are supported by all WebCLPlatforms and WebCLDevices in this system. 
*        Any string in this list, when passed to getExtension on any platform or device, must return a valid extension.
*/
Vector<String> WebCL::getSupportedExtensions(ExceptionCode&)
{
    // FIXME: Needs a proper implementation.
    return Vector<String>();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
