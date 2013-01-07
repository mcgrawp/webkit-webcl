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
#include "WebCLException.h"
#include <OpenCL/opencl.h>

namespace WebCore {

class WebCL;
class WebCLCommandQueue;
class WebCLProgram;
class WebCLMemoryObject;
class WebCLImage;
class WebCLSampler;
class WebCLEvent;
class ImageData;
class ImageBuffer;
class IntSize;
class WebCLBuffer;
class WebCLImageDescriptor;
class WebCLImageDescriptorList;
class HTMLCanvasElement;
class HTMLImageElement;
class HTMLVideoElement;
class WebGLRenderbuffer;
class WebGLBuffer;
class WebCLDevice;
class WebCLDeviceList;
class WebCLGetInfo;

class WebCLContext : public RefCounted<WebCLContext> {
public:
    virtual ~WebCLContext();
    static PassRefPtr<WebCLContext> create(WebCL*, CCContextProperties* , CCuint , CCDeviceID* , CCerror&);
    /* FIXME :: not needed as CCContextProperties contains deviceType field.
       static PassRefPtr<WebCLContext> create(WebCL*, CCContextProperties* contextProperties, unsigned int deviceType
       , int* error);*/

    PassRefPtr<WebCLBuffer> createBuffer(int, int, ArrayBuffer*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(int memFlags , int sizeInBytes , ExceptionCode& ec)
    {
        return(createBuffer(memFlags , sizeInBytes , 0 , ec));
    }
    PassRefPtr<WebCLCommandQueue> createCommandQueue(WebCLDeviceList* , int , ExceptionCode&);
    PassRefPtr<WebCLCommandQueue> createCommandQueue(WebCLDeviceList* deviceList , ExceptionCode& ec)
    {
        return(createCommandQueue(deviceList, 0 , ec));
    }
    PassRefPtr<WebCLCommandQueue> createCommandQueue(ExceptionCode& ec)
    {
        return(createCommandQueue(0, 0, ec));
    }

    PassRefPtr<WebCLBuffer> createFromGLBuffer(int , WebGLBuffer* , ExceptionCode&);

    PassRefPtr<WebCLImage> createFromGLRenderBuffer(int , WebGLRenderbuffer* , ExceptionCode&);

    PassRefPtr<WebCLMemoryObject> createFromGLTexture2D(int , GC3Denum , GC3Dint , GC3Duint , ExceptionCode&);

    PassRefPtr<WebCLImage> createImageWithDescriptor(int , WebCLImageDescriptor* , ArrayBuffer* , ExceptionCode&);
    PassRefPtr<WebCLImage> createImageWithDescriptor(int flag , WebCLImageDescriptor* descriptor , ExceptionCode& ec)
    {
        return createImageWithDescriptor( flag , descriptor , 0 , ec);
    }
    PassRefPtr<WebCLProgram> createProgram(const String&, ExceptionCode&);

    PassRefPtr<WebCLSampler> createSampler(bool, int, int, ExceptionCode&);

    PassRefPtr<WebCLEvent> createUserEvent(ExceptionCode&);

    WebCLGetInfo getInfo(int, ExceptionCode&);

    PassRefPtr<WebCLImageDescriptorList> getSupportedImageFormats(int , int , ExceptionCode&);

    // Strawman proposal
    PassRefPtr<WebCLBuffer> createBuffer(int, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(int, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int, HTMLImageElement*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int, HTMLVideoElement*, ExceptionCode&);

    // FIXME :: API not in Specs but used in Conformance test suit. Please
    // remove it in future
    PassRefPtr<WebCLMemoryObject> createImage2D(int, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLMemoryObject> createImage2D(int, HTMLImageElement*, ExceptionCode&);
    PassRefPtr<WebCLMemoryObject> createImage2D(int, HTMLVideoElement*, ExceptionCode&);
    PassRefPtr<WebCLMemoryObject> createImage2D(int, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLMemoryObject> createImage2D(int , unsigned , unsigned , ArrayBuffer* , ExceptionCode&);

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
    WebCLContext(WebCL*, CCContextProperties* , CCuint , CCDeviceID* , CCerror&);
    // WebCLContext(WebCL*, CCContextProperties* contextProperties, unsigned int deviceType, int* error);
    PassRefPtr<WebCLMemoryObject> createImage2DBaseMemory(int , int , int , const ComputeContext::ImageFormat& , void*
        , ExceptionCode&);
    PassRefPtr<WebCLImage> createImage2DBaseImage(int , int , int , const ComputeContext::ImageFormat& , void*
        , ExceptionCode&);

    WebCL* m_context;
    cl_context m_clContext;
    RefPtr<WebCLCommandQueue> m_commandQueue;
    PassRefPtr<Image> videoFrameToImage(HTMLVideoElement*);
    RefPtr<ComputeContext> m_computeContext;

    /* FIXME : These are not used. Check if needed for garbage collection. If not delete it.
    RefPtr<WebCLDevice> m_device_id;
    Vector<RefPtr<WebCLProgram> > m_program_list;
    long m_num_programs;
    Vector<RefPtr<WebCLMemoryObject> > m_mem_list;
    long m_num_mems;
    Vector<RefPtr<WebCLImage> > m_img_list;
    long m_num_images;
    Vector<RefPtr<WebCLBuffer> > m_buffer_list;
    long m_num_buffer;
    Vector<RefPtr<WebCLEvent> > m_event_list;
    long m_num_events;
    Vector<RefPtr<WebCLSampler> > m_sampler_list;
    long m_num_samplers;
    Vector<RefPtr<WebCLContext> > m_context_list;
    long m_num_contexts;*/
};


} // namespace WebCore

#endif // WebCLContext_h
