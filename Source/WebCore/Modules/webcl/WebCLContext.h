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

#ifndef WebCLContext_h
#define WebCLContext_h

#include "ScriptObject.h"
#include "ScriptState.h"

#include "ActiveDOMObject.h"
#include <wtf/Float32Array.h>
#include <wtf/Int32Array.h>
#include "ImageData.h"
#include "HTMLCanvasElement.h"
#include "HTMLImageElement.h"
#include "HTMLVideoElement.h"
#include "GraphicsContext3D.h"
#include "WebGLBuffer.h"
#include "WebGLRenderingContext.h"
#include "WebCLGetInfo.h"
#include "ScriptExecutionContext.h"
#include "Document.h"
#include "DOMWindow.h"
#include "Image.h"
#include "SharedBuffer.h"
#include "CanvasRenderingContext2D.h"
#include "ImageBuffer.h"
#include "CachedImage.h"
#include <wtf/ArrayBuffer.h>
#include "CachedImage.h"
#include "ComputeContext.h"

#include <wtf/OwnPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/Forward.h>
#include <wtf/Noncopyable.h>
#include <wtf/Vector.h>
#include <PlatformString.h>
#include <OpenCL/opencl.h>
#include <wtf/ArrayBuffer.h>
#include <WebCLImageDescriptor.h>
#include "WebCLImage.h"
#include "WebCLBuffer.h"
#include <WebCLImageDescriptorList.h>

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
class WebCLImageDescriptorList;
class WebCLContext : public RefCounted<WebCLContext> {
public:
    virtual ~WebCLContext();
    static PassRefPtr<WebCLContext> create(WebCL*, CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID *devices, int* error);
    static PassRefPtr<WebCLContext> create(WebCL*, CCContextProperties* contextProperties, unsigned int deviceType, int* error);

    WebCLGetInfo getInfo(int, ExceptionCode&);
    PassRefPtr<WebCLCommandQueue> createCommandQueue(WebCLDeviceList*, int, ExceptionCode&);
    PassRefPtr<WebCLCommandQueue> createCommandQueue(WebCLDeviceList* deviceList, ExceptionCode& ec) {
        return(createCommandQueue(deviceList, NULL, ec));
    }
    PassRefPtr<WebCLCommandQueue> createCommandQueue(ExceptionCode& ec) {
        return(createCommandQueue(NULL, NULL, ec));
    }

    PassRefPtr<WebCLProgram> createProgram(const String&, ExceptionCode&);

    PassRefPtr<WebCLBuffer> createBuffer(int, int, ArrayBuffer*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(int memFlags, int sizeInBytes, ExceptionCode& ec)
    {
            return(createBuffer(memFlags,sizeInBytes,NULL,ec));
    }

    PassRefPtr<WebCLBuffer> createBuffer(int, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createBuffer(int, HTMLCanvasElement*, ExceptionCode&);

    // Create Image using ImageData , HTMLCanvasElement or HTMLImageElement as i/p. Can return WebCLBuffer or WebCLImage
    PassRefPtr<WebCLImage> createImage(int, ImageData*, ExceptionCode&);
    PassRefPtr<WebCLImage> createImage(int, HTMLCanvasElement*, ExceptionCode&);
	PassRefPtr<WebCLImage> createImage(int, HTMLImageElement*, ExceptionCode&);

   //  Create Image using HTMLVideoElement as i/p
    PassRefPtr<WebCLImage> createImage(int, HTMLVideoElement*, ExceptionCode&);

    //  Create Image using WebCLImageDescriptor
    PassRefPtr<WebCLImage> createImageWithDescriptor(int,WebCLImageDescriptor*,ArrayBuffer*,ExceptionCode&);
    PassRefPtr<WebCLImage> createImageWithDescriptor(int flag,WebCLImageDescriptor* descriptor,ExceptionCode& ec)
    {
        return createImageWithDescriptor( flag,descriptor,NULL,ec);
    }
    PassRefPtr<WebCLMemoryObject> createImage2D(int, HTMLCanvasElement*, ExceptionCode&);
    PassRefPtr<WebCLMemoryObject> createImage2D(int, HTMLImageElement*, ExceptionCode&);
    PassRefPtr<WebCLMemoryObject> createImage2D(int, HTMLVideoElement*, ExceptionCode&);
    PassRefPtr<WebCLMemoryObject> createImage2D(int, ImageData*, ExceptionCode&);

    PassRefPtr<WebCLMemoryObject> createImage2D(int,unsigned int,unsigned int, ArrayBuffer*, ExceptionCode&);
    PassRefPtr<WebCLSampler> createSampler(bool, int, int, ExceptionCode&);
    PassRefPtr<WebCLBuffer> createFromGLBuffer(int, WebGLBuffer*, ExceptionCode&);
    //PassRefPtr<WebCLMemoryObject> createFromGLBuffer(int, WebGLBuffer*, ExceptionCode&);
    PassRefPtr<WebCLImage> createFromGLRenderBuffer(int, WebGLRenderbuffer*,ExceptionCode&);
    PassRefPtr<WebCLMemoryObject> createFromGLTexture2D(int, GC3Denum,GC3Dint, GC3Duint, ExceptionCode&);
    PassRefPtr<WebCLEvent> createUserEvent(ExceptionCode&);

    PassRefPtr<WebCLImageDescriptorList> getSupportedImageFormats(int memFlags,int imageType,ExceptionCode&);
    void setDevice(RefPtr<WebCLDevice>);
    cl_context getCLContext();
    // Fixed-size cache of reusable image buffers for video texImage2D calls.
    class LRUImageBufferCache {
    public:
        LRUImageBufferCache(int capacity);
        // The pointer returned is owned by the image buffer map.
        ImageBuffer* imageBuffer(const IntSize& size);
    private:
        void bubbleToFront(int idx);
        OwnArrayPtr<OwnPtr<ImageBuffer> > m_buffers;
        int m_capacity;
    };
    LRUImageBufferCache m_videoCache;

    ComputeContext* computeContext() const { return m_computeContext.get(); }

private:
    WebCLContext(WebCL*, CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID *devices, int* error);
    WebCLContext(WebCL*, CCContextProperties* contextProperties, unsigned int deviceType, int* error);
    PassRefPtr<WebCLMemoryObject> createImage2DBaseMemory(int flags, int width, int height, const ComputeContext::ImageFormat& imageFormat, void* data, ExceptionCode& ec);
    PassRefPtr<WebCLImage> createImage2DBaseImage(int flags, int width, int height, const ComputeContext::ImageFormat& imageFormat, void* data, ExceptionCode& ec);

    WebCL* m_context;
    cl_context m_cl_context;
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
    long m_num_contexts;
    RefPtr<WebCLCommandQueue> m_command_queue;

    PassRefPtr<Image> videoFrameToImage(HTMLVideoElement*);
    RefPtr<ComputeContext> m_computeContext;
};


} // namespace WebCore

#endif // WebCLContext_h
