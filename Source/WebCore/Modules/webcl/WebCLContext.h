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

#ifndef WebCLContext_h
#define WebCLContext_h

#if ENABLE(WEBCL)

#include "WebCLContextProperties.h"
#include "WebCLDevice.h"
#include "WebCLGetInfo.h"
#include "WebCLInputChecker.h"
#include "WebCLObject.h"

namespace WebCore {

class HTMLCanvasElement;
class HTMLImageElement;
class HTMLVideoElement;
class ImageBuffer;
class ImageData;
class WebCLBuffer;
class WebCLCommandQueue;
class WebCLDevice;
class WebCLEvent;
class WebCLImage;
class WebCLImageDescriptor;
class WebCLProgram;
class WebCLSampler;
class WebCLUserEvent;
#if ENABLE(WEBGL)
class WebGLRenderbuffer;
class WebGLTexture;
#endif

// NOTE: WebCLObject used by WebCLContext is a bit different, because the
// other WebCL classes have as platformObject() a native opencl type. However
// WebCLContext has as platformObject() an abstraction called ComputeContext

typedef ComputeContext* ComputeContextPtr;
class WebCLContext : public WebCLObjectImpl<ComputeContextPtr> {
public:
    virtual ~WebCLContext();
    static PassRefPtr<WebCLContext> create(PassRefPtr<WebCLContextProperties>, ExceptionCode&);

    PassRefPtr<WebCLCommandQueue> createCommandQueue(WebCLDevice*, CCenum commandQueueProperty, ExceptionCode&);

    PassRefPtr<WebCLImage> createImage(CCenum flag, WebCLImageDescriptor*, ArrayBufferView*, ExceptionCode&);

    PassRefPtr<WebCLProgram> createProgram(const String& programSource, ExceptionCode&);

    PassRefPtr<WebCLSampler> createSampler(CCbool normalizedCoords, CCenum addressingMode, CCenum filterMode, ExceptionCode&);

    PassRefPtr<WebCLUserEvent> createUserEvent(ExceptionCode&);

    WebCLGetInfo getInfo(CCenum flag, ExceptionCode&);

    Vector<RefPtr<WebCLImageDescriptor> > getSupportedImageFormats(CCenum memFlag, ExceptionCode&);

    PassRefPtr<WebCLBuffer> createBuffer(CCenum memFlags, CCuint sizeInBytes, ArrayBufferView*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(CCenum memFlag, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(CCenum memFlag, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(CCenum memFlag, HTMLImageElement*, ExceptionCode&);

    PassRefPtr<WebCLImage> createImage(CCenum memFlag, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(CCenum memFlag, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(CCenum memFlag, HTMLImageElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(CCenum memFlag, HTMLVideoElement*, ExceptionCode&);

#if ENABLE(WEBGL)
    PassRefPtr<WebCLBuffer> createFromGLBuffer(CCenum, WebGLBuffer*, ExceptionCode&);
    PassRefPtr<WebCLImage> createFromGLRenderbuffer(CCenum, WebGLRenderbuffer*, ExceptionCode&);
    PassRefPtr<WebCLImage> createFromGLTexture(CCenum memoryFlags, CCenum textureTarget, GC3Dint miplevel, WebGLTexture*, ExceptionCode&);
#endif

    class LRUImageBufferCache {
    public:
        LRUImageBufferCache(int capacity);
        // The pointer returned is owned by the image buffer map.
        ImageBuffer* imageBuffer(const IntSize&);
    private:
        void bubbleToFront(int idx);
        OwnArrayPtr<OwnPtr<ImageBuffer> > m_buffers;
        int m_capacity;
    };
    LRUImageBufferCache m_videoCache;

    ComputeContext* computeContext() const { return platformObject(); }

    void trackReleaseableWebCLObject(WeakPtr<WebCLObject>);
    void releaseAll();

protected:
    WebCLContext(ComputeContext*, PassRefPtr<WebCLContextProperties>);
    // FIXME: Delete the constructor below?
    // WebCLContext(CCContextProperties* contextProperties, unsigned int deviceType, int* error);

private:
    PassRefPtr<WebCLImage> createImage2DBase(CCenum flags, CCuint width, CCuint height, CCuint rowPitch, const CCImageFormat&, void*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBufferBase(CCenum memoryFlags, CCuint size, void* data, ExceptionCode&);

    void releasePlatformObjectImpl();

    RefPtr<WebCLCommandQueue> m_commandQueue;
    PassRefPtr<Image> videoFrameToImage(HTMLVideoElement*);
    RefPtr<WebCLContextProperties> m_contextProperties;

    Vector<WeakPtr<WebCLObject> > m_descendantWebCLObjects;
};

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCLContext_h
