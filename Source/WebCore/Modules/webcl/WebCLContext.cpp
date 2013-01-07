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

#include "WebCLContext.h"

#include "CachedImage.h"
#include "HTMLImageElement.h"
#include "HTMLVideoElement.h"
#include "WebCL.h"
#include "WebCLCommandQueue.h"
#include "WebCLEvent.h"
#include "WebCLImage.h"
#include "WebCLImageDescriptorList.h"
#include "WebCLKernel.h"
#include "WebCLMemoryObject.h"
#include "WebCLProgram.h"
#include "WebCLSampler.h"
#include "WebGLBuffer.h"
#include "WebGLRenderbuffer.h"

namespace WebCore {

WebCLContext::~WebCLContext()
{
}

PassRefPtr<WebCLContext> WebCLContext::create(WebCL* computeContext, CCContextProperties* contextProperties,
    CCuint numberDevices, CCDeviceID *devices, CCerror& error)
{
    return adoptRef(new WebCLContext(computeContext, contextProperties, numberDevices, devices, error));
}

WebCLContext::WebCLContext(WebCL* compute_context, CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID* devices, CCerror& error)
    : m_videoCache(4) // FIXME: Why '4'?
    , m_context(compute_context)
{
    m_computeContext = ComputeContext::create(contextProperties, numberDevices, devices, error);
    m_clContext = m_computeContext->context();
}

/*WebCLContext::WebCLContext(WebCL* compute_context, CCContextProperties* contextProperties, unsigned deviceType, int* error)
    : m_videoCache(4)
    , m_context(compute_context)
{
    m_computeContext = ComputeContext::create(contextProperties, deviceType, error);
    m_clContext = m_computeContext->context();
}*/

WebCLGetInfo WebCLContext::getInfo(int paramName, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_uint uintUnits = 0;
    RefPtr<WebCLDeviceList> deviceList = 0;
    size_t szParmDataBytes = 0;
    size_t uintArray[WebCL::CHAR_BUFFER_SIZE] = {0};
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return WebCLGetInfo();
    }
    switch (paramName) {
    case WebCL::CONTEXT_REFERENCE_COUNT:
        err = clGetContextInfo(m_clContext, CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &uintUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uintUnits));
        break;
    case WebCL::CONTEXT_NUM_DEVICES:
        err = clGetContextInfo(m_clContext, CL_CONTEXT_NUM_DEVICES, sizeof(cl_uint), &uintUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uintUnits));
        break;
    case WebCL::CONTEXT_DEVICES:
        {
            cl_device_id* cdDevices = 0;
            err = clGetContextInfo(m_clContext, CL_CONTEXT_DEVICES, 0, 0, &szParmDataBytes);
            if (err == CL_SUCCESS) {
                int nd = szParmDataBytes / sizeof(cl_device_id);
                cdDevices = (cl_device_id*) malloc(szParmDataBytes);
                if (!cdDevices) {
                    printf("Error: Memory allocation failed in getInfo\n");
                    return WebCLGetInfo();
                }
                clGetContextInfo(m_clContext, CL_CONTEXT_DEVICES, szParmDataBytes, cdDevices, 0);
                deviceList = WebCLDeviceList::create(cdDevices, nd);
                free(cdDevices);
                return WebCLGetInfo(PassRefPtr<WebCLDeviceList>(deviceList));
            }
        }
        break;
    case WebCL::CONTEXT_PROPERTIES:
        {
            err = clGetContextInfo(m_clContext, CL_CONTEXT_PROPERTIES, 0, 0, &szParmDataBytes);
            if (err == CL_SUCCESS) {
                int nd = szParmDataBytes / sizeof(cl_uint);
                if (!nd)
                    return WebCLGetInfo();
                err = clGetContextInfo(m_clContext, CL_CONTEXT_PROPERTIES, szParmDataBytes, &uintArray, &szParmDataBytes);
                if (err == CL_SUCCESS) {
                    int values[WebCL::CHAR_BUFFER_SIZE] = {0};
                    for (int i = 0; i < ((int)nd); i++)
                        values[i] = (int)uintArray[i];
                    return WebCLGetInfo(Int32Array::create(values, nd));
                }
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

PassRefPtr<WebCLCommandQueue> WebCLContext::createCommandQueue(WebCLDeviceList* devices, int commandQueueProp, ExceptionCode& ec)
{
    cl_device_id* clDevice = 0;
    cl_command_queue clCommandqueueID = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    if (devices) {
        clDevice = devices->getCLDevices();
        if (!clDevice) {
            ec = WebCLException::INVALID_DEVICE;
            printf("Error: clDevice null\n");
            return 0;
        }
    } else
        clDevice = m_context->getCLDeviceID();

    // FIXME :: Need to validate commandQueueProp.

    //  Creates a new command queue for the devices in the given array.
    // If devices is null, the WebCL implementation will select any single WebCLDevice that matches the given properties and is covered by this WebCLContext.
    // If properties is omitted, the command queue is created with out-of-order execution disabled and profiling disabled

    CCerror error = 0;
    clCommandqueueID = m_computeContext->createCommandQueue(*clDevice, commandQueueProp, error);
    if (!clCommandqueueID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLCommandQueue> commandqueueObj = WebCLCommandQueue::create(this, clCommandqueueID);
    m_commandQueue = commandqueueObj;
    return commandqueueObj;
}

PassRefPtr<WebCLProgram> WebCLContext::createProgram(const String& kernelSource, ExceptionCode& ec)
{
    cl_program clProgramID = 0;
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    const char* source = strdup(kernelSource.utf8().data());
    if (!source) {
        printf("Error: strdup returned error [%d]", errno);
        return 0;
    }

    CCerror error = 0;
    clProgramID = m_computeContext->createProgram(kernelSource, error);
    if (!clProgramID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLProgram> clProgramObj = WebCLProgram::create(this, clProgramID);
    return clProgramObj;
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(int flags, int size, ArrayBuffer* data, ExceptionCode& ec)
{
    cl_mem clMemID = 0;
    void* vData = 0;
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    if (!flags) {
        printf("Error:: CL_INVALID_VALUE \n");
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }
    if (data)
        vData = data->data();

    CCerror error;
    clMemID = m_computeContext->createBuffer(flags, size, vData, error);
    if (!clMemID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLBuffer> bufferObj = WebCLBuffer::create(m_context, clMemID, false);
    return bufferObj;
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(int memFlags, ImageData *ptr, ExceptionCode& ec)
{
    unsigned char* buffer = 0;
    int bufferSize = 0;
    cl_mem clMemID = 0;

    if (ptr && ptr->data() && ptr->data()->data()) {
        buffer = ptr->data()->data();
        bufferSize = ptr->data()->length();
    } else {
        printf("Error: Invalid ImageData object in createBuffer.\n");
        ec = WebCL::FAILURE;
        return 0;
    }

    CCerror error;
    clMemID = m_computeContext->createBuffer(memFlags, bufferSize, buffer, error);
    if (!clMemID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLBuffer> bufferObj = WebCLBuffer::create(m_context, clMemID, false);
    return bufferObj;
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(int memFlags, HTMLCanvasElement* srcCanvas, ExceptionCode& ec)
{
    int bufferSize = 0;
    cl_mem clMemID = 0;
    cl_uint width = 0;
    cl_uint height = 0;
    ImageBuffer* imageBuffer = 0;
    RefPtr<Uint8ClampedArray> bytearray = 0;
    void* image = 0;

    if (srcCanvas) {
        width = (cl_uint) srcCanvas->width();
        height = (cl_uint) srcCanvas->height();
        imageBuffer = srcCanvas->buffer();
        if (imageBuffer) {
            bytearray = imageBuffer->getUnmultipliedImageData(IntRect(0, 0, width, height));
            if (bytearray) {
                image = (void*) bytearray->data();
                bufferSize = bytearray->length();
            }
        }
    }
    if (!image) {
        printf("Error: Invalid HTMLCanvasElement obj passed in createBuffer.\n ");
        ec = WebCL::FAILURE;
        return 0;
    }

    CCerror error = 0;
    clMemID = m_computeContext->createBuffer(memFlags, bufferSize, image, error);
    if (!clMemID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLBuffer> buferObj = WebCLBuffer::create(m_context, clMemID, false);
    return buferObj;
}


// FIXME: merge createImage2DBase
PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2DBaseMemory(int flags, int width, int height, const ComputeContext::ImageFormat& imageFormat, void *data, ExceptionCode& ec)
{
    CCerror createImage2DError;
    cl_mem clMemImage = 0;
    clMemImage = m_computeContext->createImage2D(flags, width, height, imageFormat, data, createImage2DError);
    if (!clMemImage) {
        ec = createImage2DError;
        return 0;
    }

    RefPtr<WebCLMemoryObject> clMemObj = WebCLMemoryObject::create(m_context, clMemImage, false);
    return clMemObj;
}

PassRefPtr<WebCLImage> WebCLContext::createImage2DBaseImage(int flags, int width, int height, const ComputeContext::ImageFormat& imageFormat, void *data, ExceptionCode& ec)
{
    CCerror createImage2DError;
    cl_mem clMemImage = 0;
    clMemImage = m_computeContext->createImage2D(flags, width, height, imageFormat, data, createImage2DError);
    if (!clMemImage) {
        ec = createImage2DError;
        return 0;
    }

    RefPtr<WebCLImage> imageObj = WebCLImage::create(m_context, clMemImage, false);
    return imageObj;
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, HTMLCanvasElement* canvasElement, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    RefPtr<Uint8ClampedArray> bytearray = 0;
    ImageBuffer* imageBuffer = 0;
    void* image = 0;
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    if (canvasElement) {
        width = (cl_uint) canvasElement->width();
        height = (cl_uint) canvasElement->height();
        imageBuffer = canvasElement->buffer();
        if (imageBuffer) {
            bytearray = imageBuffer->getUnmultipliedImageData(IntRect(0, 0, width, height));
            if (bytearray)
                image = (void*) bytearray->data();
        }
    }
    if (!image) {
        printf("Error:: createImage2D canvasElement is invalid.\n");
        ec = WebCLException::FAILURE;
        return 0;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, image, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, HTMLImageElement* image, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    CachedImage* cachedImage = 0;
    void* data = 0;
    RefPtr<Uint8ClampedArray> byteArray;
    OwnPtr<ImageBuffer> buffer;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    if (image) {
        width = (cl_uint) image->width();
        height = (cl_uint) image->height();
        cachedImage = image->cachedImage();
        if (cachedImage) {
            IntRect rect(0, 0, width, height);
            buffer = ImageBuffer::create(IntSize(width, height));
            buffer->context()->drawImage(cachedImage->image(), ColorSpaceDeviceRGB, rect);
            byteArray = buffer->getUnmultipliedImageData(rect);
            data = byteArray->data();
        }
    }
    if (!data) {
        ec = WebCLException::FAILURE;
        printf("Error: Invalid HTMLImageElement object in createImage\n");
        return 0;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, data, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, HTMLVideoElement* video, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    RefPtr<Image> image = 0;
    SharedBuffer* sharedBuffer = 0;
    void* imageData = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    if (WebCL::MEM_READ_ONLY != flags) {
        printf("ERROR:: createImage with HTMLVideoElement can use only MEM_READ_ONLY as flags\n");
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }
    if (video) {
        width =  video->width();
        height = video->height();
        image = videoFrameToImage(video);
        if (image) {
            sharedBuffer = image->data();
            if (sharedBuffer)
                imageData = (void*) sharedBuffer->data();
        }
    }
    if (!imageData) {
        printf("Error: videoFrameToImage failed to convert in createImage.\n");
        ec = WebCLException::FAILURE;
        return 0;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, (void*) imageData, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, ImageData* data, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    Uint8ClampedArray* bytearray = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    if (data && data->data() && data->data()->data()) {
        bytearray = data->data();
        width = (cl_uint) data->width();
        height = (cl_uint) data->height();
    } else {
        printf("Error: canvasElement is  null \n");
        ec = WebCLException::FAILURE;
        return 0;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, (void*) bytearray->data(), ec);
}

// FIXME ISSUE #35 Need to rename this to createImage(int flags, WebCLImageDescriptor* descriptor...)
PassRefPtr<WebCLImage> WebCLContext::createImageWithDescriptor(int flags, WebCLImageDescriptor* descriptor, ArrayBuffer* data, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    if (descriptor) {
        width =  descriptor->width();
        height = descriptor->height();
    } else {
        printf("Error: descriptor is null\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseImage(flags, width, height, imageFormat, data->data(), ec);
}


PassRefPtr<WebCLBuffer> WebCLContext::createFromGLBuffer(int flags, WebGLBuffer* bufobj, ExceptionCode& ec)
{
    PlatformComputeObject clMemID = 0;
    Platform3DObject bufferID = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    if (bufobj)
        bufferID = bufobj->object();
    if (!bufferID) {
        printf("Error: WebGLBuffer object is invalid.\n");
        ec = WebCLException::FAILURE;
        return 0;
    }

    CCerror error;
    clMemID = m_computeContext->createFromGLBuffer(flags, bufferID, error);
    if (!clMemID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLBuffer> bufferObj = WebCLBuffer::create(m_context, clMemID, true);
    return bufferObj;
}


PassRefPtr<WebCLImage> WebCLContext::createFromGLRenderBuffer(int flags, WebGLRenderbuffer* renderbufferobj, ExceptionCode& ec)
{
    PlatformComputeObject clMemID = 0;
    GLuint rbufID = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    if (!renderbufferobj) {
        rbufID =  renderbufferobj->getInternalFormat();

    }

    CCerror error;
    clMemID = m_computeContext->createFromGLRenderbuffer(flags, rbufID, error);
    if (!clMemID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLImage> imageObj = WebCLImage::create(m_context, clMemID, true);
    return imageObj;
}

PassRefPtr<WebCLSampler> WebCLContext::createSampler(bool normCords, int addrMode, int fltrMode, ExceptionCode& ec)
{
    cl_sampler clSamplerID = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }

    CCerror error;
    clSamplerID = m_computeContext->createSampler(normCords, addrMode, fltrMode, error);
    if (!clSamplerID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLSampler> samplerObj = WebCLSampler::create(m_context, clSamplerID);
    return samplerObj;
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createFromGLTexture2D(int flags, GC3Denum textureTarget, GC3Dint miplevel, GC3Duint texture, ExceptionCode& ec)
{
    cl_mem clMemID = 0;
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }

    CCerror error;
    clMemID = m_computeContext->createFromGLTexture2D(flags, textureTarget, miplevel, texture, error);
    if (!clMemID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLMemoryObject> memoryObject = WebCLMemoryObject::create(m_context, clMemID, true);
    return memoryObject;
}

PassRefPtr<WebCLEvent> WebCLContext::createUserEvent(ExceptionCode& ec)
{
    cl_int err = -1;
    cl_event event = 0;
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }

    event =  clCreateUserEvent(m_clContext, &err);
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
        // FIXME:: Need to solve once WebCLEvent is merged.
        // RefPtr<WebCLEvent> result_event = WebCLEvent::create(m_context, event);
        // return result_event;
        return 0;
    }
    return 0;
}

PassRefPtr<Image> WebCLContext::videoFrameToImage(HTMLVideoElement* video)
{
    if (!video || !video->videoWidth() || !video->videoHeight())
        return 0;
    IntSize size(video->videoWidth(), video->videoHeight());
    ImageBuffer* buf = m_videoCache.imageBuffer(size);
    if (!buf)
        return 0;
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
    for (int i = 0; i < m_capacity; ++i) {
        ImageBuffer* buf = m_buffers[i].get();
        if (!buf)
            break;
        bubbleToFront(i);
        return buf;
    }
    OwnPtr<ImageBuffer> temp = ImageBuffer::create(size);
    if (!temp)
        return 0;
    int i = std::min(m_capacity - 1, i);
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

PassRefPtr<WebCLImageDescriptorList> WebCLContext::getSupportedImageFormats(int memFlags, int imageType, ExceptionCode &ec)
{
    //  Get context
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    // Validate the flags passed.
    switch (memFlags) {
    case CL_MEM_READ_WRITE:
    case CL_MEM_WRITE_ONLY:
    case CL_MEM_READ_ONLY:
    case CL_MEM_USE_HOST_PTR:
    case CL_MEM_ALLOC_HOST_PTR:
    case CL_MEM_COPY_HOST_PTR:
    case CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR:
    case CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR:
    case CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR:
    case CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR:
    case CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR:
    case CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR:
        // Valid flags . Continue with the cl API call.
        break;
    case 0:
        // 0 is treated as CL_MEM_READ_WRITE flag.
        memFlags = CL_MEM_READ_WRITE;
        break;
    default:
        printf("Error: INVALID_VALUE specified as flag \n");
        ec = WebCLException::INVALID_VALUE;
        break;
    }

    // Validate the imageType passed.
    switch (imageType) {
    case CL_MEM_OBJECT_IMAGE2D :
    case CL_MEM_OBJECT_IMAGE3D :
        // Valid Image Types supported by OpenCL. Continue ...
        break;
    default:
        printf("Error: INVALID_VALUE specified as imageType\n");
        ec = WebCLException::INVALID_VALUE;
        break;
    }

    // Max Number of Formats expected FIXME check how to standardise this
    cl_uint uintNumEntries = 0;
    CCerror error = m_computeContext->supportedImageFormats(memFlags, imageType, 0, &uintNumEntries, 0);

    // FIXME: Should the error code be checked here instead?
    if (!uintNumEntries) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    // Max Number of Formats expected FIXME check how to standardise this

    // FIXME: we should use WebKit memory allocator
    // Array of size uint_num_entries to hold formats from opencl API
    CCImageFormat* imageFormat =  new CCImageFormat[uintNumEntries];

    // reference to take the num of supported formats
    // FIXME: Error code is not checked.
    error = m_computeContext->supportedImageFormats(memFlags, imageType, uintNumEntries, &uintNumEntries, imageFormat);

    RefPtr<WebCLImageDescriptorList> result = WebCLImageDescriptorList::create(imageFormat, uintNumEntries);
    return result;
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags, unsigned width, unsigned height, ArrayBuffer* data, ExceptionCode& ec)
{
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, data->data(), ec);
}


PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags, HTMLCanvasElement* canvasElement, ExceptionCode& ec)
{
    cl_uint width = 0;
    cl_uint height = 0;
    ImageBuffer* imageBuffer = 0;
    RefPtr<Uint8ClampedArray> bytearray = 0;
    void* image = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (canvasElement) {
        width = (cl_uint) canvasElement->width();
        height = (cl_uint) canvasElement->height();
        imageBuffer = canvasElement->buffer();
        if (imageBuffer) {
            bytearray = imageBuffer->getUnmultipliedImageData(IntRect(0, 0, width, height));
            if (bytearray)
                image = (void*) bytearray->data();
        }
    }
    if (!image) {
        printf("Error:: createImage2D canvasElement is invalid.\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, image, ec);
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags, HTMLImageElement* image, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    CachedImage* cachedImage = 0;
    void* imageObj = 0;
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    if (image) {
        width = (cl_uint) image->width();
        height = (cl_uint) image->height();
        cachedImage = image->cachedImage();
        if (cachedImage)
            imageObj = (void*)cachedImage->image()->data()->data();
    }
    if (!imageObj) {
        ec = WebCLException::FAILURE;
        printf("Error: Invalid image object in createImage2D\n");
        return 0;
    }
    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, imageObj, ec);
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags, HTMLVideoElement* video, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    RefPtr<Image> image = 0;
    SharedBuffer* sharedBuffer = 0;
    void* imageData = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    if (video) {
        width =  video->width();
        height = video->height();
        image = videoFrameToImage(video);
        if (image) {
            sharedBuffer = image->data();
            if (sharedBuffer)
                imageData = (void*) sharedBuffer->data();
        }
    }
    if (!imageData) {
        printf("Error: videoFrameToImage failed to convert.\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, imageData, ec);
}

PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2D(int flags, ImageData* data, ExceptionCode& ec)
{
    int width = 0;
    int height = 0;
    Uint8ClampedArray* bytearray = 0;

    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::FAILURE;
        return 0;
    }
    if (data) {
        if (data->data()) {
            bytearray = data->data();
            if (!bytearray)
                return 0;
        }
        width = (cl_uint) data->width();
        height = (cl_uint) data->height();
    } else {
        printf("Error: canvasElement null\n");
        ec = WebCLException::FAILURE;
        return 0;
    }

    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBaseMemory(flags, width, height, imageFormat, bytearray->data(), ec);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
