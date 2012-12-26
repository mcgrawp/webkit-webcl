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

#ifndef WebCLCommandQueue_h
#define WebCLCommandQueue_h

#include <OpenCL/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>
#include <wtf/ArrayBufferView.h>
#include <wtf/Uint8ClampedArray.h>

#include "WebCLGetInfo.h"
#include "WebCLFinishCallback.h"
#include "WebCLProgram.h"
#include "WebCLKernel.h"
#include "WebCLMemoryObject.h"
#include "WebCLMemoryObjectList.h"
#include "WebCLImage.h"
#include "WebCLBuffer.h"
#include "WebCLEvent.h"
#include "WebCLSampler.h"
#include "ImageData.h"
#include "HTMLCanvasElement.h"

namespace WebCore {

class WebCL;
class WebCLContext;
class WebCLEventList;
class WebCLCommandQueue : public RefCounted<WebCLCommandQueue> {
public:
    ~WebCLCommandQueue();
    static PassRefPtr<WebCLCommandQueue> create(WebCLContext*, cl_command_queue);
    WebCLGetInfo getInfo(int, ExceptionCode&);

    void enqueueWriteBuffer(WebCLBuffer*, bool, int, int, ArrayBufferView*, WebCLEventList* ,WebCLEvent* , ExceptionCode&);

    void enqueueWriteBuffer(WebCLBuffer* buffer, bool blocking_write, int offset, int buffer_size, ArrayBufferView* ptr,
                            WebCLEventList* events, ExceptionCode& ec) {
        return(enqueueWriteBuffer(buffer, blocking_write, offset, buffer_size, ptr, events, NULL, ec));
    }

    void enqueueWriteBuffer(WebCLBuffer* buffer, bool blocking_write, int offset, int buffer_size, ArrayBufferView* ptr,
                            ExceptionCode& ec) {
        return(enqueueWriteBuffer(buffer, blocking_write, offset, buffer_size, ptr, NULL, NULL, ec));
    }

    void enqueueWriteBuffer(WebCLBuffer*, bool, int, int,ImageData*,WebCLEventList* ,WebCLEvent*,ExceptionCode&);
    
    void enqueueWriteBuffer(WebCLBuffer* buffer, bool blocking_write, int offset, int buffer_size, ImageData* ptr,WebCLEventList* events, ExceptionCode& ec)
    {
        return(enqueueWriteBuffer(buffer, blocking_write, offset, buffer_size, ptr, events, NULL, ec));
    }
    
    void enqueueWriteBuffer(WebCLBuffer* buffer, bool blocking_write, int offset, int buffer_size, ImageData* ptr,ExceptionCode& ec)
    {
        return(enqueueWriteBuffer(buffer, blocking_write, offset, buffer_size, ptr,NULL,NULL, ec));
    }
    void enqueueWriteBufferRect( WebCLBuffer* ,bool ,Int32Array* ,Int32Array* ,Int32Array* ,int ,int ,int ,int ,ArrayBufferView*,
                                 WebCLEventList* ,WebCLEvent* ,ExceptionCode&);
    void  enqueueWriteBufferRect( WebCLBuffer* buffer,bool blocking_write,
                                  Int32Array* bufferOrigin,Int32Array* hostOrigin,Int32Array* region,
                                  int bufferRowPitch,int bufferSlicePitch,int hostRowPitch,int hostSlicePitch,ArrayBufferView* ptr,
                                  WebCLEventList* eventWaitList,ExceptionCode& ec )
    {
        return (enqueueWriteBufferRect(buffer,blocking_write,bufferOrigin,hostOrigin,region,
                                       bufferRowPitch,bufferSlicePitch,hostRowPitch,hostSlicePitch,ptr,
                                       eventWaitList,NULL,ec));
    }
    void  enqueueWriteBufferRect( WebCLBuffer* buffer,bool blocking_write,
                                  Int32Array* bufferOrigin,Int32Array* hostOrigin,Int32Array* region,
                                  int bufferRowPitch,int bufferSlicePitch,int hostRowPitch,int hostSlicePitch,ArrayBufferView* ptr,
                                  ExceptionCode& ec )
    {
        return (enqueueWriteBufferRect(buffer,blocking_write,bufferOrigin,hostOrigin,region,
                                       bufferRowPitch,bufferSlicePitch,hostRowPitch,hostSlicePitch,ptr,
                                       NULL,NULL,ec));
    }
 
    void enqueueWriteBufferRect( WebCLBuffer* mem, bool blocking_write, int offset, int buffer_size, ImageData* ptr,ExceptionCode& ec);
    
    
     
    void enqueueReadBuffer(WebCLBuffer*, bool, int, int,ImageData*,WebCLEventList* ,WebCLEvent*,ExceptionCode&);
     
    void enqueueReadBuffer( WebCLBuffer* buffer, bool blocking_read, int offset, int buffer_size, ImageData* ptr,WebCLEventList* events, ExceptionCode& ec) 
    {
        return(enqueueReadBuffer(buffer, blocking_read, offset, buffer_size, ptr, events, NULL, ec));
    }
    
    void enqueueReadBuffer(WebCLBuffer* buffer, bool blocking_read, int offset, int buffer_size, ImageData* ptr, ExceptionCode& ec) {
        return(enqueueReadBuffer(buffer, blocking_read, offset, buffer_size, ptr, NULL, NULL, ec));
    }
	
    void enqueueReadBuffer(WebCLBuffer*, bool, int, int, ArrayBufferView*, WebCLEventList* ,WebCLEvent* , ExceptionCode&);
    void enqueueReadBuffer( WebCLBuffer* buffer, bool blocking_read, int offset, int buffer_size, ArrayBufferView* ptr, 
                            WebCLEventList* events, ExceptionCode& ec) {
        return(enqueueReadBuffer(buffer, blocking_read, offset, buffer_size, ptr, events, NULL, ec));
    }

    void enqueueReadBuffer(WebCLBuffer* buffer, bool blocking_read, int offset, int buffer_size, ArrayBufferView* ptr, 
                           ExceptionCode& ec) {
        return(enqueueReadBuffer(buffer, blocking_read, offset, buffer_size, ptr, NULL, NULL, ec));
    }
    void enqueueReadBufferRect( WebCLBuffer* ,bool ,Int32Array* ,Int32Array* ,Int32Array* ,int ,int ,int ,int ,ArrayBufferView*,
                                WebCLEventList* ,WebCLEvent* ,ExceptionCode&);
    void  enqueueReadBufferRect( WebCLBuffer* buffer,bool blocking_read,
                                 Int32Array* bufferOrigin,Int32Array* hostOrigin,Int32Array* region,
                                 int bufferRowPitch,int bufferSlicePitch,int hostRowPitch,int hostSlicePitch,ArrayBufferView* ptr,
                                 WebCLEventList* eventWaitList,ExceptionCode& ec )
    {
        return (enqueueReadBufferRect(buffer,blocking_read,bufferOrigin,hostOrigin,region,
                                      bufferRowPitch,bufferSlicePitch,hostRowPitch,hostSlicePitch,ptr,
                                      eventWaitList,NULL,ec));
    }
    void  enqueueReadBufferRect( WebCLBuffer* buffer,bool blocking_read,
                                 Int32Array* bufferOrigin,Int32Array* hostOrigin,Int32Array* region,
                                 int bufferRowPitch,int bufferSlicePitch,int hostRowPitch,int hostSlicePitch,ArrayBufferView* ptr,
                                 ExceptionCode& ec )
    {
        return (enqueueReadBufferRect(buffer,blocking_read,bufferOrigin,hostOrigin,region,
                                      bufferRowPitch,bufferSlicePitch,hostRowPitch,hostSlicePitch,ptr,
                                      NULL,NULL,ec));
    }
    void  enqueueNDRangeKernel(WebCLKernel* ,Int32Array* ,
                               Int32Array* ,Int32Array* ,WebCLEventList* ,WebCLEvent* , ExceptionCode&);
    void  enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* offsets,
                               Int32Array* global_work_size, Int32Array* local_work_size, WebCLEventList* events, ExceptionCode& ec) {
        return (enqueueNDRangeKernel(kernel ,offsets ,global_work_size ,local_work_size ,events, NULL, ec)); 	
    }
    void  enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* offsets,
                               Int32Array* global_work_size, Int32Array* local_work_size, ExceptionCode& ec) {
        return (enqueueNDRangeKernel(kernel ,offsets ,global_work_size ,local_work_size , NULL,	NULL, ec)); 	
    }
	
    void finish(ExceptionCode&);
    void flush( ExceptionCode&);
    void enqueueWriteImage(WebCLImage* ,bool ,Int32Array* ,Int32Array* ,int ,int ,ArrayBufferView* ,WebCLEventList* ,WebCLEvent* ,ExceptionCode&);
    void enqueueWriteImage(WebCLImage* image,bool blockingWrite,Int32Array* origin,Int32Array* region,
                           int inputRowPitch,int inputSlicePitch,ArrayBufferView* ptr,WebCLEventList* eventWaitList,ExceptionCode& ec){
        enqueueWriteImage(image,blockingWrite,origin,region,inputRowPitch,inputSlicePitch,ptr,eventWaitList,NULL,ec);
    }
    void enqueueWriteImage(WebCLImage* image,bool blockingWrite,Int32Array* origin,Int32Array* region,
                           int inputRowPitch,int inputSlicePitch,ArrayBufferView* ptr,ExceptionCode& ec){
        enqueueWriteImage(image,blockingWrite,origin,region,inputRowPitch,inputSlicePitch,ptr,NULL,NULL,ec);
    }
	
    void enqueueReadImage(WebCLImage*, bool, Int32Array*, Int32Array*, int, int, ArrayBufferView*, WebCLEventList* ,WebCLEvent* , ExceptionCode&);
	
    void enqueueReadImage(WebCLImage* image , bool blocking_read, Int32Array* origin, Int32Array* region,int rowPitch , int slicePitch, ArrayBufferView* ptr,
                          WebCLEventList* events, ExceptionCode& ec) {
        return(enqueueReadImage(image, blocking_read, origin, region, rowPitch, slicePitch, ptr, events, NULL, ec));
    }

    void enqueueReadImage(WebCLImage* image, bool blocking_read, Int32Array* origin, Int32Array* region,int rowPitch , int slicePitch, ArrayBufferView* ptr,
                          ExceptionCode& ec) {
        return(enqueueReadImage(image, blocking_read, origin, region, rowPitch, slicePitch, ptr, NULL, NULL, ec));
    }


    void enqueueAcquireGLObjects(WebCLMemoryObject* ,WebCLEventList* ,WebCLEvent*, ExceptionCode&);
    void enqueueAcquireGLObjects(WebCLMemoryObject *mem_list, WebCLEventList* events, ExceptionCode& ec) {
        return(enqueueAcquireGLObjects( mem_list , events,  NULL, ec));
    }
    void enqueueAcquireGLObjects(WebCLMemoryObject *mem_list , ExceptionCode& ec) {
        return(enqueueAcquireGLObjects(mem_list,  NULL, NULL, ec));
    }
	
    void enqueueReleaseGLObjects(WebCLMemoryObject* , WebCLEventList* ,WebCLEvent*, ExceptionCode&);
    void enqueueReleaseGLObjects(WebCLMemoryObject *mem_list,WebCLEventList* events, ExceptionCode& ec) {
        return(enqueueReleaseGLObjects(mem_list,  events,  NULL, ec));
    }
    void enqueueReleaseGLObjects(WebCLMemoryObject  *mem_list, ExceptionCode& ec) {
        return(enqueueReleaseGLObjects(mem_list,  NULL, NULL, ec));
    }

    void enqueueCopyBuffer(WebCLBuffer*, WebCLBuffer*, int, ExceptionCode&);
    void enqueueCopyBuffer(WebCLBuffer*, WebCLBuffer*, int, int, int, WebCLEventList* ,WebCLEvent*, ExceptionCode&);
    void enqueueCopyBuffer(WebCLBuffer* src_buffer, WebCLBuffer* dst_buffer,int srcOffset,int dstOffset,int sizeInBytes, WebCLEventList* events,
                           ExceptionCode& ec) {
        return	(enqueueCopyBuffer(src_buffer,dst_buffer,srcOffset,dstOffset,sizeInBytes,events,NULL,ec));
    }

    void enqueueCopyBuffer(WebCLBuffer* src_buffer, WebCLBuffer* dst_buffer,int srcOffset,int dstOffset,int sizeInBytes,
                           ExceptionCode& ec) {
        return	(enqueueCopyBuffer(src_buffer,dst_buffer,srcOffset,dstOffset,sizeInBytes,NULL,NULL,ec));
    }

    void enqueueCopyBufferRect( WebCLBuffer* ,WebCLBuffer* ,Int32Array* ,Int32Array* ,Int32Array* ,int ,int ,int ,int ,
                                WebCLEventList* ,WebCLEvent* ,ExceptionCode&);
    void  enqueueCopyBufferRect( WebCLBuffer* srcBuffer,WebCLBuffer* dstBuffer,
                                 Int32Array* srcOrigin,Int32Array* dstOrigin,Int32Array* region,int srcRowPitch,int srcSlicePitch,int dstRowPitch,int dstSlicePitch,
                                 WebCLEventList* eventWaitList,ExceptionCode& ec )
    {
        return (enqueueCopyBufferRect(srcBuffer,dstBuffer,srcOrigin,dstOrigin,region,srcRowPitch,srcSlicePitch,dstRowPitch,dstSlicePitch,
                                      eventWaitList,NULL,ec));
    }
    void  enqueueCopyBufferRect( WebCLBuffer* srcBuffer,WebCLBuffer* dstBuffer,
                                 Int32Array* srcOrigin,Int32Array* dstOrigin,Int32Array* region,int srcRowPitch,int srcSlicePitch,int dstRowPitch,int dstSlicePitch,
                                 ExceptionCode& ec )
    {
        return (enqueueCopyBufferRect(srcBuffer,dstBuffer,srcOrigin,dstOrigin,region,srcRowPitch,srcSlicePitch,dstRowPitch,dstSlicePitch,
                                      NULL,NULL,ec));
    }

    void enqueueCopyImage(WebCLImage*, WebCLImage*, Int32Array*, Int32Array*, Int32Array*, WebCLEventList* ,WebCLEvent*, ExceptionCode&);
    void enqueueCopyImage(WebCLImage* src_image, WebCLImage* dst_image,Int32Array* src_origin,Int32Array* dst_origin,Int32Array* region, WebCLEventList* events,
                          ExceptionCode& ec) {
        return	(enqueueCopyImage(src_image,dst_image,src_origin,dst_origin,region,events,NULL,ec));
    }

    void enqueueCopyImage(WebCLImage* src_image, WebCLImage* dst_image,Int32Array* src_origin,Int32Array* dst_origin,Int32Array* region,
                          ExceptionCode& ec) {
        return	(enqueueCopyImage(src_image,dst_image,src_origin,dst_origin,region,NULL,NULL,ec));
    }

    void enqueueCopyImageToBuffer(WebCLImage* ,WebCLBuffer* ,Int32Array* ,Int32Array* ,int , WebCLEventList* ,WebCLEvent*, ExceptionCode&);
    void enqueueCopyImageToBuffer(WebCLImage* srcImage,WebCLBuffer *dstBuffer,Int32Array* srcOrigin,Int32Array* region,int dstOffset,
                                  WebCLEventList* eventWaitList,ExceptionCode& ec){
        enqueueCopyImageToBuffer(srcImage,dstBuffer,srcOrigin,region,dstOffset,eventWaitList,NULL,ec);
    }
    void enqueueCopyImageToBuffer(WebCLImage* srcImage,WebCLBuffer *dstBuffer,Int32Array* srcOrigin,Int32Array* region,int dstOffset,ExceptionCode& ec){
        enqueueCopyImageToBuffer(srcImage,dstBuffer,srcOrigin,region,dstOffset,NULL,NULL,ec);
    }

    void enqueueCopyBufferToImage(WebCLBuffer* ,WebCLImage* ,int ,Int32Array* ,Int32Array* , WebCLEventList* ,WebCLEvent*, ExceptionCode&);
    void enqueueCopyBufferToImage(WebCLBuffer *srcBuffer ,WebCLImage *dstImage ,int srcOffset ,Int32Array* dstOrigin,Int32Array* region, 
                                  WebCLEventList* eventWaitList, ExceptionCode& ec){
        enqueueCopyBufferToImage(srcBuffer,dstImage,srcOffset,dstOrigin,region,eventWaitList,NULL,ec);
    }
    void enqueueCopyBufferToImage(WebCLBuffer *srcBuffer ,WebCLImage *dstImage ,int srcOffset ,Int32Array* dstOrigin,Int32Array* region, ExceptionCode& ec){
        enqueueCopyBufferToImage(srcBuffer,dstImage,srcOffset,dstOrigin,region,NULL,NULL,ec);
    }

    void enqueueBarrier(WebCLEventList* eventsWaitList, WebCLEvent* event, ExceptionCode&);
    void enqueueBarrier(WebCLEventList* eventsWaitList, ExceptionCode& ec)
    {
        return enqueueBarrier(eventsWaitList, NULL, ec);
    }
    void enqueueBarrier(ExceptionCode& ec)
    {
        return enqueueBarrier(NULL, NULL, ec);
    }

    void enqueueMarker(WebCLEventList* eventsWaitList, WebCLEvent*, ExceptionCode&);
    void enqueueMarker(WebCLEventList* eventsWaitList, ExceptionCode& ec)
    {
        return enqueueMarker(eventsWaitList, NULL, ec);
    }
    void enqueueMarker(ExceptionCode& ec)
    {
        return enqueueMarker(NULL, NULL, ec);
    }

    PassRefPtr<WebCLEvent> enqueueTask(WebCLKernel*, int, ExceptionCode&);

    cl_command_queue getCLCommandQueue();	
private:
    WebCLCommandQueue(WebCLContext*, cl_command_queue);

    WebCLContext* m_context;
    cl_command_queue m_cl_command_queue;
    RefPtr<WebCLFinishCallback> m_finishCallback;
    RefPtr<WebCLCommandQueue> m_command_queue;

    long m_num_commandqueues;
    long m_num_mems;
    Vector<RefPtr<WebCLCommandQueue> > m_commandqueue_list;
    Vector<RefPtr<WebCLMemoryObject> > m_mem_list;
};

} // namespace WebCore
#endif // WebCLCommandQueue_h
