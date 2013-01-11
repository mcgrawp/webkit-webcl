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

#include "HTMLCanvasElement.h"
#include "ImageData.h"
#include "WebCLBuffer.h"
#include "WebCLEvent.h"
#include "WebCLFinishCallback.h"
#include "WebCLGetInfo.h"
#include "WebCLImage.h"
#include "WebCLKernel.h"
#include "WebCLMemoryObject.h"
#include "WebCLMemoryObjectList.h"
#include "WebCLProgram.h"
#include "WebCLSampler.h"

#include <OpenCL/opencl.h>

#include <wtf/ArrayBufferView.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Uint8ClampedArray.h>
#include <wtf/Vector.h>

namespace WebCore {

class WebCL;
class WebCLContext;
class WebCLEventList;
class WebCLCommandQueue : public RefCounted<WebCLCommandQueue> {
public:
    ~WebCLCommandQueue();
    static PassRefPtr<WebCLCommandQueue> create(WebCLContext*, cl_command_queue);
    WebCLGetInfo getInfo(int, ExceptionCode&);

    void enqueueWriteBuffer(WebCLBuffer*, bool, int, int, ArrayBufferView*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ArrayBufferView* ptr, WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueWriteBuffer(buffer, blockingWrite, offset, bufferSize, ptr, events, 0, ec);
    }
    void enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueWriteBuffer(buffer, blockingWrite, offset, bufferSize, ptr, 0, 0, ec);
    }
    void enqueueWriteBuffer(WebCLBuffer*, bool, int, int, ImageData*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ImageData* ptr, WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueWriteBuffer(buffer, blockingWrite, offset, bufferSize, ptr, events, 0, ec);
    }
    void enqueueWriteBuffer(WebCLBuffer* buffer, bool blockingWrite, int offset, int bufferSize, ImageData* ptr, ExceptionCode& ec)
    {
        enqueueWriteBuffer(buffer, blockingWrite, offset, bufferSize, ptr, 0, 0, ec);
    }

    void enqueueWriteBufferRect(WebCLBuffer*, bool, Int32Array*, Int32Array*, Int32Array*, int, int, int, int, ArrayBufferView*,
        WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBufferRect(WebCLBuffer* buffer, bool blockingWrite, Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
        int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch, ArrayBufferView* ptr, WebCLEventList* eventWaitList, ExceptionCode& ec)
    {
        enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, hostOrigin, region,
            bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, eventWaitList, 0, ec);
    }
    void enqueueWriteBufferRect(WebCLBuffer* buffer, bool blockingWrite, Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
        int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, hostOrigin, region,
            bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, 0, 0, ec);
    }
    void enqueueWriteBufferRect(WebCLBuffer* mem, bool blockingWrite, int offset, int bufferSize, ImageData* ptr, ExceptionCode&);

    void enqueueReadBuffer(WebCLBuffer*, bool, int, int, ImageData*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ImageData* ptr, WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueReadBuffer(buffer, blockingRead, offset, bufferSize, ptr, events, 0, ec);
    }
    void enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ImageData* ptr, ExceptionCode& ec)
    {
        enqueueReadBuffer(buffer, blockingRead, offset, bufferSize, ptr, 0, 0, ec);
    }
    void enqueueReadBuffer(WebCLBuffer*, bool, int, int, ArrayBufferView*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ArrayBufferView* ptr,
        WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueReadBuffer(buffer, blockingRead, offset, bufferSize, ptr, events, 0, ec);
    }
    void enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueReadBuffer(buffer, blockingRead, offset, bufferSize, ptr, 0, 0, ec);
    }

    void enqueueReadBufferRect(WebCLBuffer*, bool, Int32Array*, Int32Array*, Int32Array*, int, int, int, int, ArrayBufferView*,
        WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueReadBufferRect(WebCLBuffer* buffer, bool blockingRead, Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region,
        int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch, ArrayBufferView* ptr, WebCLEventList* eventWaitList, ExceptionCode& ec)
    {
        enqueueReadBufferRect(buffer, blockingRead, bufferOrigin, hostOrigin, region,
            bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, eventWaitList, 0, ec);
    }
    void enqueueReadBufferRect(WebCLBuffer* buffer, bool blockingRead,
        Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region, int bufferRowPitch,
        int bufferSlicePitch, int hostRowPitch, int hostSlicePitch, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueReadBufferRect(buffer, blockingRead, bufferOrigin, hostOrigin, region,
            bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, 0, 0, ec);
    }

    void enqueueNDRangeKernel(WebCLKernel*, Int32Array*, Int32Array*, Int32Array*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* offsets, Int32Array* globalWorkSize, Int32Array* localWorkSize, WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueNDRangeKernel(kernel, offsets, globalWorkSize, localWorkSize, events, 0, ec);
    }
    void enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* offsets, Int32Array* globalWorkSize, Int32Array* localWorkSize, ExceptionCode& ec)
    {
        enqueueNDRangeKernel(kernel, offsets, globalWorkSize, localWorkSize, 0, 0, ec);
    }

    void finish(ExceptionCode&);
    void flush(ExceptionCode&);

    void enqueueWriteImage(WebCLImage*, bool, Int32Array*, Int32Array*, int, int, ArrayBufferView*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueWriteImage(WebCLImage* image, bool blockingWrite, Int32Array* origin, Int32Array* region,
        int inputRowPitch, int inputSlicePitch, ArrayBufferView* ptr, WebCLEventList* eventWaitList, ExceptionCode& ec)
    {
        enqueueWriteImage(image, blockingWrite, origin, region, inputRowPitch, inputSlicePitch, ptr, eventWaitList, 0, ec);
    }
    void enqueueWriteImage(WebCLImage* image, bool blockingWrite, Int32Array* origin, Int32Array* region,
        int inputRowPitch, int inputSlicePitch, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueWriteImage(image, blockingWrite, origin, region, inputRowPitch, inputSlicePitch, ptr, 0, 0, ec);
    }

    void enqueueReadImage(WebCLImage*, bool, Int32Array*, Int32Array*, int, int, ArrayBufferView*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueReadImage(WebCLImage* image, bool blockingRead, Int32Array* origin, Int32Array* region, int rowPitch, int slicePitch, ArrayBufferView* ptr,
        WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueReadImage(image, blockingRead, origin, region, rowPitch, slicePitch, ptr, events, 0, ec);
    }
    void enqueueReadImage(WebCLImage* image, bool blockingRead, Int32Array* origin, Int32Array* region, int rowPitch, int slicePitch, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueReadImage(image, blockingRead, origin, region, rowPitch, slicePitch, ptr, 0, 0, ec);
    }

    void enqueueAcquireGLObjects(WebCLMemoryObject*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueAcquireGLObjects(WebCLMemoryObject* memoryList, WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueAcquireGLObjects(memoryList, events,  0, ec);
    }
    void enqueueAcquireGLObjects(WebCLMemoryObject* memoryList, ExceptionCode& ec)
    {
        enqueueAcquireGLObjects(memoryList, 0, 0, ec);
    }

    void enqueueReleaseGLObjects(WebCLMemoryObject*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueReleaseGLObjects(WebCLMemoryObject* memoryList, WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueReleaseGLObjects(memoryList, events, 0, ec);
    }
    void enqueueReleaseGLObjects(WebCLMemoryObject *memoryList, ExceptionCode& ec)
    {
        enqueueReleaseGLObjects(memoryList, 0, 0, ec);
    }

    void enqueueCopyBuffer(WebCLBuffer*, WebCLBuffer*, int, int, int, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueCopyBuffer(WebCLBuffer* srcBuffer, WebCLBuffer* dstBuffer, int srcOffset, int dstOffset, int sizeInBytes, WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueCopyBuffer(srcBuffer, dstBuffer, srcOffset, dstOffset, sizeInBytes, events, 0, ec);
    }
    void enqueueCopyBuffer(WebCLBuffer* srcBuffer, WebCLBuffer* dstBuffer, int srcOffset, int dstOffset, int sizeInBytes, ExceptionCode& ec)
    {
        enqueueCopyBuffer(srcBuffer, dstBuffer, srcOffset, dstOffset, sizeInBytes, 0, 0, ec);
    }

    void enqueueCopyBufferRect(WebCLBuffer*, WebCLBuffer*, Int32Array*, Int32Array*, Int32Array*, int, int, int, int, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueCopyBufferRect(WebCLBuffer* srcBuffer, WebCLBuffer* dstBuffer, Int32Array* srcOrigin, Int32Array* dstOrigin,
        Int32Array* region, int srcRowPitch, int srcSlicePitch, int dstRowPitch, int dstSlicePitch, WebCLEventList* eventWaitList, ExceptionCode& ec)
    {
        enqueueCopyBufferRect(srcBuffer, dstBuffer, srcOrigin, dstOrigin, region, srcRowPitch, srcSlicePitch, dstRowPitch, dstSlicePitch, eventWaitList, 0, ec);
    }
    void enqueueCopyBufferRect(WebCLBuffer* srcBuffer, WebCLBuffer* dstBuffer, Int32Array* srcOrigin, Int32Array* dstOrigin,
        Int32Array* region, int srcRowPitch, int srcSlicePitch, int dstRowPitch, int dstSlicePitch, ExceptionCode& ec)
    {
        enqueueCopyBufferRect(srcBuffer, dstBuffer, srcOrigin, dstOrigin, region, srcRowPitch, srcSlicePitch, dstRowPitch, dstSlicePitch, 0, 0, ec);
    }

    void enqueueCopyImage(WebCLImage*, WebCLImage*, Int32Array*, Int32Array*, Int32Array*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueCopyImage(WebCLImage* srcImage, WebCLImage* dstImage, Int32Array* srcOrigin, Int32Array* dstOrigin, Int32Array* region, WebCLEventList* events, ExceptionCode& ec)
    {
        enqueueCopyImage(srcImage, dstImage, srcOrigin, dstOrigin, region, events, 0, ec);
    }
    void enqueueCopyImage(WebCLImage* srcImage, WebCLImage* dstImage, Int32Array* srcOrigin, Int32Array* dstOrigin, Int32Array* region, ExceptionCode& ec)
    {
        enqueueCopyImage(srcImage, dstImage, srcOrigin, dstOrigin, region, 0, 0, ec);
    }

    void enqueueCopyImageToBuffer(WebCLImage*, WebCLBuffer*, Int32Array*, Int32Array*, int, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueCopyImageToBuffer(WebCLImage* srcImage, WebCLBuffer *dstBuffer, Int32Array* srcOrigin, Int32Array* region, int dstOffset,
        WebCLEventList* eventWaitList, ExceptionCode& ec)
    {
        enqueueCopyImageToBuffer(srcImage, dstBuffer, srcOrigin, region, dstOffset, eventWaitList, 0, ec);
    }
    void enqueueCopyImageToBuffer(WebCLImage* srcImage, WebCLBuffer *dstBuffer, Int32Array* srcOrigin, Int32Array* region, int dstOffset, ExceptionCode& ec)
    {
        enqueueCopyImageToBuffer(srcImage, dstBuffer, srcOrigin, region, dstOffset, 0, 0, ec);
    }

    void enqueueCopyBufferToImage(WebCLBuffer*, WebCLImage*, int, Int32Array*, Int32Array*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueCopyBufferToImage(WebCLBuffer *srcBuffer, WebCLImage *dstImage, int srcOffset, Int32Array* dstOrigin, Int32Array* region,
        WebCLEventList* eventWaitList, ExceptionCode& ec)
    {
        enqueueCopyBufferToImage(srcBuffer, dstImage, srcOffset, dstOrigin, region, eventWaitList, 0, ec);
    }
    void enqueueCopyBufferToImage(WebCLBuffer *srcBuffer, WebCLImage *dstImage, int srcOffset, Int32Array* dstOrigin, Int32Array* region, ExceptionCode& ec)
    {
        enqueueCopyBufferToImage(srcBuffer, dstImage, srcOffset, dstOrigin, region, 0, 0, ec);
    }

    void enqueueBarrier(WebCLEventList* eventsWaitList, WebCLEvent*, ExceptionCode&);
    void enqueueBarrier(WebCLEventList* eventsWaitList, ExceptionCode& ec)
    {
        enqueueBarrier(eventsWaitList, 0, ec);
    }
    void enqueueBarrier(ExceptionCode& ec)
    {
        enqueueBarrier(0, 0, ec);
    }

    void enqueueMarker(WebCLEventList* eventsWaitList, WebCLEvent*, ExceptionCode&);
    void enqueueMarker(WebCLEventList* eventsWaitList, ExceptionCode& ec)
    {
        enqueueMarker(eventsWaitList, 0, ec);
    }
    void enqueueMarker(ExceptionCode& ec)
    {
        enqueueMarker(0, 0, ec);
    }

    void enqueueTask(WebCLKernel*, WebCLEventList*, WebCLEvent*, ExceptionCode&);
    void enqueueTask(WebCLKernel* kernel, WebCLEventList* eventsWaitList, ExceptionCode& ec)
    {
        enqueueTask(kernel, eventsWaitList, 0, ec);
    }
    void enqueueTask(WebCLKernel* kernel, ExceptionCode& ec)
    {
        enqueueTask(kernel, 0, 0, ec);
    }

    cl_command_queue getCLCommandQueue();
private:
    WebCLCommandQueue(WebCLContext*, cl_command_queue);

    WebCLContext* m_context;
    cl_command_queue m_ccCommandQueue;
};

} // namespace WebCore
#endif // WebCLCommandQueue_h
