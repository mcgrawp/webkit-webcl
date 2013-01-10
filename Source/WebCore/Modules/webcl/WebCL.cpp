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
#include "WebCLEventList.h"
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

PassRefPtr<WebCLPlatformList> WebCL::getPlatforms(ExceptionCode& ec)
{
    CCerror error;
    RefPtr<WebCLPlatformList> platformListPtr = WebCLPlatformList::create(error);
    if (!platformListPtr) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::INVALID_PLATFORM;
        return 0;
    }

    return platformListPtr;
}

WebCLGetInfo WebCL::getImageInfo(WebCLImage* image, cl_image_info paramName, ExceptionCode& ec)
{
    cl_mem clImageID = 0;
    cl_int err = 0;
    size_t units = 0;
    if (image) {
        clImageID = image->getCLImage();
        if (!clImageID) {
            printf("Error: cl_Image_id null in getImageInfo\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return WebCLGetInfo();
        }
    }

    switch (paramName) {
    case IMAGE_ELEMENT_SIZE:
        err = clGetImageInfo(clImageID, CL_IMAGE_ELEMENT_SIZE, sizeof(size_t), &units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<int>(units));
        break;
    case IMAGE_WIDTH:
        err = clGetImageInfo(clImageID, CL_IMAGE_WIDTH, sizeof(size_t), &units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<int>(units));
        break;
    case IMAGE_HEIGHT:
        err = clGetImageInfo(clImageID, CL_IMAGE_HEIGHT, sizeof(size_t), &units, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<int>(units));
        break;
    default:
        printf("Error: Unsupported Image Info type in getImageInfo\n");
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

void WebCL::waitForEvents(WebCLEventList* events, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_event* clEventID = 0;

    if (events) {
        clEventID = events->getCLEvents();
        if (!clEventID) {
            printf("Error: cl_event null in waitForEvents\n");
            ec = WebCLException::INVALID_EVENT;
            return;
        }
    }
    err = clWaitForEvents(events->length(), clEventID);
    if (err != CL_SUCCESS) {
        switch (err) {
        case CL_INVALID_CONTEXT:
            printf("Error: CL_INVALID_CONTEXT in waitForEvents\n");
            ec = WebCLException::INVALID_CONTEXT;
            break;
        case CL_INVALID_VALUE:
            printf("Error: CL_INVALID_VALUE in waitForEvents \n");
            ec = WebCLException::INVALID_VALUE;
            break;
        case CL_INVALID_EVENT :
            printf("Error: CL_INVALID_EVENT in waitForEvents \n");
            ec = WebCLException::INVALID_VALUE;
            break;
        case CL_OUT_OF_RESOURCES:
            printf("Error: CL_OUT_OF_RESOURCES in waitForEvents\n");
            ec = WebCLException::OUT_OF_RESOURCES;
            break;
        case CL_OUT_OF_HOST_MEMORY:
            printf("Error: CL_OUT_OF_HOST_MEMORY in waitForEvents \n");
            ec = WebCLException::OUT_OF_HOST_MEMORY;
            break;
        default:
            printf("Error: Invaild Error Type in waitForEvents\n");
            ec = WebCLException::FAILURE;
            break;
        }
    }
    return;
}

PassRefPtr<WebCLContext> WebCL::createContext(WebCLContextProperties* properties, ExceptionCode& ec)
{
    cl_device_id* clDevice = 0;
    cl_platform_id* clPlatforms = 0;
    cl_device_id* clDevices = 0;
    int numofDevice = 1;
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

    // FIXME ISSUE #25 ::WebCLContextProperties::objhint  not used.

    if (properties && properties->devices()) {
        numofDevice = properties->devices()->length();
        clDevice = (properties->devices())->getCLDevices();
        if (!clDevice) {
            ec = WebCLException::INVALID_DEVICE;
            printf("Error: no Device pased in WebCLContextProperties \n");
            this->setCLDeviceID(0);
            return 0;
        }
    } else {
        CCerror platformIdError;
        CCint numPlatforms = ComputeContext::platformIDs(0, 0, platformIdError);
        CCint numberPlatformId;
        if (numPlatforms) {
            clPlatforms = new cl_platform_id[numPlatforms];
            numberPlatformId = ComputeContext::platformIDs(numPlatforms, clPlatforms, platformIdError);
        }
        cl_int numDevices = 0;
        CCerror deviceIdError;
        if (platformIdError == ComputeContext::SUCCESS)
            numDevices = ComputeContext::deviceIDs(clPlatforms[0], CL_DEVICE_TYPE_DEFAULT, 1, 0, deviceIdError);
        if (deviceIdError == ComputeContext::SUCCESS) {
            clDevices = new cl_device_id[numDevices];
            numDevices = ComputeContext::deviceIDs(clPlatforms[0], CL_DEVICE_TYPE_DEFAULT, 1, clDevices, deviceIdError);
            clDevice =  clDevices;
            this->setCLDeviceID(clDevice);
            numofDevice = numDevices;
        } else {
            ec = WebCLException::INVALID_DEVICE;
            printf("Error: Device Type Not Supported \n");
            return 0;
        }
    }

    CCerror error;
    RefPtr<WebCLContext> webCLContext = WebCLContext::create(this, propIndex ? contextProperties : 0, numofDevice, clDevice, error);
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
