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
#include "WebCLContextProperties.h"

#include "CachedImage.h"
#include "HTMLImageElement.h"
#include "HTMLVideoElement.h"
#include "WebCL.h"
#include "WebCLCommandQueue.h"
#include "WebCLEvent.h"
#include "WebCLImage.h"
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

// FIXME: Use Vector<CCDeviceID> and collapse 'numberDevices' and 'devices'.
PassRefPtr<WebCLContext> WebCLContext::create(WebCL* context, CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID *devices, CCerror& error)
{
    RefPtr<ComputeContext> computeContext = ComputeContext::create(contextProperties, numberDevices, devices, error);
    if (!computeContext) {
        ASSERT(error != ComputeContext::SUCCESS);
        return 0;
    }

    return adoptRef(new WebCLContext(context, computeContext));
}

WebCLContext::WebCLContext(WebCL* context, RefPtr<ComputeContext>& computeContext)
    : m_videoCache(4) // FIXME: Why '4'?
    , m_context(context)
{
    m_computeContext = computeContext;
    m_clContext = m_computeContext->context();

    // Cache all properties since they are immutable.
    ensureCachedContextProperties();
}

void WebCLContext::ensureCachedContextProperties()
{
    if (m_cachedContextProperties)
        return;

    CCerror err = 0;
    size_t dataSizeInBytes = 0;

    // 1) Devices.
    err = clGetContextInfo(m_clContext, ComputeContext::CONTEXT_DEVICES, 0, 0, &dataSizeInBytes);
    ASSERT(err == CL_SUCCESS);

    size_t numberOfDevices = dataSizeInBytes / sizeof(CCDeviceID);
    ASSERT(numberOfDevices);

    Vector<CCDeviceID, 5> ccDevices;
    err = clGetContextInfo(m_clContext, ComputeContext::CONTEXT_DEVICES, dataSizeInBytes, ccDevices.data(), 0);
    ASSERT(err == ComputeContext::SUCCESS);
    ccDevices.resize(numberOfDevices);

    Vector<RefPtr<WebCLDevice> > devices;
    toWebCLDeviceArray(ccDevices, devices);

    // 2) Platform.
    CCPlatformID ccPlatformID = 0;
    err = clGetContextInfo(m_clContext, ComputeContext::CONTEXT_PROPERTIES, 0, 0, &dataSizeInBytes);
    ASSERT(err == CL_SUCCESS);
    size_t numberOfProperties = dataSizeInBytes / sizeof(CCContextProperties);
    if (numberOfProperties) {
        CCContextProperties contextProperties[numberOfProperties];
        err = clGetContextInfo(m_clContext, ComputeContext::CONTEXT_PROPERTIES, dataSizeInBytes, &contextProperties, 0);
        ASSERT(err == CL_SUCCESS);
        for (size_t i = 0; i < numberOfProperties && contextProperties[i]; ++i) {
            if (contextProperties[i] == ComputeContext::CONTEXT_PLATFORM)
                ccPlatformID = (CCPlatformID) contextProperties[++i];
        }
    } else
        err = ComputeContext::getDeviceInfo(ccDevices[0], ComputeContext::DEVICE_PLATFORM, sizeof(ccPlatformID), &ccPlatformID);

    RefPtr<WebCLPlatform> platform = WebCLPlatform::create(ccPlatformID);

    // 3) Device type - bitfield'ed.
    CCulong deviceType = 0;
    for (size_t i = 0; i < numberOfDevices; ++i) {
        CCDeviceType type = 0;
        err = ComputeContext::getDeviceInfo(ccDevices[i], ComputeContext::DEVICE_TYPE, sizeof(CCDeviceType), &type);
        ASSERT(err == ComputeContext::SUCCESS);
        deviceType |= type;
    }

    // FIXME: 4/5) Needs to implement support for 'shareGroup' and 'hits'.
    m_cachedContextProperties = WebCLContextProperties::create(platform.release(), devices, deviceType, 0 /*sharedGroup*/, String() /*hints*/);
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
    if (!m_clContext) {
        printf("Error: Invalid CL Context\n");
        ec = WebCLException::INVALID_CONTEXT;
        return WebCLGetInfo();
    }

    switch (paramName) {
    case ComputeContext::CONTEXT_NUM_DEVICES:
        return WebCLGetInfo(m_cachedContextProperties->devices().size());
    case ComputeContext::CONTEXT_DEVICES:
        return WebCLGetInfo(m_cachedContextProperties->devices());
    case ComputeContext::CONTEXT_PROPERTIES:
        // FIXME: Is this cast needed?
        return WebCLGetInfo(PassRefPtr<WebCLContextProperties>(m_cachedContextProperties));
    default:
        ec = WebCLException::INVALID_VALUE;
    }

    return WebCLGetInfo();
}

PassRefPtr<WebCLCommandQueue> WebCLContext::createCommandQueue(WebCLDevice* device, int commandQueueProp, ExceptionCode& ec)
{
    UNUSED_PARAM(device);

    if (!m_clContext) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    CCDeviceID ccDevice;
    if (device)
        ccDevice = device->getCLDevice();
    else
        ccDevice = *m_context->getCLDeviceID();

    //  Creates a new command queue for the devices in the given array.
    // If devices is null, the WebCL implementation will select any single WebCLDevice that matches the given properties and is covered by this WebCLContext.
    // If properties is omitted, the command queue is created with out-of-order execution disabled and profiling disabled

    CCerror error = 0;
    CCCommandQueue clCommandqueueID = m_computeContext->createCommandQueue(ccDevice, commandQueueProp, error);
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

    RefPtr<WebCLBuffer> bufferObj = WebCLBuffer::create(this, clMemID, false);
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
        ec = ComputeContext::INVALID_VALUE;
        return 0;
    }

    CCerror error;
    clMemID = m_computeContext->createBuffer(memFlags, bufferSize, buffer, error);
    if (!clMemID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLBuffer> bufferObj = WebCLBuffer::create(this, clMemID, false);
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
        ec = ComputeContext::INVALID_VALUE;
        return 0;
    }

    CCerror error = 0;
    clMemID = m_computeContext->createBuffer(memFlags, bufferSize, image, error);
    if (!clMemID) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLBuffer> buferObj = WebCLBuffer::create(this, clMemID, false);
    return buferObj;
}


// FIXME: merge createImage2DBase
PassRefPtr<WebCLMemoryObject> WebCLContext::createImage2DBaseMemory(int flags, int width, int height, const ComputeContext::ImageFormat& imageFormat, void *data, ExceptionCode& ec)
{
    CCerror createImage2DError;
    cl_mem clMemImage = 0;
    clMemImage = m_computeContext->createImage2D(flags, width, height, imageFormat, data, createImage2DError);
    if (!clMemImage) {
        ASSERT(createImage2DError != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(createImage2DError);
        return 0;
    }

    RefPtr<WebCLMemoryObject> clMemObj = WebCLMemoryObject::create(this, clMemImage, false);
    return clMemObj;
}

PassRefPtr<WebCLImage> WebCLContext::createImage2DBaseImage(int flags, int width, int height, const ComputeContext::ImageFormat& imageFormat, void *data, ExceptionCode& ec)
{
    CCerror createImage2DError;
    cl_mem clMemImage = 0;

    if (!width || !height) {
        ec = WebCLException::INVALID_IMAGE_SIZE;
        return 0;
    }
    clMemImage = m_computeContext->createImage2D(flags, width, height, imageFormat, data, createImage2DError);
    if (!clMemImage) {
        ASSERT(createImage2DError != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(createImage2DError);
        return 0;
    }

    RefPtr<WebCLImage> imageObj = WebCLImage::create(this, clMemImage, false);
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
    if (ComputeContext::MEM_READ_ONLY != flags) {
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
    if (!m_clContext) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    ComputeContext::ImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    int width = 0;
    int height = 0;
    if (descriptor) {
        width =  descriptor->width();
        height = descriptor->height();
        imageFormat.channelOrder = descriptor->channelOrder();
        imageFormat.channelDataType = descriptor->channelType();
    } else {
        ec = WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR;
        return 0;
    }
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
    RefPtr<WebCLBuffer> bufferObj = WebCLBuffer::create(this, clMemID, true);
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
    RefPtr<WebCLImage> imageObj = WebCLImage::create(this, clMemID, true);
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

    RefPtr<WebCLSampler> samplerObj = WebCLSampler::create(this, clSamplerID);
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
    RefPtr<WebCLMemoryObject> memoryObject = WebCLMemoryObject::create(this, clMemID, true);
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
        RefPtr<WebCLEvent> resultEvent = WebCLEvent::create(this, event);
        return resultEvent;
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

Vector<RefPtr<WebCLImageDescriptor> > WebCLContext::getSupportedImageFormats(int memFlags, ExceptionCode &ec)
{
    Vector<RefPtr<WebCLImageDescriptor>> imageDescriptors;
    if (!m_clContext) {
        ec = WebCLException::INVALID_CONTEXT;
        return imageDescriptors;
    }

    CCuint numberOfSupportedImages;
    CCerror error;
    CCImageFormat* imageFormats = m_computeContext->supportedImageFormats(memFlags, ComputeContext::MEM_OBJECT_IMAGE2D, numberOfSupportedImages, error);

    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return imageDescriptors;
    }

    for (size_t i = 0; i < numberOfSupportedImages; ++i)
        imageDescriptors.append(WebCLImageDescriptor::create(imageFormats[i]));
    return imageDescriptors;
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
