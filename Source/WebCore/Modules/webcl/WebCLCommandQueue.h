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
class WebCLCommandQueue : public RefCounted<WebCLCommandQueue> {
public:
    ~WebCLCommandQueue();
    static PassRefPtr<WebCLCommandQueue> create(WebCLContext*, cl_command_queue);
    WebCLGetInfo getInfo(int, ExceptionCode&);

    void enqueueWriteBuffer(WebCLBuffer*, bool, int, int, ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBuffer(WebCLBuffer* webCLBuffer, bool blockingWrite, int offset, int sizeInBytes, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueWriteBuffer(webCLBuffer, blockingWrite, offset, sizeInBytes, ptr, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }
    void enqueueWriteBuffer(WebCLBuffer* webCLBuffer, bool blockingWrite, int offset, int sizeInBytes, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueWriteBuffer(webCLBuffer, blockingWrite, offset, sizeInBytes, ptr, events, 0, ec);
    }

    void enqueueWriteBuffer(WebCLBuffer*, bool, int, int, ImageData*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBuffer(WebCLBuffer* webCLBuffer, bool blockingWrite, int offset, int sizeInBytes, ImageData* ptr, ExceptionCode& ec)
    {
        enqueueWriteBuffer(webCLBuffer, blockingWrite, offset, sizeInBytes, ptr, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }
    void enqueueWriteBuffer(WebCLBuffer* webCLBuffer, bool blockingWrite, int offset, int sizeInBytes, ImageData* ptr, const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueWriteBuffer(webCLBuffer, blockingWrite, offset, sizeInBytes, ptr, events, 0, ec);
    }

    void enqueueWriteBufferRect(WebCLBuffer*, bool, Int32Array*, Int32Array*, Int32Array*, int, int, int, int,
        ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBufferRect(WebCLBuffer* buffer, bool blockingWrite, Int32Array* bufferOrigin, Int32Array*
        hostOrigin, Int32Array* region, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch,
        ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& eventWaitList, ExceptionCode& ec)
    {
        enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, hostOrigin, region,
            bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, eventWaitList, 0, ec);
    }
    void enqueueWriteBufferRect(WebCLBuffer* buffer, bool blockingWrite, Int32Array* bufferOrigin, Int32Array*
        hostOrigin, Int32Array* region, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch,
        ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueWriteBufferRect(buffer, blockingWrite, bufferOrigin, hostOrigin, region,
            bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }
    void enqueueWriteBufferRect(WebCLBuffer* mem, bool blockingWrite, int offset, int bufferSize, ImageData* ptr,
        ExceptionCode&);

    void enqueueReadBuffer(WebCLBuffer*, bool, int, int, ImageData*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ImageData* ptr,
        Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueReadBuffer(buffer, blockingRead, offset, bufferSize, ptr, events, 0, ec);
    }
    void enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ImageData* ptr,
        ExceptionCode& ec)
    {
        enqueueReadBuffer(buffer, blockingRead, offset, bufferSize, ptr, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueReadBuffer(WebCLBuffer*, bool, int, int, ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ArrayBufferView* ptr,
        const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueReadBuffer(buffer, blockingRead, offset, bufferSize, ptr, events, 0, ec);
    }
    void enqueueReadBuffer(WebCLBuffer* buffer, bool blockingRead, int offset, int bufferSize, ArrayBufferView* ptr,
        ExceptionCode& ec)
    {
        enqueueReadBuffer(buffer, blockingRead, offset, bufferSize, ptr, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueReadBufferRect(WebCLBuffer*, bool, Int32Array*, Int32Array*, Int32Array*, int, int, int, int,
        ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadBufferRect(WebCLBuffer* buffer, bool blockingRead, Int32Array* bufferOrigin, Int32Array* hostOrigin,
        Int32Array* region, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch,
        ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& eventWaitList, ExceptionCode& ec)
    {
        enqueueReadBufferRect(buffer, blockingRead, bufferOrigin, hostOrigin, region,
            bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, eventWaitList, 0, ec);
    }
    void enqueueReadBufferRect(WebCLBuffer* buffer, bool blockingRead,
        Int32Array* bufferOrigin, Int32Array* hostOrigin, Int32Array* region, int bufferRowPitch,
        int bufferSlicePitch, int hostRowPitch, int hostSlicePitch, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueReadBufferRect(buffer, blockingRead, bufferOrigin, hostOrigin, region,
            bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, ptr, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueNDRangeKernel(WebCLKernel*, Int32Array*, Int32Array*, Int32Array*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* offsets, Int32Array* globalWorkSize, Int32Array*
        localWorkSize, const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueNDRangeKernel(kernel, offsets, globalWorkSize, localWorkSize, events, 0, ec);
    }
    void enqueueNDRangeKernel(WebCLKernel* kernel, Int32Array* offsets, Int32Array* globalWorkSize, Int32Array*
        localWorkSize, ExceptionCode& ec)
    {
        enqueueNDRangeKernel(kernel, offsets, globalWorkSize, localWorkSize, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void finish(ExceptionCode&);
    void flush(ExceptionCode&);

    void enqueueWriteImage(WebCLImage*, bool, Int32Array*, Int32Array*, int, int, ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&,
        WebCLEvent*, ExceptionCode&);
    void enqueueWriteImage(WebCLImage* image, bool blockingWrite, Int32Array* origin, Int32Array* region,
        int inputRowPitch, int inputSlicePitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& eventWaitList, ExceptionCode& ec)
    {
        enqueueWriteImage(image, blockingWrite, origin, region, inputRowPitch, inputSlicePitch, ptr, eventWaitList, 0, ec);
    }
    void enqueueWriteImage(WebCLImage* image, bool blockingWrite, Int32Array* origin, Int32Array* region,
        int inputRowPitch, int inputSlicePitch, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueWriteImage(image, blockingWrite, origin, region, inputRowPitch, inputSlicePitch, ptr, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueReadImage(WebCLImage*, bool, Int32Array*, Int32Array*, int, int, ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&,
        WebCLEvent*, ExceptionCode&);
    void enqueueReadImage(WebCLImage* image, bool blockingRead, Int32Array* origin, Int32Array* region, int rowPitch,
        int slicePitch, ArrayBufferView* ptr, const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueReadImage(image, blockingRead, origin, region, rowPitch, slicePitch, ptr, events, 0, ec);
    }
    void enqueueReadImage(WebCLImage* image, bool blockingRead, Int32Array* origin, Int32Array* region, int rowPitch,
        int slicePitch, ArrayBufferView* ptr, ExceptionCode& ec)
    {
        enqueueReadImage(image, blockingRead, origin, region, rowPitch, slicePitch, ptr, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueAcquireGLObjects(WebCLMemoryObject*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueAcquireGLObjects(WebCLMemoryObject* memoryList, const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueAcquireGLObjects(memoryList, events,  0, ec);
    }
    void enqueueAcquireGLObjects(WebCLMemoryObject* memoryList, ExceptionCode& ec)
    {
        enqueueAcquireGLObjects(memoryList, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueReleaseGLObjects(WebCLMemoryObject*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReleaseGLObjects(WebCLMemoryObject* memoryList, const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueReleaseGLObjects(memoryList, events, 0, ec);
    }
    void enqueueReleaseGLObjects(WebCLMemoryObject *memoryList, ExceptionCode& ec)
    {
        enqueueReleaseGLObjects(memoryList, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueCopyBuffer(WebCLBuffer*, WebCLBuffer*, int, int, int, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueCopyBuffer(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, int sourceOffset, int targetOffset,
        int sizeInBytes, const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueCopyBuffer(sourceBuffer, targetBuffer, sourceOffset, targetOffset, sizeInBytes, events, 0, ec);
    }
    void enqueueCopyBuffer(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, int sourceOffset, int targetOffset,
        int sizeInBytes, ExceptionCode& ec)
    {
        enqueueCopyBuffer(sourceBuffer, targetBuffer, sourceOffset, targetOffset, sizeInBytes, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueCopyBufferRect(WebCLBuffer*, WebCLBuffer*, Int32Array*, Int32Array*, Int32Array*, int, int, int, int,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueCopyBufferRect(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, Int32Array* sourceOrigin,
        Int32Array* targetOrigin, Int32Array* region, int sourceRowPitch, int sourceSlicePitch, int targetRowPitch,
        int targetSlicePitch, const Vector<RefPtr<WebCLEvent> >& eventWaitList, ExceptionCode& ec)
    {
        enqueueCopyBufferRect(sourceBuffer, targetBuffer, sourceOrigin, targetOrigin, region, sourceRowPitch,
            sourceSlicePitch, targetRowPitch, targetSlicePitch, eventWaitList, 0, ec);
    }
    void enqueueCopyBufferRect(WebCLBuffer* sourceBuffer, WebCLBuffer* targetBuffer, Int32Array* sourceOrigin,
        Int32Array* targetOrigin, Int32Array* region, int sourceRowPitch, int sourceSlicePitch, int targetRowPitch,
        int targetSlicePitch, ExceptionCode& ec)
    {
        enqueueCopyBufferRect(sourceBuffer, targetBuffer, sourceOrigin, targetOrigin, region, sourceRowPitch,
            sourceSlicePitch, targetRowPitch, targetSlicePitch, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueCopyImage(WebCLImage*, WebCLImage*, Int32Array*, Int32Array*, Int32Array*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*,
        ExceptionCode&);
    void enqueueCopyImage(WebCLImage* sourceImage, WebCLImage* targetImage, Int32Array* sourceOrigin, Int32Array*
        targetOrigin, Int32Array* region, const Vector<RefPtr<WebCLEvent> >& events, ExceptionCode& ec)
    {
        enqueueCopyImage(sourceImage, targetImage, sourceOrigin, targetOrigin, region, events, 0, ec);
    }
    void enqueueCopyImage(WebCLImage* sourceImage, WebCLImage* targetImage, Int32Array* sourceOrigin, Int32Array*
        targetOrigin, Int32Array* region, ExceptionCode& ec)
    {
        enqueueCopyImage(sourceImage, targetImage, sourceOrigin, targetOrigin, region, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueCopyImageToBuffer(WebCLImage*, WebCLBuffer*, Int32Array*, Int32Array*, int, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*,
        ExceptionCode&);
    void enqueueCopyImageToBuffer(WebCLImage* sourceImage, WebCLBuffer *targetBuffer, Int32Array* sourceOrigin,
        Int32Array* region, int targetOffset, const Vector<RefPtr<WebCLEvent> >& eventWaitList, ExceptionCode& ec)
    {
        enqueueCopyImageToBuffer(sourceImage, targetBuffer, sourceOrigin, region, targetOffset, eventWaitList, 0, ec);
    }
    void enqueueCopyImageToBuffer(WebCLImage* sourceImage, WebCLBuffer *targetBuffer, Int32Array* sourceOrigin,
        Int32Array* region, int targetOffset, ExceptionCode& ec)
    {
        enqueueCopyImageToBuffer(sourceImage, targetBuffer, sourceOrigin, region, targetOffset, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueCopyBufferToImage(WebCLBuffer*, WebCLImage*, int, Int32Array*, Int32Array*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*,
        ExceptionCode&);
    void enqueueCopyBufferToImage(WebCLBuffer *sourceBuffer, WebCLImage *targetImage, int sourceOffset, Int32Array*
        targetOrigin, Int32Array* region, const Vector<RefPtr<WebCLEvent> >& eventWaitList, ExceptionCode& ec)
    {
        enqueueCopyBufferToImage(sourceBuffer, targetImage, sourceOffset, targetOrigin, region, eventWaitList, 0, ec);
    }
    void enqueueCopyBufferToImage(WebCLBuffer *sourceBuffer, WebCLImage *targetImage, int sourceOffset, Int32Array*
        targetOrigin, Int32Array* region, ExceptionCode& ec)
    {
        enqueueCopyBufferToImage(sourceBuffer, targetImage, sourceOffset, targetOrigin, region, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

    void enqueueBarrier(ExceptionCode&);

    void enqueueMarker(WebCLEvent*, ExceptionCode&);
    void enqueueMarker(ExceptionCode& ec)
    {
        enqueueMarker(0, ec);
    }

    void enqueueTask(WebCLKernel*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueTask(WebCLKernel* kernel, const Vector<RefPtr<WebCLEvent> >& eventsWaitList, ExceptionCode& ec)
    {
        enqueueTask(kernel, eventsWaitList, 0, ec);
    }
    void enqueueTask(WebCLKernel* kernel, ExceptionCode& ec)
    {
        enqueueTask(kernel, Vector<RefPtr<WebCLEvent> >(), 0, ec);
    }

private:
    WebCLCommandQueue(WebCLContext*, cl_command_queue);

    void enqueueWriteBufferBase(WebCLBuffer*, bool, int, int, void*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    WebCLContext* m_context;
    cl_command_queue m_ccCommandQueue;
};

} // namespace WebCore
#endif // WebCLCommandQueue_h
