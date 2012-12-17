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
#include "ScriptExecutionContext.h"
#include "Document.h"
#include "DOMWindow.h"
#include "Image.h"
#include "SharedBuffer.h"
#include "CanvasRenderingContext2D.h"
#include "ImageBuffer.h"
#include "CachedImage.h"
#include <wtf/ArrayBuffer.h>
#include "HTMLCanvasElement.h"
#include "WebCLException.h"
#include <stdio.h>
#include <wtf/Uint8ClampedArray.h>
#include <CanvasRenderingContext.h>

class CanvasRenderingContext;

using namespace JSC;

namespace WebCore {

WebCL::WebCL(ScriptExecutionContext* context) : ActiveDOMObject(context, this)
{
    m_num_mems = 0;
    m_num_programs = 0;
    m_num_events = 0;
    m_num_samplers = 0;
    m_num_contexts = 0;
    m_num_commandqueues = 0;
}

PassRefPtr<WebCL> WebCL::create(ScriptExecutionContext* context)
{
    return adoptRef(new WebCL(context));
}

WebCL::~WebCL()
{
}

PassRefPtr<WebCLPlatformList> WebCL::getPlatforms(ExceptionCode& ec)
{
    RefPtr<WebCLPlatformList> cl_platform_list_ptr = WebCLPlatformList::create();
    if (cl_platform_list_ptr != NULL) {
        m_platform_id = cl_platform_list_ptr;
        return cl_platform_list_ptr;
    } else {
        ec = WebCLException::INVALID_PLATFORM;
        return NULL;
    }
}

WebCLGetInfo WebCL::getImageInfo(WebCLImage* image, cl_image_info param_name, ExceptionCode& ec)
{
    cl_mem cl_Image_id = NULL;
    cl_int err = 0;
    size_t sizet_units = 0;
    if (image != NULL) {
        cl_Image_id = image->getCLImage();
        if (cl_Image_id == NULL) {
            printf("Error: cl_Image_id null\n");
            ec = WebCLException::INVALID_MEM_OBJECT;
            return WebCLGetInfo();
        }
    }
    else
    {
        //TODO Throwing error need to confirm behaviour
        ec = WebCLException::INVALID_MEM_OBJECT;
        return WebCLGetInfo();
    }
    switch(param_name)
    {   
    case IMAGE_ELEMENT_SIZE:
            err=clGetImageInfo(cl_Image_id, CL_IMAGE_ELEMENT_SIZE, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case IMAGE_ROW_PITCH:
            err=clGetImageInfo(cl_Image_id, CL_IMAGE_ROW_PITCH, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case IMAGE_SLICE_PITCH:
            err=clGetImageInfo(cl_Image_id, CL_IMAGE_SLICE_PITCH, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case IMAGE_WIDTH:
            err=clGetImageInfo(cl_Image_id, CL_IMAGE_WIDTH, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case IMAGE_HEIGHT:
            err=clGetImageInfo(cl_Image_id, CL_IMAGE_HEIGHT, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case IMAGE_DEPTH:
            err=clGetImageInfo(cl_Image_id, CL_IMAGE_DEPTH, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
            // TODO (siba samal) Handle Image Format & CL_IMAGE_D3D10_SUBRESOURCE_KHR types
        default:
            printf("Error: Unsupported Image Info type\n");
            return WebCLGetInfo();
    }
    switch (err) {
        case CL_INVALID_MEM_OBJECT:
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error: CL_INVALID_MEM_OBJECT \n");
            break;
        case CL_INVALID_VALUE:
            ec = WebCLException::INVALID_VALUE;
            printf("Error: CL_INVALID_VALUE \n");
            break;
        case CL_OUT_OF_RESOURCES:
            ec = WebCLException::OUT_OF_RESOURCES;
            printf("Error: CL_OUT_OF_RESOURCES\n");
            break; 
        case CL_OUT_OF_HOST_MEMORY:
            ec = WebCLException::OUT_OF_HOST_MEMORY;
            printf("Error: CL_OUT_OF_HOST_MEMORY\n");
            break;
        default:
            printf("Error: Invaild Error Type\n");
            ec = WebCLException::FAILURE;
            break;
    }				
    return WebCLGetInfo();

}

void WebCL::waitForEvents(WebCLEventList* events, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_event* cl_event_id = NULL;

    if (events != NULL) {
        cl_event_id = events->getCLEvents();
        if (cl_event_id == NULL) {
            printf("Error: cl_event null\n");
            ec = WebCLException::INVALID_EVENT;
            return;
        }
    }
    err = clWaitForEvents(events->length(), cl_event_id);
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_CONTEXT:
                printf("Error: CL_INVALID_CONTEXT \n");
                ec = WebCLException::INVALID_CONTEXT;
                break;
            case CL_INVALID_VALUE:
                printf("Error: CL_INVALID_VALUE \n");
                ec = WebCLException::INVALID_VALUE;
                break;
            case CL_INVALID_EVENT :
                printf("Error: CL_INVALID_EVENT  \n");
                ec = WebCLException::INVALID_VALUE;
                break;
                //OpenCL 1.1
                //case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
                //	printf("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST \n");
                //	break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES \n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY \n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;

            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;

        }
    }
}

PassRefPtr<WebCLContext> WebCL::createContext(WebCLContextProperties* properties, ExceptionCode& ec)
{
     cl_device_id* cl_device = NULL;

    cl_platform_id* m_cl_platforms = NULL;
    cl_device_id* m_cl_devices = NULL;

    int numofDevice = 1;

    cl_context_properties *contextProperties;


    if (properties != NULL && properties->devices() != NULL) {

        numofDevice = properties->devices()->length();
        //printf(" numofDevice = %d \n ", numofDevice);
        cl_device = (properties->devices())->getCLDevices();//devices->getCLDevices();

        this->setcl_device_id(cl_device);

        if (cl_device == NULL) {
            ec = WebCLException::INVALID_DEVICE;
            printf("Error: devices null\n");
            this->setcl_device_id(NULL);
            return NULL;
        }

        if(properties != NULL && properties->platform() != NULL)
        {
            //printf(" properties is not NULL and  properties->platform() is also not null \n ");
            contextProperties = new cl_context_properties[1];
            contextProperties[0] = (cl_context_properties)(properties->platform()->getCLPlatform());
        }
        else
        {
            //printf(" properties is NULL or   properties->platform() is :null \n ");
            //contextProperties = new cl_context_properties[1];
            //contextProperties[0] = CL_CONTEXT_PLATFORM;
            contextProperties = NULL;

        }

    }
    else
    {
        cl_int m_num_platforms = 0;
        m_num_platforms = ComputeContext::platformIDs(0, NULL);

        CCint numberPlatformId;
        if(m_num_platforms) {
            m_cl_platforms = new cl_platform_id[m_num_platforms];
            numberPlatformId = ComputeContext::platformIDs(m_num_platforms, m_cl_platforms);
        }
        cl_int numDevices;
        if(numberPlatformId) {
            numDevices = ComputeContext::deviceIDs(m_cl_platforms[0], CL_DEVICE_TYPE_DEFAULT, 1, NULL);
        }
        if(numDevices) {
            m_cl_devices = new cl_device_id[numDevices];
            numDevices = ComputeContext::deviceIDs(m_cl_platforms[0], CL_DEVICE_TYPE_DEFAULT, 1, m_cl_devices);
            cl_device =  m_cl_devices;
            this->setcl_device_id(cl_device);
            numofDevice = numDevices;
            contextProperties = NULL;
        }
        else {
            ec = WebCLException::INVALID_DEVICE;
            printf("Error: Device Type Not Supported \n");
            return NULL;
        }
    }

    // TODO(won.jeon) - prop, pfn_notify, and user_data need to be addressed later
    int error;
    RefPtr<WebCLContext> o = WebCLContext::create(this, contextProperties, numofDevice, cl_device, &error);
    if (o != NULL) {
        //printf(" WebCLContext object o is not NULL \n ");
        m_context = o;
        return o;
    } else
        ec = error;

    return NULL;
}

void WebCL::setcl_device_id(cl_device_id*  cd)
{
    this->my_cl_device_id = cd;
}

cl_device_id* WebCL::getcl_device_id()
{
    return this->my_cl_device_id;
}

PassRefPtr<WebCLContext> WebCL::createContextFromType(int contextProperties, int device_type, int pfn_notify, int user_data, ExceptionCode& ec)
{
    UNUSED_PARAM(pfn_notify);
    UNUSED_PARAM(user_data);

    //TODO (siba samal) Need to handle context properties
    if((CONTEXT_PLATFORM != contextProperties) &&  (0 != contextProperties))
    {
        ec = WebCLException::INVALID_PROPERTY;
        printf("Error: INVALID CONTEXT PROPERTIES\n");
        return NULL;
    }

    int error;
    RefPtr<WebCLContext> o = WebCLContext::create(this, NULL, device_type, &error);
    if (o != NULL) {
        m_context = o;
        return o;
    } else
        ec = error;

    return NULL;
}

PassRefPtr<WebCLContext> WebCL::createSharedContext(int device_type, int pfn_notify, int user_data, ExceptionCode& ec)
{
    UNUSED_PARAM(device_type);
    UNUSED_PARAM(pfn_notify);
    UNUSED_PARAM(user_data);

    CGLContextObj kCGLContext = CGLGetCurrentContext();
    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);

    cl_context_properties properties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)kCGLShareGroup, 0
    };
    // TODO (siba samal) Handle NULL parameters
    int error;
    RefPtr<WebCLContext> o = WebCLContext::create(this, properties, 0, NULL, &error);
    if (o != NULL) {
        m_context = o;
        return o;
    } else
        ec = error;

    return NULL;
}

/*
* Desc:: Turns an array of extension names that are supported by all WebCLPlatforms and WebCLDevices in this system. 
*        Any string in this list, when passed to getExtension on any platform or device, must return a valid extension object.
*/
Vector<String> WebCL::getSupportedExtensions(ExceptionCode&)
{
    // FIXME: Needs a proper implementation.
    return Vector<String>();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
