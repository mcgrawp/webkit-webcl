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

#include "WebCLContext.h"
#include "WebCLContext.h"
#include "WebCLCommandQueue.h"
#include "WebCLProgram.h"
#include "WebCLKernel.h"
#include "WebCLMemoryObject.h"
#include "WebCLImage.h"
#include "WebCLEvent.h"
#include "WebCLSampler.h"
#include "WebCL.h"
#include "WebCLException.h"

namespace WebCore {

WebCLContext::~WebCLContext()
{
}

PassRefPtr<WebCLContext> WebCLContext::create(WebCL* computeContext, CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID *devices, CCerror* error)
{
    return adoptRef(new WebCLContext(computeContext, contextProperties, numberDevices, devices, error));
}

PassRefPtr<WebCLContext> WebCLContext::create(WebCL* computeContext, CCContextProperties* contextProperties, unsigned int deviceType, CCerror* error)
{
    return adoptRef(new WebCLContext(computeContext, contextProperties, deviceType, error));
}

WebCLContext::WebCLContext(WebCL* compute_context, CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID* devices, CCerror* error)
    : m_videoCache(4),
      m_context(compute_context)
{
    m_num_programs = 0;
    m_num_mems = 0;
    m_num_events = 0;
    m_num_samplers = 0;
    m_num_contexts = 0;

    m_computeContext = ComputeContext::create(contextProperties, numberDevices, devices, error);
    m_cl_context = m_computeContext->context();
}

WebCLContext::WebCLContext(WebCL* compute_context, CCContextProperties* contextProperties, unsigned int deviceType, CCerror* error)
    : m_videoCache(4),
      m_context(compute_context)
{
    m_num_programs = 0;
    m_num_mems = 0;
    m_num_events = 0;
    m_num_samplers = 0;
    m_num_contexts = 0;

    m_computeContext = ComputeContext::create(contextProperties, deviceType, error);
    m_cl_context = m_computeContext->context();
}

WebCLGetInfo WebCLContext::getInfo(int param_name, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_uint uint_units = 0;
    RefPtr<WebCLDeviceList> deviceList  = NULL;
    size_t szParmDataBytes = 0;
    size_t uint_array[WebCL::CHAR_BUFFER_SIZE] = {0};

    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return WebCLGetInfo();
    }

    switch(param_name) {
    case WebCL::CONTEXT_REFERENCE_COUNT:
        err = clGetContextInfo(m_cl_context, CL_CONTEXT_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned int>(uint_units));
        break;
    case WebCL::CONTEXT_DEVICES:
        cl_device_id* cdDevices;
        clGetContextInfo(m_cl_context, CL_CONTEXT_DEVICES, 0, NULL, &szParmDataBytes);
        if (err == CL_SUCCESS) {
            int nd = szParmDataBytes / sizeof(cl_device_id);
            cdDevices = (cl_device_id*) malloc(szParmDataBytes);
            clGetContextInfo(m_cl_context, CL_CONTEXT_DEVICES, szParmDataBytes, cdDevices, NULL);
            deviceList = WebCLDeviceList::create(cdDevices, nd);
            printf("Size Vs Size = %lu %d %d \n\n", szParmDataBytes,nd,deviceList->length());
            free(cdDevices);
            return WebCLGetInfo(PassRefPtr<WebCLDeviceList>(deviceList));
        }
        break;
    case WebCL::CONTEXT_PROPERTIES:
        err = clGetContextInfo(m_cl_context, CL_CONTEXT_PROPERTIES, 0, NULL, &szParmDataBytes);
        if (err == CL_SUCCESS) {
            printf(" szParmDataBytes  => %lu ", szParmDataBytes );
            int nd = szParmDataBytes / sizeof(cl_uint);
            if(nd == 0) {
                printf("No Context Properties defined \n");
                return WebCLGetInfo();
            }
            err = clGetContextInfo(m_cl_context, CL_CONTEXT_PROPERTIES, szParmDataBytes, &uint_array, &szParmDataBytes);
            if (err == CL_SUCCESS) {
                // Should int repacle cl_context_properties
                int values[WebCL::CHAR_BUFFER_SIZE] = {0};
                for(int i=0; i<((int)nd); i++) {
                    values[i] = (int)uint_array[i];
                    printf("%d\n", values[i]);
                }
                return WebCLGetInfo(Int32Array::create(values, nd));
            }
        }
        break;
    default:
        printf("Error: Unsupported Context Info type\n");
        ec = WebCLException::FAILURE;
        return WebCLGetInfo();
    }
    switch (err) {
        case CL_INVALID_CONTEXT:
            ec = WebCLException::INVALID_CONTEXT;
            printf("Error: CL_INVALID_CONTEXT \n");
            break;
        case CL_INVALID_VALUE:
            ec = WebCLException::INVALID_VALUE;
            printf("Error: CL_INVALID_VALUE\n");
            break;
        case CL_OUT_OF_RESOURCES:
            ec = WebCLException::OUT_OF_RESOURCES;
            printf("Error: CL_OUT_OF_RESOURCES \n");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            ec = WebCLException::OUT_OF_HOST_MEMORY;
            printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
            break;
        default:
            ec = WebCLException::FAILURE;
            printf("Error: Invaild Error Type\n");
            break;
    }
    return WebCLGetInfo();
}

PassRefPtr<WebCLCommandQueue> WebCLContext::createCommandQueue(WebCLDeviceList* devices, 
        int command_queue_prop, ExceptionCode& ec)
{
    cl_device_id* cl_device = NULL;
    cl_command_queue cl_command_queue_id = NULL;

    //cl_int err_dev = 0;
    //cl_uint m_num_platforms = 0;
    //cl_uint num_devices = 0;
    //cl_platform_id* m_cl_platforms = NULL;


    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return NULL;
    }
    if (devices != NULL) {
        cl_device = devices->getCLDevices();
        if (cl_device == NULL) {
            ec = WebCLException::INVALID_DEVICE;
            printf("Error: cl_device null\n");
            return NULL;
        }
    }
    else {

        printf(" devices is NULL \n ");
        cl_device = m_context->getcl_device_id();

        //	Creates a new command queue for the devices in the given array.
        //If devices is null, the WebCL implementation will select any single WebCLDevice that matches the given properties and is covered by this WebCLContext.
        //If properties is omitted, the command queue is created with out-of-order execution disabled and profiling disabled


    }

    CCerror error;
    cl_command_queue_id = m_computeContext->createCommandQueue(*cl_device, command_queue_prop, error);
    if (!cl_command_queue_id) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }
    RefPtr<WebCLCommandQueue> o = WebCLCommandQueue::create(this, cl_command_queue_id);
    m_command_queue = o;
    return o;
}


PassRefPtr<WebCLProgram> WebCLContext::createProgram(const String& kernelSource, ExceptionCode& ec)
{
    cl_program cl_program_id = NULL;
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return NULL;
    }

    CCerror error;
    cl_program_id = m_computeContext->createProgram(kernelSource, error);
    if (!cl_program_id) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }
    RefPtr<WebCLProgram> o = WebCLProgram::create(this, cl_program_id);
    o->setDevice(m_device_id);
    m_program_list.append(o);
    m_num_programs++;
    return o;
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(int flags, int size, ArrayBuffer* data, ExceptionCode& ec)
{
    cl_mem cl_mem_id = NULL;
    void *vData = NULL;
    if (m_cl_context == NULL) 
    {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return NULL;
    }

    //Saurabh
    if (data == NULL) {
        // Commenting the 2 lines below as they are failing with data not being sent. Spec says its optional so it must work fine with no data param.
        //ec = WebCLException::FAILURE;
        //return NULL;
        vData = NULL; //Saurabh Commented
    }
    else {
        vData = data->data();
    }

    CCerror error;
    cl_mem_id = m_computeContext->createBuffer(flags, size, vData, error);
    if (!cl_mem_id) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }

    RefPtr<WebCLBuffer> buffer_obj = WebCLBuffer::create(m_context, cl_mem_id, false);
    m_buffer_list.append(buffer_obj);
    m_num_buffer++;
    return buffer_obj;
}


PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(int memFlags, ImageData *ptr , ExceptionCode& ec)
{
    void* buffer;
    int buffer_size = 0;
    cl_mem cl_mem_id = NULL;

    if(ptr!=NULL && ptr->data()!=NULL && ptr->data()->data()!=NULL && ptr->data()->data()!=NULL )
    {
        buffer = ptr->data()->data();
        buffer_size =  ptr->data()->length();
    }
    else
    {
        printf("Error: Invalid ImageData\n");
        ec = WebCL::FAILURE;
        return NULL;
    }

    CCerror error;
    cl_mem_id = m_computeContext->createBuffer(memFlags, buffer_size, buffer, error);
    if (!cl_mem_id) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }

    RefPtr<WebCLBuffer> buffer_obj  = WebCLBuffer::create(m_context, cl_mem_id, false);
    m_buffer_list.append(buffer_obj);
    m_num_buffer++;
    return buffer_obj;
}


PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(int memFlags, HTMLCanvasElement *srcCanvas,ExceptionCode& ec)
{
    int buffer_size = 0;
    cl_mem cl_mem_id = NULL;
    int width  =0;
    int height =0;
    ImageBuffer* imageBuffer = NULL;
    RefPtr<Uint8ClampedArray> bytearray = NULL;
    void* image = NULL;

    if(srcCanvas != NULL) {
        width = srcCanvas->width();
        height = srcCanvas->height();
        printf(" WebCLContext::createBuffer(int memFlags, HTMLCanvasElement *srcCanvas,ExceptionCode& ec): width => %d  height => %d   \n ",width,height);
        imageBuffer = srcCanvas->buffer();
        if ( imageBuffer == NULL)
        {
            printf("image is null\n");
        }
        bytearray = imageBuffer->getUnmultipliedImageData(IntRect(0,0,width,height));
        if ( bytearray == NULL)
        {
            printf("bytearray is null\n");
        }
        if(bytearray->data() == NULL)
        {
            printf("bytearray->data() is null\n");
            ec = WebCLException::FAILURE;
            return NULL;
        }
        image = (void*) bytearray->data();
        buffer_size = bytearray->length();
        if(image == NULL)
        {
            printf("image is null\n");
            ec = WebCLException::FAILURE;
            return NULL;
        }
    }
    else
    {
        printf("Error: HTMLCanvasElement:srcCanvas is NULL \n ");
        ec = WebCL::FAILURE;
        return NULL;
    }

    CCerror error;
    cl_mem_id = m_computeContext->createBuffer(memFlags, buffer_size, image, error);
    if (!cl_mem_id) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }

    RefPtr<WebCLBuffer> bufer_obj  = WebCLBuffer::create(m_context, cl_mem_id, false);
    m_buffer_list.append(bufer_obj);
    m_num_buffer++;
    return bufer_obj;
}


// XXX: merge createImage2DBase
PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2DBaseMemory(int flags, int width, int height, const ComputeContext::ImageFormat& imageFormat, void *data, ExceptionCode& ec)
{
    CCerror createImage2DError;
    cl_mem cl_mem_image = NULL;
    cl_mem_image = m_computeContext->createImage2D(flags, width, height, imageFormat, data, createImage2DError);
    if (!cl_mem_image) {
        ec = createImage2DError;
        return NULL;
    }

    RefPtr<WebCLMemoryObject> o = WebCLMemoryObject::create(m_context, cl_mem_image,false);
    m_mem_list.append(o);
    m_num_mems++;
    return o;
}

PassRefPtr<WebCLImage> WebCLContext::createImage2DBaseImage(int flags, int width, int height, const ComputeContext::ImageFormat& imageFormat, void *data, ExceptionCode& ec)
{
    CCerror createImage2DError;
    cl_mem cl_mem_image = NULL;
    cl_mem_image = m_computeContext->createImage2D(flags, width, height, imageFormat, data, createImage2DError);
    if (!cl_mem_image) {
        ec = createImage2DError;
        return NULL;
    }

    RefPtr<WebCLImage> image_obj  = WebCLImage::create(m_context, cl_mem_image, false);
    m_img_list.append(image_obj);
    m_num_images++;
    return image_obj;
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags,HTMLCanvasElement* canvasElement, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return NULL;
    }
    ImageBuffer* imageBuffer = NULL;
    RefPtr<Uint8ClampedArray> bytearray = NULL;
    if (canvasElement != NULL) {
        width = canvasElement->width();
        height = canvasElement->height();

        imageBuffer = canvasElement->buffer();
        if ( imageBuffer == NULL)
            printf("image is null\n");
        bytearray = imageBuffer->getUnmultipliedImageData(IntRect(0,0,width,height));

        if ( bytearray == NULL)
            printf("bytearray is null\n");
    }

    if(bytearray->data() == NULL)
    {
        printf("bytearray->data() is null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    void* image = (void*) bytearray->data();
    if(image == NULL)
    {
        printf("image is null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, image, ec);
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags, HTMLImageElement* image, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;

    Image* imagebuf = NULL;
    CachedImage* cachedImage = NULL;
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return NULL;
    }
    if (image != NULL) {
        cachedImage = image->cachedImage();
        if (cachedImage == NULL) {
            ec = WebCLException::FAILURE;
            printf("Error: image null\n");
            return NULL;
        } else {
            width = image->width();
            height = image->height();
            imagebuf = cachedImage->image();
            if(imagebuf == NULL) {
                ec = WebCLException::FAILURE;
                printf("Error: imagebuf null\n");
                return NULL;
            }
        }
    }
    else {
        printf("Error: imageElement null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    void* image1  = (void*) cachedImage->image()->data()->data() ;
    if(image1 == NULL)
    {
        ec = WebCLException::FAILURE;
        printf("Error: image data is null\n");
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, image1, ec);
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags, HTMLVideoElement* video, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;

    RefPtr<Image> image = NULL;
    SharedBuffer* sharedBuffer = NULL;
    void* image_data = NULL;
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    if (video != NULL) {
        width =  video->width();
        height = video->height();
        image = videoFrameToImage(video);
        sharedBuffer = image->data();
        if (sharedBuffer == NULL) {
            printf("Error: sharedBuffer null\n");
            ec = WebCLException::FAILURE;
            return NULL;
        } else {
            image_data = (void*) sharedBuffer->data();

            if (image_data == NULL) {
                printf("Error: image_data null\n");
                ec = WebCLException::FAILURE;
                return NULL;
            }
        }
    } else {
        printf("Error: canvasElement null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, image_data, ec);
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags, ImageData* data, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;

    //CanvasPixelArray* pixelarray = NULL;
    Uint8ClampedArray* bytearray = NULL;
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    if (data != NULL) {
        //pixelarray = data->data();
        if(data->data() != NULL) {
            bytearray = data->data();
            if(bytearray == NULL)
                return NULL;
        }
        width = data->width();
        height = data->height();
    } else {
        printf("Error: canvasElement null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, bytearray->data(), ec);
}


// Create Image using ImageData , HTMLCanvasElement or HTMLImageElement as i/p. Can return WebCLBuffer or WebCLImage
PassRefPtr<WebCLImage> WebCLContext::createImage(int flags,HTMLCanvasElement* canvasElement, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return NULL;
    }
    RefPtr<Uint8ClampedArray> bytearray = NULL;
    if (canvasElement != NULL) {
        width = canvasElement->width();
        height = canvasElement->height();

        if ( canvasElement->buffer() == NULL)
        {
            printf("image is null\n");
        }
        bytearray = canvasElement->buffer()->getUnmultipliedImageData(IntRect(0,0,width,height));
        if ( bytearray == NULL)
        {
            printf("bytearray is null\n");
        }
        else
        {
            for(unsigned int  i =0 ;i<24 ; i++)
            {
                printf("  before clCreateImage2D(canvasElement) call pixelarray => %d => %u \n ",i,bytearray->item(i));
            }
        }
    }

    if(bytearray->data() == NULL)
    {
        printf("bytearray->data() is null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    void* image = (void*) bytearray->data();
    if(image == NULL)
    {
        printf("image is null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, image, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, HTMLImageElement* image, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;

    Image* imagebuf = NULL;
    CachedImage* cachedImage = NULL;
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return NULL;
    }
    if (image != NULL) {
        cachedImage = image->cachedImage();
        if (cachedImage == NULL) {
            ec = WebCLException::FAILURE;
            printf("Error: image null\n");
            return NULL;
        } else {
            width = image->width();
            height = image->height();
            imagebuf = cachedImage->image();
            if(imagebuf == NULL)
            {
                ec = WebCLException::FAILURE;
                printf("Error: imagebuf null\n");
                return NULL;
            }
        }
    } else {
        printf("Error: imageElement null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    OwnPtr<ImageBuffer> buffer;
    IntRect rect(0, 0, width, height);
    RefPtr<Uint8ClampedArray> byteArray;
    void *data;

    buffer = ImageBuffer::create(IntSize(width, height));
    buffer->context()->drawImage(cachedImage->image(), ColorSpaceDeviceRGB, rect);
    byteArray = buffer->getUnmultipliedImageData(rect);
    data = byteArray->data();

    if(data == NULL) {
        ec = WebCLException::FAILURE;
        printf("Error: image data is null\n");
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, data, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, HTMLVideoElement* video, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;

    RefPtr<Image> image = NULL;
    SharedBuffer* sharedBuffer = NULL;
    const char* image_data = NULL;
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    if (video != NULL) {
        width = video->width();
        height = video->height();
        image = videoFrameToImage(video);
        sharedBuffer = image->data();
        if (sharedBuffer == NULL) {
            printf("Error: sharedBuffer null\n");
            ec = WebCLException::FAILURE;
            return NULL;
        } else {
            image_data = sharedBuffer->data();
            if (image_data == NULL) {
                printf("Error: image_data null\n");
                ec = WebCLException::FAILURE;
                return NULL;
            }
        }
    } else {
        printf("Error: canvasElement null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, (void*) image_data, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, ImageData* data, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    Uint8ClampedArray* bytearray = NULL;

    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    if (data != NULL) {
        if(data->data() != NULL )
        {
            if(data->data()->data() != NULL)
            {
                bytearray = data->data();
            }
            else
            {
                printf("Error: canvasElement data->data()->data() is  null \n");
                ec = WebCLException::FAILURE;
                return NULL;
            }
        }
        else {
            printf("Error: canvasElement data->data() is  null \n");
            ec = WebCLException::FAILURE;
            return NULL;
        }

        width = data->width();
        height = data->height();
    }
    else {
        printf("Error: canvasElement null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, (void*) bytearray->data(), ec);
}


PassRefPtr<WebCLImage> WebCLContext::createImageWithDescriptor(int flags, WebCLImageDescriptor* descriptor, ArrayBuffer* data,ExceptionCode& ec )
{
    int width = 0;
    int height = 0;

    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    if (data != NULL) {
        width =  descriptor->width();
        height = descriptor->height();
    }
    else {
        printf("Error: canvasElement null\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, data->data(), ec);
}


PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags, unsigned int width,
                                                          unsigned int height, ArrayBuffer* data, ExceptionCode& ec)
{
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, data->data(), ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createFromGLBuffer(int flags, WebGLBuffer* bufobj, ExceptionCode& ec)
{
    PlatformComputeObject cl_mem_id = NULL;
    Platform3DObject buf_id = 0;

    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    if (bufobj != NULL) {
        buf_id = bufobj->object();
        if (buf_id == 0) {
            printf("Error: buf_id null\n");
            ec = WebCLException::FAILURE;
            return NULL;
        }
    }

    CCerror error;
    cl_mem_id = m_computeContext->createFromGLBuffer(flags, buf_id, error);
    if (!cl_mem_id) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }

    RefPtr<WebCLBuffer> buffer_obj = WebCLBuffer::create(m_context, cl_mem_id, true);
    m_buffer_list.append(buffer_obj);
    m_num_buffer++;
    return buffer_obj;
}


PassRefPtr<WebCLImage> WebCLContext::createFromGLRenderBuffer(int flags, WebGLRenderbuffer* renderbufferobj, ExceptionCode& ec)
{
    PlatformComputeObject cl_mem_id = NULL;
    GLuint rbuf_id = 0;

    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return NULL;
    }
    if (renderbufferobj != NULL) {
        rbuf_id =  renderbufferobj->getInternalFormat();

    }

    CCerror error;
    cl_mem_id = m_computeContext->createFromGLRenderbuffer(flags, rbuf_id, error);
    if (!cl_mem_id) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }
    RefPtr<WebCLImage> o = WebCLImage::create(m_context, cl_mem_id, true);
    m_img_list.append(o);
    m_num_images++;
    return o;
}

PassRefPtr<WebCLSampler> WebCLContext::createSampler(bool norm_cords, int addr_mode, int fltr_mode, ExceptionCode& ec)
{
    cl_sampler cl_sampler_id = NULL;

    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    CCerror error;
    cl_sampler_id = m_computeContext->createSampler(norm_cords, addr_mode, fltr_mode, error);
    if (!cl_sampler_id) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }

    RefPtr<WebCLSampler> o = WebCLSampler::create(m_context, cl_sampler_id);
    m_sampler_list.append(o);
    m_num_samplers++;
    return o;
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createFromGLTexture2D(int flags, GC3Denum texture_target, GC3Dint miplevel, GC3Duint texture, ExceptionCode& ec)
{
    cl_mem cl_mem_id = NULL;

    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return NULL;
    }

    CCerror error;
    cl_mem_id = m_computeContext->createFromGLTexture2D(flags, texture_target, miplevel, texture, error);
    if (!cl_mem_id) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }
    RefPtr<WebCLMemoryObject> o = WebCLMemoryObject::create(m_context, cl_mem_id, true);
    m_mem_list.append(o);
    m_num_mems++;
    return o;
}

PassRefPtr<WebCLEvent> WebCLContext::createUserEvent( ExceptionCode& ec)
{
    /*    
          cl_int err = -1;	
          cl_event event = NULL;
          if (m_cl_context == NULL) {
          printf("Error: Invalid CL Context\n");
          ec = WebCLException::FAILURE;
          return NULL;
          }


    //printf(" inside createUserEvent call before clCreateUserEvent ");
    //(TODO) To be uncommented for OpenCL1.1
    event =  clCreateUserEvent(m_cl_context, &err);
    //printf(" inside createUserEvent call after clCreateUserEvent ");
    if (err != CL_SUCCESS) {
    switch (err) {
    case CL_INVALID_CONTEXT :
    printf("Error: CL_INVALID_CONTEXT \n");
    ec = WebCLException::INVALID_CONTEXT;
    break;
    case CL_OUT_OF_RESOURCES :
    printf("Error: CCL_OUT_OF_RESOURCES \n");
    ec = WebCLException::OUT_OF_RESOURCES;
    break;
    case CL_OUT_OF_HOST_MEMORY :
    printf("Error: CCL_OUT_OF_HOST_MEMORY \n");
    ec = WebCLException::OUT_OF_HOST_MEMORY;
    break;
    default:
    printf("Error: Invaild Error Type\n");
    ec = WebCLException::FAILURE;
    break;
    }

    } else {
    RefPtr<WebCLEvent> o = WebCLEvent::create(m_context, event);
    m_event_list.append(o);
    m_num_events++;
    return o;
    }
    */
    UNUSED_PARAM(ec);
    return NULL;
}

void WebCLContext::setDevice(RefPtr<WebCLDevice> device_id)
{
    m_device_id = device_id;
}
cl_context WebCLContext::getCLContext()
{
    return m_cl_context;
}
PassRefPtr<Image> WebCLContext::videoFrameToImage(HTMLVideoElement* video)
{
    if (!video || !video->videoWidth() || !video->videoHeight()) {
        return 0;
    }
    IntSize size(video->videoWidth(), video->videoHeight());
    ImageBuffer* buf = m_videoCache.imageBuffer(size);
    if (!buf) {
        return 0;
    }
    IntRect destRect(0, 0, size.width(), size.height());
    // FIXME: Turn this into a GPU-GPU texture copy instead of CPU readback.
    video->paintCurrentFrameInContext(buf->context(), destRect);
    return buf->copyImage();
}

    WebCLContext::LRUImageBufferCache::LRUImageBufferCache(int capacity)
    : m_buffers(adoptArrayPtr(new OwnPtr<ImageBuffer>[capacity]))
      , m_capacity(capacity)
{
}

ImageBuffer* WebCLContext::LRUImageBufferCache::imageBuffer(const IntSize& size)
{
    int i;
    for (i = 0; i < m_capacity; ++i) {
        ImageBuffer* buf = m_buffers[i].get();
        if (!buf)
            break;
        // TODO siba check this call
        //if (buf->size() != size)
        //	continue;
        bubbleToFront(i);
        return buf;
    }

    OwnPtr<ImageBuffer> temp = ImageBuffer::create(size);
    if (!temp)
        return 0;
    i = std::min(m_capacity - 1, i);
    m_buffers[i] = temp.release();

    ImageBuffer* buf = m_buffers[i].get();
    bubbleToFront(i);
    return buf;
}

void WebCLContext::LRUImageBufferCache::bubbleToFront(int idx)
{
    for (int i = idx; i > 0; --i)
        m_buffers[i].swap(m_buffers[i-1]);
}

PassRefPtr<WebCLImageDescriptorList> WebCLContext::getSupportedImageFormats(int memFlags,int imageType ,ExceptionCode &ec)
{
    //  Get context
    if (m_cl_context == NULL) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return NULL;
    }

    cl_uint uint_num_entries = 0;
    CCerror error = m_computeContext->supportedImageFormats(memFlags, imageType, 0, &uint_num_entries, NULL);

    if (uint_num_entries == 0) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return NULL;
    }
    // Max Number of Formats expected TODO check how to standardise this

    // FIXME: we should use WebKit memory allocator
    // Array of size uint_num_entries to hold formats from opencl API
    CCImageFormat *imageFormat =  new CCImageFormat[uint_num_entries];

    // reference to take the num of supported formats
    // FIXME: Error code is not checked.
    error = m_computeContext->supportedImageFormats(memFlags, imageType, uint_num_entries, &uint_num_entries, imageFormat);

    RefPtr<WebCLImageDescriptorList> result = WebCLImageDescriptorList::create(imageFormat, uint_num_entries);
    return result;
}


} // namespace WebCore

#endif // ENABLE(WEBCL)
