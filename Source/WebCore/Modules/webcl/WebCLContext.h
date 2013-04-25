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

#include "ComputeContext.h"
#include "GraphicsContext3D.h"
#include "ImageBuffer.h"
#include "ImageData.h"
#include "ScriptObject.h"
#include "ScriptState.h"
#include "SharedBuffer.h"
#include "WebCLBuffer.h"
#include "WebCLCommandQueue.h"
#include "WebCLDevice.h"
#include "WebCLException.h"
#include "WebCLContextProperties.h"
#include "WebCLGetInfo.h"

namespace WebCore {

class ImageData;
class ImageBuffer;
class HTMLCanvasElement;
class HTMLImageElement;
class HTMLVideoElement;
class WebCL;
class WebCLProgram;
class WebCLMemoryObject;
class WebCLImage;
class WebCLSampler;
class WebCLEvent;
class WebCLContextProperties;
class WebCLImageDescriptor;
class WebGLRenderbuffer;
class WebGLBuffer;
class WebCLDevice;
class WebCLGetInfo;
class WebCLCommandQueue;

// NOTE: WebCLObject used by WebCLContext is a bit different, because the
// other WebCL classes have as platformObject() a native opencl type. However
// WebCLContext has as platformObject() an abstraction called ComputeContext

typedef ComputeContext* ComputeContextPtr;
class WebCLContext : public WebCLObject<ComputeContextPtr> {
public:
    virtual ~WebCLContext();
    static PassRefPtr<WebCLContext> create(PassRefPtr<WebCLContextProperties>, CCerror&);

    PassRefPtr<WebCLBuffer> createBuffer(int memFlags, int sizeInBytes, ArrayBuffer*, ExceptionCode&);

    PassRefPtr<WebCLCommandQueue> createCommandQueue(WebCLDevice*, int commandQueueProperty, ExceptionCode&);

    PassRefPtr<WebCLImage> createImage(int flag, WebCLImageDescriptor*, ArrayBuffer*, ExceptionCode&);

    PassRefPtr<WebCLProgram> createProgram(const String& programSource, ExceptionCode&);

    PassRefPtr<WebCLSampler> createSampler(bool normalizedCoords, int addressingMode, int filterMode, ExceptionCode&);

    PassRefPtr<WebCLEvent> createUserEvent(ExceptionCode&);

    WebCLGetInfo getInfo(int flag, ExceptionCode&);

    Vector<RefPtr<WebCLImageDescriptor> > getSupportedImageFormats(int memFlag, ExceptionCode&);

    // Strawman proposal
    PassRefPtr<WebCLBuffer> createBuffer(int memFlag, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(int memFlag, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int memFlag, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int memFlag, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int memFlag, HTMLImageElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int memFlag, HTMLVideoElement*, ExceptionCode&);

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

protected:
    WebCLContext(ComputeContext*, PassRefPtr<WebCLContextProperties>);
    // WebCLContext(CCContextProperties* contextProperties, unsigned int deviceType, int* error);

    template <class T>
    static void createBase(PassRefPtr<WebCLContextProperties> properties, CCerror& error, RefPtr<T>& out);

    template <class T>
    void createCommandQueueBase(WebCLDevice*, int queueProperties, ExceptionCode&, RefPtr<T>& out);

private:
    PassRefPtr<WebCLImage> createImage2DBase(int flags, int width, int height, const CCImageFormat&, void*, ExceptionCode&);

    void releasePlatformObjectImpl();

    RefPtr<WebCLCommandQueue> m_commandQueue;
    PassRefPtr<Image> videoFrameToImage(HTMLVideoElement*);
    RefPtr<WebCLContextProperties> m_contextProperties;
};

template <class T>
inline void WebCLContext::createBase(PassRefPtr<WebCLContextProperties> properties, CCerror& error, RefPtr<T>& out)
{
    ASSERT(properties);
    out = 0;

    Vector<CCDeviceID> ccDevices;
    for (size_t i = 0; i < properties->devices().size(); ++i)
        ccDevices.append(properties->devices()[i]->platformObject());

    ComputeContext* computeContext = new ComputeContext(properties->computeContextProperties().data(), ccDevices, error);
    if (error != ComputeContext::SUCCESS) {
        delete computeContext;
        return;
    }

    out = adoptRef(new T(computeContext, properties));
}

template <class T>
inline void WebCLContext::createCommandQueueBase(WebCLDevice* device, int queueProperties, ExceptionCode& ec, RefPtr<T>& out)
{
    out = 0;
    if (!platformObject()) {
        ec = WebCLException::INVALID_CONTEXT;
        return;
    }

    RefPtr<WebCLDevice> webCLDevice;
    if (!device) {
        Vector<RefPtr<WebCLDevice> > webCLDevices = m_contextProperties->devices();
        // NOTE: This can be slow, depending the number of 'devices' available.
        for (size_t i = 0; i < webCLDevices.size(); ++i) {
            WebCLGetInfo info = webCLDevices[i]->getInfo(ComputeContext::DEVICE_QUEUE_PROPERTIES, ec);
            if (ec == WebCLException::SUCCESS
                && (info.getUnsignedInt() == static_cast<unsigned>(queueProperties) || !queueProperties)) {
                webCLDevice = webCLDevices[i];
                break;
            }
        }
        //FIXME: Spec needs to say what we need to do here
        ASSERT(webCLDevice);
    } else
        webCLDevice = device;

    out = T::create(this, queueProperties, webCLDevice, ec);
    m_commandQueue = out;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCLContext_h
