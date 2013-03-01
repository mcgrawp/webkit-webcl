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

#include "ComputeContext.h"
#include "GraphicsContext3D.h"
#include "ImageBuffer.h"
#include "ImageData.h"
#include "ScriptObject.h"
#include "ScriptState.h"
#include "SharedBuffer.h"
#include "WebCLBuffer.h"
#include "WebCLCommandQueue.h"
#include "WebCLException.h"

#include <OpenCL/opencl.h>

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

class WebCLContext : public RefCounted<WebCLContext> {
public:
    virtual ~WebCLContext();
    static PassRefPtr<WebCLContext> create(WebCL*, PassRefPtr<WebCLContextProperties>, CCerror&);
    /* FIXME :: not needed as CCContextProperties contains deviceType field.
       static PassRefPtr<WebCLContext> create(WebCL*, CCContextProperties* contextProperties, unsigned int deviceType
       , int* error);*/

    PassRefPtr<WebCLBuffer> createBuffer(int, int, ArrayBuffer*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(int memFlags , int sizeInBytes , ExceptionCode& ec)
    {
        return(createBuffer(memFlags , sizeInBytes , 0 , ec));
    }
    PassRefPtr<WebCLCommandQueue> createCommandQueue(WebCLDevice*, int , ExceptionCode&);
    PassRefPtr<WebCLCommandQueue> createCommandQueue(WebCLDevice* devices, ExceptionCode& ec)
    {
        return(createCommandQueue(devices, 0 , ec));
    }
    PassRefPtr<WebCLCommandQueue> createCommandQueue(ExceptionCode& ec)
    {
        return(createCommandQueue(0, 0, ec));
    }
    PassRefPtr<WebCLCommandQueue> createCommandQueue(int buildProp, ExceptionCode& ec)
    {
        return(createCommandQueue(0, buildProp, ec));
    }

    PassRefPtr<WebCLBuffer> createFromGLBuffer(int , WebGLBuffer* , ExceptionCode&);

    PassRefPtr<WebCLImage> createFromGLRenderBuffer(int , WebGLRenderbuffer* , ExceptionCode&);

    PassRefPtr<WebCLMemoryObject> createFromGLTexture2D(int , GC3Denum , GC3Dint , GC3Duint , ExceptionCode&);

    PassRefPtr<WebCLImage> createImage(int flag, WebCLImageDescriptor* , ArrayBuffer* , ExceptionCode&);

    PassRefPtr<WebCLProgram> createProgram(const String&, ExceptionCode&);

    PassRefPtr<WebCLSampler> createSampler(bool, int, int, ExceptionCode&);

    PassRefPtr<WebCLEvent> createUserEvent(ExceptionCode&);

    WebCLGetInfo getInfo(int, ExceptionCode&);

    Vector<RefPtr<WebCLImageDescriptor> > getSupportedImageFormats(int, ExceptionCode&);

    // Strawman proposal
    PassRefPtr<WebCLBuffer> createBuffer(int, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(int, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int, HTMLImageElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int, HTMLVideoElement*, ExceptionCode&);

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

    ComputeContext* computeContext() const { return m_computeContext.get(); }

private:
    WebCLContext(WebCL*, RefPtr<ComputeContext>&, PassRefPtr<WebCLContextProperties>, const Vector<CCDeviceID>&);
    // WebCLContext(WebCL*, CCContextProperties* contextProperties, unsigned int deviceType, int* error);
    PassRefPtr<WebCLImage> createImage2DBase(int , int , int , const CCImageFormat& , void*, ExceptionCode&);

    cl_context m_clContext;
    RefPtr<WebCLCommandQueue> m_commandQueue;
    PassRefPtr<Image> videoFrameToImage(HTMLVideoElement*);
    RefPtr<ComputeContext> m_computeContext;
    Vector<CCDeviceID> m_devices;
    RefPtr<WebCLContextProperties> m_contextProperties;
};

} // namespace WebCore

#endif // WebCLContext_h
