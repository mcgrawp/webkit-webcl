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

#include "ComputeDevice.h"
#include "ComputePlatform.h"
#include "HTMLImageElement.h"
#include "HTMLVideoElement.h"
#include "ImageData.h"
#include "SharedBuffer.h"
#include "WebCL.h"
#include "WebCLBuffer.h"
#include "WebCLCommandQueue.h"
#include "WebCLEvent.h"
#include "WebCLHTMLInterop.h"
#include "WebCLImage.h"
#include "WebCLImageDescriptor.h"
#include "WebCLMemoryInitializer.h"
#include "WebCLProgram.h"
#include "WebCLSampler.h"
#include "WebCLUserEvent.h"
#if ENABLE(WEBGL)
#include "WebGLBuffer.h"
#include "WebGLRenderbuffer.h"
#include "WebGLRenderingContext.h"
#include "WebGLTexture.h"
#endif

namespace WebCore {

WebCLContext::~WebCLContext()
{
    releasePlatformObject();
}

static inline void getEnabledExtensions(WebCL* webCL, WebCLPlatform* platform, const Vector<RefPtr<WebCLDevice> >& devices, HashSet<String>& enabledExtensions)
{
    webCL->getEnabledExtensions(enabledExtensions);
    platform->getEnabledExtensions(enabledExtensions);

    for (size_t i = 0; i < devices.size(); i++)
        devices[i]->getEnabledExtensions(enabledExtensions);
}

PassRefPtr<WebCLContext> WebCLContext::create(WebCL* webCL, WebGLRenderingContext* glContext, WebCLPlatform* platform, const Vector<RefPtr<WebCLDevice> >& devices, ExceptionCode& ec)
{
    // Check all the enabled extensions and cache it to avoid enabling after context creation.
    HashSet<String> enabledExtensions;
    getEnabledExtensions(webCL, platform, devices, enabledExtensions);
    // If WebGLRenderingContext is sent and KHR_gl_sharing is not Enabled, throw WEBCL_EXTENSION_NOT_ENABLED.
    if (glContext && !enabledExtensions.contains("KHR_gl_sharing")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }

    Vector<ComputeDevice*> ccDevices;
    for (size_t i = 0; i < devices.size(); ++i) {
        RefPtr<WebCLDevice> device = devices[i];
        if (!device) {
            ec = WebCLException::INVALID_DEVICE;
            return 0;
        }

        if (device->platform()->platformObject() != platform->platformObject()) {
            ec = WebCLException::INVALID_DEVICE;
            return 0;
        }

        ccDevices.append(devices[i]->platformObject());
    }

    CCerror error = ComputeContext::SUCCESS;
    ComputePlatform* computePlatform = platform ? platform->platformObject() : 0;
    GraphicsContext3D* graphicsContext3D = glContext ? glContext->graphicsContext3D() : 0;

    ComputeContext* computeContext = new ComputeContext(ccDevices, computePlatform, graphicsContext3D, error);
    if (error != ComputeContext::SUCCESS) {
        delete computeContext;
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLContext> context = adoptRef(new WebCLContext(webCL, computeContext, devices, enabledExtensions));
    return context.release();
}

WebCLContext::WebCLContext(WebCL* webCL, ComputeContext* computeContext, const Vector<RefPtr<WebCLDevice> >& devices, HashSet<String>& enabledExtensions)
    : WebCLObjectImpl(computeContext)
    , m_videoCache(4) // FIXME: Why '4'?
    , m_webCL(webCL)
    , m_devices(devices)
    , m_memoryInitializer(this)
    , m_enabledExtensions(enabledExtensions)
{
    webCL->trackReleaseableWebCLObject(createWeakPtr());
}

WebCLGetInfo WebCLContext::getInfo(CCenum paramName, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return WebCLGetInfo();
    }

    switch (paramName) {
    case ComputeContext::CONTEXT_NUM_DEVICES:
        return WebCLGetInfo(static_cast<CCuint>(m_devices.size()));
    case ComputeContext::CONTEXT_DEVICES:
        return WebCLGetInfo(m_devices);
    default:
        ec = WebCLException::INVALID_VALUE;
    }

    return WebCLGetInfo();
}

bool WebCLContext::isExtensionEnabled(const String& name) const
{
    return m_enabledExtensions.contains(name);
}

PassRefPtr<WebCLCommandQueue> WebCLContext::createCommandQueue(WebCLDevice* device, CCenum properties, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!WebCLInputChecker::isValidCommandQueueProperty(properties)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    RefPtr<WebCLDevice> webCLDevice;
    if (!device) {
        Vector<RefPtr<WebCLDevice> > webCLDevices = m_devices;
        // NOTE: This can be slow, depending the number of 'devices' available.
        for (size_t i = 0; i < webCLDevices.size(); ++i) {
            WebCLGetInfo info = webCLDevices[i]->getInfo(ComputeContext::DEVICE_QUEUE_PROPERTIES, ec);
            if (ec == WebCLException::SUCCESS
                && (info.getUnsignedInt() == static_cast<unsigned>(properties) || !properties)) {
                webCLDevice = webCLDevices[i];
                break;
            } else {
                ec = WebCLException::INVALID_QUEUE_PROPERTIES;
                return 0;
            }
        }
        //FIXME: Spec needs to say what we need to do here
        ASSERT(webCLDevice);
    } else {
        size_t i = 0;
        for (i = 0; i < m_devices.size(); i++) {
            if (m_devices[i]->platformObject() == device->platformObject()) {
                webCLDevice = device;
                break;
            }
        }
        if (i == m_devices.size()) {
            ec = WebCLException::INVALID_DEVICE;
            return 0;
        }
    }

    RefPtr<WebCLCommandQueue> queue = WebCLCommandQueue::create(this, properties, webCLDevice.get(), ec);
    postCreateCommandQueue(queue.get(), ec);
    return queue.release();
}

PassRefPtr<WebCLProgram> WebCLContext::createProgram(const String& programSource, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!programSource.length()) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    return WebCLProgram::create(this, programSource, ec);
}

void WebCLContext::postCreateCommandQueue(WebCLCommandQueue* queue, ExceptionCode& ec)
{
    if (!queue)
        return;

    m_memoryInitializer.commandQueueCreated(queue, ec);
}

void WebCLContext::postCreateBuffer(WebCLBuffer* buffer, ExceptionCode& ec)
{
    if (!buffer)
        return;

    m_memoryInitializer.bufferCreated(buffer, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBufferBase(CCenum memoryFlags, CCuint size, void* hostPtr, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    if (!WebCLInputChecker::isValidMemoryObjectFlag(memoryFlags)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    bool needsInitialization = !hostPtr;
    if (hostPtr)
        memoryFlags |= ComputeContext::MEM_COPY_HOST_PTR;

    RefPtr<WebCLBuffer> buffer = WebCLBuffer::create(this, memoryFlags, size, hostPtr, ec);
    if (needsInitialization && buffer) {
        ASSERT(ec == WebCLException::SUCCESS);
        postCreateBuffer(buffer.get(), ec);
    }

    return buffer.release();
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(CCenum memoryFlags, CCuint size, ArrayBufferView* hostPtr, ExceptionCode& ec)
{
    RefPtr<ArrayBuffer> buffer;
    if (hostPtr) {
        if (hostPtr->byteLength() < size) {
            ec = WebCLException::INVALID_HOST_PTR;
            return 0;
        }

        if (!hostPtr->buffer()) {
            ec = WebCLException::INVALID_HOST_PTR;
            return 0;
        }
        buffer = hostPtr->buffer();
    }

    return createBufferBase(memoryFlags, size, buffer ? buffer->data() : 0, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(CCenum memoryFlags, ImageData* srcPixels, ExceptionCode& ec)
{
    if (!isExtensionEnabled("WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }

    void* hostPtr = 0;
    size_t pixelSize = 0;
    WebCLHTMLInterop::extractDataFromImageData(srcPixels, hostPtr, pixelSize, ec);
    if (ec != WebCLException::SUCCESS)
        return 0;

    return createBufferBase(memoryFlags, pixelSize, hostPtr, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(CCenum memoryFlags, HTMLCanvasElement* srcCanvas, ExceptionCode& ec)
{
    if (!isExtensionEnabled("WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }
    void* hostPtr = 0;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(srcCanvas, hostPtr, canvasSize, ec);
    if (ec != WebCLException::SUCCESS)
        return 0;

    return createBufferBase(memoryFlags, canvasSize, hostPtr, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(CCenum memoryFlags, HTMLImageElement* srcImage, ExceptionCode& ec)
{
    if (!isExtensionEnabled("WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }
    void* hostPtr = 0;
    size_t bufferSize = 0;
    WebCLHTMLInterop::extractDataFromImage(srcImage, hostPtr, bufferSize, ec);
    if (ec != WebCLException::SUCCESS)
        return 0;

    return createBufferBase(memoryFlags, bufferSize, hostPtr, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage2DBase(CCenum flags, CCuint width, CCuint height, CCuint rowPitch, const CCImageFormat& imageFormat,
    void* data, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!width || !height) {
        ec = WebCLException::INVALID_IMAGE_SIZE;
        return 0;
    }

    if (!WebCLInputChecker::isValidMemoryObjectFlag(flags)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    ASSERT(data);
    flags |= ComputeContext::MEM_COPY_HOST_PTR;

    RefPtr<WebCLImageDescriptor> imageDescriptor = WebCLImageDescriptor::create(width, height, rowPitch, imageFormat);
    return WebCLImage::create(this, flags, imageDescriptor.release(), data, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(CCenum flags, HTMLCanvasElement* srcCanvas, ExceptionCode& ec)
{
    if (!isExtensionEnabled("WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }
    void* hostPtr = 0;
    size_t canvasSize = 0;
    WebCLHTMLInterop::extractDataFromCanvas(srcCanvas, hostPtr, canvasSize, ec);
    if (ec != WebCLException::SUCCESS)
        return 0;

    CCuint width = srcCanvas->width();
    CCuint height = srcCanvas->height();
    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBase(flags, width, height, 0 /* rowPitch */, imageFormat, hostPtr, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(CCenum flags, HTMLImageElement* srcImage, ExceptionCode& ec)
{
    if (!isExtensionEnabled("WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }
    void* hostPtr = 0;
    size_t bufferSize = 0;
    WebCLHTMLInterop::extractDataFromImage(srcImage, hostPtr, bufferSize, ec);
    if (ec != WebCLException::SUCCESS)
        return 0;

    CCuint width = srcImage->width();
    CCuint height = srcImage->height();

    CCImageFormat srcImageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBase(flags, width, height, 0 /* rowPitch */, srcImageFormat, hostPtr, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(CCenum flags, HTMLVideoElement* video, ExceptionCode& ec)
{
    if (ComputeContext::MEM_READ_ONLY != flags) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }
    if (!video) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }
    CCuint width =  video->videoWidth();
    CCuint height = video->videoHeight();
    RefPtr<Image> image = videoFrameToImage(video);

    if (!image || !image->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }
    SharedBuffer* sharedBuffer = image->data();
    void* imageData = (void*)sharedBuffer->data();

    if (!imageData) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBase(flags, width, height, 0 /* rowPitch */, imageFormat, imageData, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(CCenum flags, ImageData* srcPixels, ExceptionCode& ec)
{
    if (!isExtensionEnabled("WEBCL_html_image")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }
    void* hostPtr = 0;
    size_t pixelSize = 0;
    WebCLHTMLInterop::extractDataFromImageData(srcPixels, hostPtr, pixelSize, ec);
    if (ec != WebCLException::SUCCESS)
        return 0;

    CCuint width = srcPixels->width();
    CCuint height = srcPixels->height();
    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBase(flags, width, height, 0 /* rowPitch */, imageFormat, hostPtr, ec);
}

unsigned WebCLContext::numberOfChannelsForChannelOrder(CCenum order)
{
    switch(order) {
    case ComputeContext::R:
    case ComputeContext::A:
    case ComputeContext::INTENSITY:
    case ComputeContext::LUMINANCE:
        return 1;
    case ComputeContext::RG:
    case ComputeContext::RA:
    case ComputeContext::Rx:
        return 2;
    case ComputeContext::RGB:
    case ComputeContext::RGx:
        return 3;
    case ComputeContext::RGBA:
    case ComputeContext::BGRA:
    case ComputeContext::ARGB:
    case ComputeContext::RGBx:
        return 4;
    }

    ASSERT_NOT_REACHED();
    return 0;
}

unsigned WebCLContext::bytesPerChannelType(CCenum channelType)
{
    switch(channelType) {
    case ComputeContext::SNORM_INT8:
    case ComputeContext::UNORM_INT8:
    case ComputeContext::SIGNED_INT8:
    case ComputeContext::UNSIGNED_INT8:
        return 1;
    case ComputeContext::SNORM_INT16:
    case ComputeContext::UNORM_INT16:
    case ComputeContext::SIGNED_INT16:
    case ComputeContext::UNSIGNED_INT16:
    case ComputeContext::HALF_FLOAT:
        return 2;
    case ComputeContext::SIGNED_INT32:
    case ComputeContext::UNSIGNED_INT32:
    case ComputeContext::FLOAT:
        return 4;
    case ComputeContext::UNORM_SHORT_565:
    case ComputeContext::UNORM_SHORT_555:
    case ComputeContext::UNORM_INT_101010:
        break;
    }

    ASSERT_NOT_REACHED();
    return 0;
}

PassRefPtr<WebCLImage> WebCLContext::createImage(CCenum flags, WebCLImageDescriptor* descriptor, ArrayBufferView* hostPtr, ExceptionCode& ec)
{
    if (!descriptor) {
        ec = WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR;
        return 0;
    }

    CCuint width =  descriptor->width();
    CCuint height = descriptor->height();
    CCuint rowPitch = descriptor->rowPitch();
    CCenum channelOrder = descriptor->channelOrder();
    CCenum channelType = descriptor->channelType();

    if (!WebCLInputChecker::isValidChannelOrder(channelOrder) || !WebCLInputChecker::isValidChannelType(channelType)) {
        ec = WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR;
        return 0;
    }
    unsigned numberOfChannels = numberOfChannelsForChannelOrder(channelOrder);
    unsigned bytesPerChannel = bytesPerChannelType(channelType);

    // If rowPitch is specified, must be hostPtr != 0 & rowPitch > width * bytesPerPixel.
    if (rowPitch && (!hostPtr || rowPitch < (width * numberOfChannels * bytesPerChannel))) {
        ec = WebCLException::INVALID_IMAGE_SIZE;
        return 0;
    }

    RefPtr<ArrayBuffer> buffer;
    if (hostPtr) {
        unsigned byteLength = hostPtr->byteLength();
        // Validate user sent rowPitch to avoid out of bound access of hostPtr.
        // When rowPitch = 0, rowPitch = width * bytes/pixel, which will be checked by 2nd expression.
        if ((rowPitch && byteLength < (rowPitch * height))
            || byteLength < (width * height * numberOfChannels * bytesPerChannel)) {
            ec = WebCLException::INVALID_HOST_PTR;
            return 0;
        }
        buffer = hostPtr->buffer();
    } else {
        // FIXME :: This is a slow initialization method. Need to verify against kernel init method.
        buffer = ArrayBuffer::create(width * height * numberOfChannels * bytesPerChannel, 1);
    }

    CCImageFormat imageFormat = {channelOrder, channelType};
    return createImage2DBase(flags, width, height, rowPitch, imageFormat, buffer->data(), ec);
}

PassRefPtr<WebCLSampler> WebCLContext::createSampler(CCbool normCoords, CCenum addressingMode, CCenum filterMode, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!WebCLInputChecker::isValidAddressingMode(addressingMode)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    if (!WebCLInputChecker::isValidFilterMode(filterMode)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    if (!normCoords && (addressingMode == ComputeContext::ADDRESS_REPEAT || addressingMode == ComputeContext::ADDRESS_MIRRORED_REPEAT)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    return WebCLSampler::create(this, normCoords, addressingMode, filterMode, ec);
}

PassRefPtr<WebCLUserEvent> WebCLContext::createUserEvent(ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    return WebCLUserEvent::create(this, ec);
}


// FIXME:: Should be a static local function.
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
    : m_buffers(std::make_unique<std::unique_ptr<ImageBuffer>[]>(capacity))
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
        if (buf->logicalSize() != size)
            continue;
        bubbleToFront(i);
        return buf;
    }
    std::unique_ptr<ImageBuffer> temp = ImageBuffer::create(size, 1);
    if (!temp)
        return 0;
    i = std::min(m_capacity - 1, i);
    m_buffers[i] = std::move(temp);

    ImageBuffer* buf = m_buffers[i].get();
    bubbleToFront(i);
    return buf;
}

void WebCLContext::LRUImageBufferCache::bubbleToFront(int idx)
{
    for (int i = idx; i > 0; --i)
        m_buffers[i].swap(m_buffers[i-1]);
}

Vector<RefPtr<WebCLImageDescriptor> > WebCLContext::getSupportedImageFormats(CCenum memoryFlags, ExceptionCode &ec)
{
    Vector<RefPtr<WebCLImageDescriptor> > imageDescriptors;
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return imageDescriptors;
    }

    if (!WebCLInputChecker::isValidMemoryObjectFlag(memoryFlags)) {
        ec = WebCLException::INVALID_VALUE;
        return imageDescriptors;
    }

    Vector<CCImageFormat> supportedImageFormats;
    CCerror error = platformObject()->supportedImageFormats(memoryFlags, ComputeContext::MEM_OBJECT_IMAGE2D, supportedImageFormats);

    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return imageDescriptors;
    }

    CCuint numberOfSupportedImages = supportedImageFormats.size();
    for (size_t i = 0; i < numberOfSupportedImages; ++i)
        imageDescriptors.append(WebCLImageDescriptor::create(supportedImageFormats[i]));
    return imageDescriptors;
}

void WebCLContext::trackReleaseableWebCLObject(WeakPtr<WebCLObject> object)
{
    m_descendantWebCLObjects.append(object);
}

void WebCLContext::releaseAll()
{
    if (isPlatformObjectNeutralized())
        return;

    for (size_t i = 0; i < m_descendantWebCLObjects.size(); ++i) {
        WebCLObject* object = m_descendantWebCLObjects.at(i).get();
        if (!object)
            continue;

        object->release();
    }

    release();
}

#if ENABLE(WEBGL)
PassRefPtr<WebCLBuffer> WebCLContext::createFromGLBuffer(CCenum flags, WebGLBuffer* webGLBuffer, ExceptionCode& ec)
{
    if (!isExtensionEnabled("KHR_gl_sharing")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }

    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!webGLBuffer || !webGLBuffer->object()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    return WebCLBuffer::create(this, flags, webGLBuffer, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createFromGLRenderbuffer(CCenum flags, WebGLRenderbuffer* renderbuffer, ExceptionCode& ec)
{
    if (!isExtensionEnabled("KHR_gl_sharing")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }

    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!renderbuffer) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    return WebCLImage::create(this, flags, renderbuffer, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createFromGLTexture(CCenum flags, CCenum textureTarget, GC3Dint miplevel, WebGLTexture* texture, ExceptionCode& ec)
{
    if (!isExtensionEnabled("KHR_gl_sharing")) {
        ec = WebCLException::WEBCL_EXTENSION_NOT_ENABLED;
        return 0;
    }

    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!texture) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    return WebCLImage::create(this, flags, textureTarget, miplevel, texture, ec);
}
#endif

} // namespace WebCore

#endif // ENABLE(WEBCL)
