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
    Vector<RefPtr<WebCLPlatform> > webCLPlatformList;
    CCint numberOfPlatforms = ComputeContext::platformIDs(0, 0, error);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return webCLPlatformList;
    }

    Vector<CCPlatformID> platformIDs(numberOfPlatforms);
    ComputeContext::platformIDs(numberOfPlatforms, platformIDs.data(), error);
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return webCLPlatformList;
    }

    for (int i = 0; i < numberOfPlatforms; ++i)
        webCLPlatformList.append(WebCLPlatform::create(platformIDs[i]));

    return webCLPlatformList;
}

WebCLGetInfo WebCL::getImageInfo(WebCLImage* image, cl_image_info paramName, ExceptionCode& ec)
{
    cl_mem clImageID = 0;
    if (image) {
        clImageID = image->getCLImage();
        if (!clImageID) {
            printf("Error: cl_Image_id null in getImageInfo\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return WebCLGetInfo();
        }
    }

    cl_int err = 0;
    switch (paramName) {
    case ComputeContext::IMAGE_ELEMENT_SIZE:
    case ComputeContext::IMAGE_WIDTH:
    case ComputeContext::IMAGE_HEIGHT: {
        size_t units = 0;
        err = clGetImageInfo(clImageID, paramName, sizeof(size_t), &units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<int>(units));
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    switch (err) {
    case CL_INVALID_MEM_OBJECT:
        ec = WebCLException::INVALID_MEM_OBJECT;
        printf("Error: CL_INVALID_MEM_OBJECT in getImageInfo \n");
        break;
    case CL_INVALID_VALUE:
        ec = WebCLException::INVALID_VALUE;
        printf("Error: CL_INVALID_VALUE in getImageInfo\n");
        break;
    case CL_OUT_OF_RESOURCES:
        ec = WebCLException::OUT_OF_RESOURCES;
        printf("Error: CL_OUT_OF_RESOURCES in getImageInfo\n");
        break;
    case CL_OUT_OF_HOST_MEMORY:
        ec = WebCLException::OUT_OF_HOST_MEMORY;
        printf("Error: CL_OUT_OF_HOST_MEMORY in getImageInfo \n");
        break;
    default:
        printf("Error: Invaild Error Type in getImageInfo\n");
        ec = WebCLException::FAILURE;
        break;
    }
    return WebCLGetInfo();
}

void WebCL::waitForEvents(const Vector<WebCLEvent*>& events, ExceptionCode& ec)
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

    // FIXME: Once we get rid of WebCLEventList (Issue #73), we will
    // be able to get rid of 'clDevices'.
    int numberOfDevices = 0;
    CCerror error = 0;
    // FIXME: Hardcoding '5' here, as it is enough number of devices in real world.
    Vector<CCDeviceID, 5> ccDevices;
    if (properties && properties->devices().size()) {
        numberOfDevices = properties->devices().size();
        for (int i = 0; i < numberOfDevices; ++i)
            ccDevices.append(properties->devices()[i]->getCLDevice());
    } else {
        CCint numberOfPlatforms = ComputeContext::platformIDs(0, 0, error);
        if (!numberOfPlatforms) {
            ec = WebCLException::INVALID_PLATFORM;
            return 0;
        }

        if (error != ComputeContext::SUCCESS) {
            ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
            return 0;
        }

        Vector<CCPlatformID> platforms;
        if (!platforms.tryReserveCapacity(numberOfPlatforms)) {
            ec = WebCLException::OUT_OF_HOST_MEMORY;
            return 0;
        }
        platforms.resize(numberOfPlatforms);

        // Return value can be ignored this time.
        ComputeContext::platformIDs(numberOfPlatforms, platforms.data(), error);
        if (error != ComputeContext::SUCCESS) {
            ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
            return 0;
        }

        numberOfDevices = ComputeContext::deviceIDs(platforms[0], ComputeContext::DEVICE_TYPE_DEFAULT, 1, 0, error);
        if (error != ComputeContext::SUCCESS) {
            ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
            return 0;
        }

        // Hardcoding '5' here, as it is enough number of devices in real world.
        ASSERT(numberOfDevices <= 5);

        // Return value can be ignored this time.
        ComputeContext::deviceIDs(platforms[0], ComputeContext::DEVICE_TYPE_DEFAULT, 1, ccDevices.data(), error);
        if (error != ComputeContext::SUCCESS) {
            ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
            return 0;
        }
    }

    this->setCLDeviceID(ccDevices.data());

    RefPtr<WebCLContext> webCLContext = WebCLContext::create(this, propIndex ? contextProperties : 0, numberOfDevices, ccDevices.data(), error);
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

void WebCL::setCLDeviceID(cl_device_id* deviceID)
{
    this->m_cldeviceid = deviceID;
}

cl_device_id* WebCL::getCLDeviceID()
{
    return this->m_cldeviceid;
}
} // namespace WebCore

#endif // ENABLE(WEBCL)
