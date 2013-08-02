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
#include "WebCLCommandQueue.h"
#include "WebCLEvent.h"
#include "WebCLImage.h"
#include "WebCLInputChecker.h"
#include "WebCLKernel.h"
#include "WebCLMemoryObject.h"
#include "WebCLProgram.h"
#include "WebCLSampler.h"
#include "WebGLBuffer.h"
#include "WebGLRenderbuffer.h"

namespace WebCore {

WebCLContext::~WebCLContext()
{
    releasePlatformObject();
}

PassRefPtr<WebCLContext> WebCLContext::create(PassRefPtr<WebCLContextProperties> properties, CCerror& error)
{
    RefPtr<WebCLContext> context;
    createBase(properties, error, context);
    return context.release();
}

WebCLContext::WebCLContext(ComputeContext* computeContext, PassRefPtr<WebCLContextProperties> properties)
    : WebCLObject(computeContext)
    , m_videoCache(4) // FIXME: Why '4'?
    , m_contextProperties(properties)
{
}

WebCLGetInfo WebCLContext::getInfo(int paramName, ExceptionCode& ec)
{
    if (!platformObject()) {
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

PassRefPtr<WebCLCommandQueue> WebCLContext::createCommandQueue(WebCLDevice* device, int commandQueueProperty, ExceptionCode& ec)
{
    RefPtr<WebCLCommandQueue> queue;
    createCommandQueueBase(device, commandQueueProperty, ec, queue);
    return queue.release();
}

PassRefPtr<WebCLProgram> WebCLContext::createProgram(const String& kernelSource, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    return WebCLProgram::create(this, kernelSource, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(int memoryFlags, int size, ArrayBuffer* data, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    if (!WebCLInputChecker::isValidMemoryObjectFlag(memoryFlags)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }

    void* arrayBufferData = 0;
    if (data) {
        memoryFlags |= ComputeContext::MEM_COPY_HOST_PTR;
        arrayBufferData = data->data();
    }

    return WebCLBuffer::create(this, memoryFlags, size, arrayBufferData, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(int memoryFlags, ImageData *ptr, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!ptr || !ptr->data() || !ptr->data()->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    void* buffer = ptr->data()->data();
    int bufferSize = ptr->data()->length();
    memoryFlags |= ComputeContext::MEM_COPY_HOST_PTR;

   return WebCLBuffer::create(this, memoryFlags, bufferSize, buffer, ec);
}

PassRefPtr<WebCLBuffer> WebCLContext::createBuffer(int memoryFlags, HTMLCanvasElement* srcCanvas, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!srcCanvas || !srcCanvas->buffer()) {
        ec = ComputeContext::INVALID_HOST_PTR;
        return 0;
    }

    CCuint width = srcCanvas->width();
    CCuint height = srcCanvas->height();

    ImageBuffer* imageBuffer = srcCanvas->buffer();
    RefPtr<Uint8ClampedArray> byteArray = imageBuffer->getUnmultipliedImageData(IntRect(0, 0, width, height));

    if (!byteArray) {
        ec = ComputeContext::INVALID_HOST_PTR;
        return 0;
    }

    void* imageData = byteArray->data();
    int bufferSize = byteArray->length();
    if (!imageData || !bufferSize) {
        ec = ComputeContext::INVALID_HOST_PTR;
        return 0;
    }
    memoryFlags |= ComputeContext::MEM_COPY_HOST_PTR;

    return WebCLBuffer::create(this, memoryFlags, bufferSize, imageData, ec);
}


PassRefPtr<WebCLImage> WebCLContext::createImage2DBase(int flags, int width, int height, const CCImageFormat& imageFormat, void *data, ExceptionCode& ec)
{
    if (!width || !height) {
        ec = WebCLException::INVALID_IMAGE_SIZE;
        return 0;
    }

    if (!WebCLInputChecker::isValidMemoryObjectFlag(flags)) {
        ec = WebCLException::INVALID_VALUE;
        return 0;
    }
    if (data)
        flags |= ComputeContext::MEM_COPY_HOST_PTR;


    RefPtr<WebCLImage> imageObject = WebCLImage::create(this, flags, width, height, imageFormat, data, ec);
    return imageObject.release();
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, HTMLCanvasElement* canvasElement, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!canvasElement || !canvasElement->buffer()) {
        ec = ComputeContext::INVALID_HOST_PTR;
        return 0;
    }

    CCuint width = canvasElement->width();
    CCuint height = canvasElement->height();

    ImageBuffer* imageBuffer = canvasElement->buffer();
    RefPtr<Uint8ClampedArray> byteArray = imageBuffer->getUnmultipliedImageData(IntRect(0, 0, width, height));

    if (!byteArray) {
        ec = ComputeContext::INVALID_HOST_PTR;
        return 0;
    }

    void* imageData = byteArray->data();
    int bufferSize = byteArray->length();

    if (!imageData || !bufferSize) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }
    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBase(flags, width, height, imageFormat, imageData, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, HTMLImageElement* image, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!image || !image->cachedImage()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }
    CCuint width = image->width();
    CCuint height = image->height();
    CachedImage* cachedImage = image->cachedImage();

    IntRect rect(0, 0, width, height);
    OwnPtr<ImageBuffer> buffer = ImageBuffer::create(rect.size());
    buffer->context()->drawImage(cachedImage->image(), ColorSpaceDeviceRGB, rect);

    RefPtr<Uint8ClampedArray> byteArray =  buffer->getUnmultipliedImageData(rect);
    if (!byteArray) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }
#if 0
            // FIXME: Is this useful?
            data = (void*) cachedImage->image()->data()->data();
#else
    void* data = byteArray->data();
#endif
    if (!data) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }

    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBase(flags, width, height, imageFormat, data, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, HTMLVideoElement* video, ExceptionCode& ec)
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
    return createImage2DBase(flags, width, height, imageFormat, imageData, ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, ImageData* data, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }
    if (!data || !data->data() || !data->data()->data()) {
        ec = WebCLException::INVALID_HOST_PTR;
        return 0;
    }
    CCuint width = data->width();
    CCuint height = data->height();
    Uint8ClampedArray* byteArray = data->data();

    CCImageFormat imageFormat = {ComputeContext::RGBA, ComputeContext::UNORM_INT8};
    return createImage2DBase(flags, width, height, imageFormat, (void*) byteArray->data(), ec);
}

PassRefPtr<WebCLImage> WebCLContext::createImage(int flags, WebCLImageDescriptor* descriptor, ArrayBuffer* data, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    if (!descriptor) {
        ec = WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR;
        return 0;
    }

    CCuint width =  descriptor->width();
    CCuint height = descriptor->height();

    CCenum channelOrder = static_cast<CCenum>(descriptor->channelOrder());
    CCenum channelType = static_cast<CCenum>(descriptor->channelType());
    if (!WebCLInputChecker::isValidChannelOrder(channelOrder) || !WebCLInputChecker::isValidChannelType(channelType)) {
        ec = WebCLException::INVALID_IMAGE_FORMAT_DESCRIPTOR;
        return 0;
    }

    CCImageFormat imageFormat = {channelOrder, channelType};
    return createImage2DBase(flags, width, height, imageFormat, data ? data->data() : 0, ec);
}

PassRefPtr<WebCLSampler> WebCLContext::createSampler(bool normCoords, int addressingMode, int filterMode, ExceptionCode& ec)
{
    if (!platformObject()) {
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

    return WebCLSampler::create(this, normCoords, addressingMode, filterMode, ec);
}

PassRefPtr<WebCLEvent> WebCLContext::createUserEvent(ExceptionCode& ec)
{
    UNUSED_PARAM(ec);
    /*if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return 0;
    }

    CCerror error;
    CCEvent ccEvent = platformObject()->createUserEvent(error);
    if (!ccEvent) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    RefPtr<WebCLEvent> resultEvent = WebCLEvent::create(this, ccEvent);
    return resultEvent.release();*/

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

Vector<RefPtr<WebCLImageDescriptor> > WebCLContext::getSupportedImageFormats(int memoryFlags, ExceptionCode &ec)
{
    Vector<RefPtr<WebCLImageDescriptor> > imageDescriptors;
    if (!platformObject()) {
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

void WebCLContext::releasePlatformObjectImpl()
{
    delete platformObject();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
