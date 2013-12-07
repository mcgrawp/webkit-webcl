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
#include "ImageData.h"
#include "SharedBuffer.h"
#include "WebCLBuffer.h"
#include "WebCLCommandQueue.h"
#include "WebCLEvent.h"
#include "WebCLImage.h"
#include "WebCLImageDescriptor.h"
#include "WebCLProgram.h"
#include "WebCLSampler.h"
#include "WebCLUserEvent.h"
#if ENABLE(WEBGL)
#include "WebGLBuffer.h"
#include "WebGLRenderbuffer.h"
#include "WebGLTexture.h"
#endif

namespace WebCore {

WebCLContext::~WebCLContext()
{
    releasePlatformObject();
}

PassRefPtr<WebCLContext> WebCLContext::create(PassRefPtr<WebCLContextProperties> properties, ExceptionCode& ec)
{
    ASSERT(properties);

    Vector<CCDeviceID> ccDevices;
    for (size_t i = 0; i < properties->devices().size(); ++i)
        ccDevices.append(properties->devices()[i]->platformObject());

    CCerror error = ComputeContext::SUCCESS;
    ComputeContext* computeContext = new ComputeContext(properties->computeContextProperties().data(), ccDevices, error);
    if (error != ComputeContext::SUCCESS) {
        delete computeContext;
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLContext> context = adoptRef(new WebCLContext(computeContext, properties));
    return context.release();
}

WebCLContext::WebCLContext(ComputeContext* computeContext, PassRefPtr<WebCLContextProperties> properties)
    : WebCLObjectImpl(computeContext)
    , m_videoCache(4) // FIXME: Why '4'?
    , m_contextProperties(properties)
{
}

WebCLGetInfo WebCLContext::getInfo(CCenum paramName, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_CONTEXT;
        return WebCLGetInfo();
    }

    switch (paramName) {
    case ComputeContext::CONTEXT_NUM_DEVICES:
        return WebCLGetInfo(m_contextProperties->devices().size());
    case ComputeContext::CONTEXT_DEVICES:
        return WebCLGetInfo(m_contextProperties->devices());
    case ComputeContext::CONTEXT_PROPERTIES:
        return WebCLGetInfo(m_contextProperties.get());
    default:
        ec = WebCLException::INVALID_VALUE;
    }

    return WebCLGetInfo();
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
        Vector<RefPtr<WebCLDevice> > webCLDevices = m_contextProperties->devices();
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
    } else
        webCLDevice = device;

    return WebCLCommandQueue::create(this, properties, webCLDevice, ec);
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

    if (hostPtr)
        memoryFlags |= ComputeContext::MEM_COPY_HOST_PTR;

    return WebCLBuffer::create(this, memoryFlags, size, hostPtr, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(CCenum memoryFlags, CCuint size, ArrayBufferView* data, ExceptionCode& ec)
{
    void* hostPtr = nullptr;
    if (data) {
        if (data->byteLength() < size) {
            ec = WebCLException::INVALID_HOST_PTR;
            return 0;
        }

        if (!data->buffer()) {
            ec = WebCLException::INVALID_HOST_PTR;
            return 0;
        }
        hostPtr = data->buffer()->data();
    }

    return createBufferBase(memoryFlags, size, hostPtr, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(CCenum memoryFlags, ImageData* srcPixels, ExceptionCode& ec)
{
    if (!srcPixels || !srcPixels->data() || !srcPixels->data()->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    void* hostPtr = srcPixels->data()->data();
    CCuint bufferSize = srcPixels->data()->length();

    return createBufferBase(memoryFlags, bufferSize, hostPtr, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(CCenum memoryFlags, HTMLCanvasElement* srcCanvas, ExceptionCode& ec)
{
    if (!srcCanvas || !srcCanvas->buffer()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    RefPtr<ImageData> imageData = srcCanvas->getImageData();
    if (!imageData || !imageData->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    void* hostPtr = imageData->data()->data();
    CCuint bufferSize = imageData->data()->length();

    return createBufferBase(memoryFlags, bufferSize, hostPtr, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(CCenum memoryFlags, HTMLImageElement* srcImage, ExceptionCode& ec)
{
    if (!srcImage && !srcImage->cachedImage()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    CachedImage* cachedImage = srcImage->cachedImage();
    if (!cachedImage || !cachedImage->image() || !cachedImage->image()->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    void* hostPtr = (void*) cachedImage->image()->data()->data();
    unsigned bufferSize = srcImage->width() * srcImage->height() * 4 /* source pixel format is treated as 32-bit(4 byte) RGBA regardless of the source. */;
    if (!hostPtr) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

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

    if (rowPitch && !data) {
        ec = WebCLException::INVALID_IMAGE_SIZE;
        return 0;
    }

    if (data)
        flags |= ComputeContext::MEM_COPY_HOST_PTR;

    return WebCLImage::create(this, flags, width, height, rowPitch, imageFormat, data, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(CCenum flags, HTMLCanvasElement* srcCanvas, ExceptionCode& ec)
{
    if (!srcCanvas || !srcCanvas->getImageData()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }
    RefPtr<ImageData> imageData = srcCanvas->getImageData();

    if (!imageData && !imageData->data() && !imageData->data()->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    void* hostPtr = imageData->data()->data();
    CCuint width = srcCanvas->width();
    CCuint height = srcCanvas->height();

    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBase(flags, width, height, 0 /* rowPitch */, imageFormat, hostPtr, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(CCenum flags, HTMLImageElement* srcImage, ExceptionCode& ec)
{
    if (!srcImage || !srcImage->cachedImage()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    CachedImage* cachedImage = srcImage->cachedImage();
    if (!cachedImage || !cachedImage->image() || !cachedImage->image()->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    void* hostPtr = (void*) cachedImage->image()->data()->data();
    if (!hostPtr) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

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
    CCuint width =  video->width();
    CCuint height = video->height();
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
    if (!srcPixels || !srcPixels->data() || !srcPixels->data()->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }
    CCuint width = srcPixels->width();
    CCuint height = srcPixels->height();
    Uint8ClampedArray* byteArray = srcPixels->data();

    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBase(flags, width, height, 0 /* rowPitch */, imageFormat, (void*) byteArray->data(), ec);
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

    if (hostPtr && hostPtr->byteLength() < (rowPitch * height)) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    CCuint channelOrder = static_cast<CCenum>(descriptor->channelOrder());
    CCuint channelType = static_cast<CCenum>(descriptor->channelType());
    if (!WebCLInputChecker::isValidChannelOrder(channelOrder) || !WebCLInputChecker::isValidChannelType(channelType)) {
        ec = WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR;
        return 0;
    }

    CCImageFormat imageFormat = {channelOrder, channelType};
    return createImage2DBase(flags, width, height, rowPitch, imageFormat, hostPtr ? hostPtr->baseAddress() : 0, ec);
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
    : m_buffers(adoptArrayPtr(new OwnPtr<ImageBuffer>[capacity]))
    , m_capacity(capacity)
{
}

ImageBuffer* WebCLContext::LRUImageBufferCache::imageBuffer(const IntSize& size)
{
    int i = 0;
    for (i = 0; i < m_capacity; ++i) {
        ImageBuffer* buf = m_buffers[i].get();
        if (!buf)
            break;
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

void WebCLContext::releasePlatformObjectImpl()
{
    // FIXME: We should special case WebCLContext release, instead of
    // deleting the ComputeContext.
    delete platformObject();
}

#if ENABLE(WEBGL)
PassRefPtr<WebCLBuffer> WebCLContext::createFromGLBuffer(CCenum flags, WebGLBuffer* webGLBuffer, ExceptionCode& ec)
{
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
