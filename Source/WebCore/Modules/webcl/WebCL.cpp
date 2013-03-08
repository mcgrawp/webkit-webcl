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
    CCerror error;
    Vector<RefPtr<WebCLPlatform> > webCLPlatforms;
    Vector<CCPlatformID> ccPlatforms;

    error = ComputeContext::getPlatformIDs(ccPlatforms);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return webCLPlatforms;
    }

    for (size_t i = 0; i < ccPlatforms.size(); ++i)
        webCLPlatforms.append(WebCLPlatform::create(ccPlatforms[i]));

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

PassRefPtr<WebCLContext> WebCL::createContext(WebCLContextProperties* properties, ExceptionCode& ec)
{
    cl_context_properties contextProperties[5] = {0};
    int propIndex = 0;
    int deviceType = CL_DEVICE_TYPE_DEFAULT;

    if (properties && properties->platform()) {
        // append properties->platform to cl_context_properties variable.
        contextProperties[propIndex++] = CL_CONTEXT_PLATFORM;
        contextProperties[propIndex++] =
            (cl_context_properties)(properties->platform()->getCLPlatform());
    }

    if (properties && properties->shareGroup()) {
        // Set shareGroup in contextProperties. Currently only OpenGL
        // context can be shared.
        CGLContextObj kCGLContext = CGLGetCurrentContext();
        CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
        contextProperties[propIndex++] =
            CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE;
        contextProperties[propIndex++] = (cl_context_properties)kCGLShareGroup;
    }

    if (properties && properties->deviceType())
        deviceType = properties->deviceType();

    // FIXME: (Issue #25) WebCLContextProperties::objhint not used.

    Vector<CCDeviceID> ccDevices;
    if (properties && properties->devices().size()) {
        for (size_t i = 0; i < properties->devices().size(); ++i)
            ccDevices.append(properties->devices()[i]->getCLDevice());
    } else {
        Vector<CCPlatformID> ccPlatforms;
        CCerror error = ComputeContext::getPlatformIDs(ccPlatforms);
        if (error != ComputeContext::SUCCESS) {
            ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
            return 0;
        }

        error = ComputeContext::getDeviceIDs(ccPlatforms[0], ComputeContext::DEVICE_TYPE_DEFAULT, ccDevices);
        if (error != ComputeContext::SUCCESS) {
            ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
            return 0;
        }
    }

    CCerror error = ComputeContext::SUCCESS;
    RefPtr<WebCLContext> webCLContext = WebCLContext::create(this, propIndex ? contextProperties : 0, ccDevices, error);
    if (!webCLContext) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    m_context = webCLContext;
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
