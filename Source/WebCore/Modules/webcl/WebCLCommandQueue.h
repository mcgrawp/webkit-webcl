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

#ifndef WebCLCommandQueue_h
#define WebCLCommandQueue_h

#if ENABLE(WEBCL)

#include "HTMLCanvasElement.h"
#include "ImageData.h"
#include "WebCLBuffer.h"
#include "WebCLContext.h"
#include "WebCLEvent.h"
#include "WebCLFinishCallback.h"
#include "WebCLGetInfo.h"
#include "WebCLImage.h"
#include "WebCLProgram.h"

#include <wtf/ArrayBufferView.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Uint8ClampedArray.h>
#include <wtf/Vector.h>

namespace WebCore {

class WebCLContext;
class WebCLKernel;

class WebCLCommandQueue : public WebCLObject<CCCommandQueue> {
public:
    ~WebCLCommandQueue();
    static PassRefPtr<WebCLCommandQueue> create(WebCLContext*, int queueProperties, const RefPtr<WebCLDevice>&, ExceptionCode&);
    WebCLGetInfo getInfo(int, ExceptionCode&);

    void enqueueWriteBuffer(WebCLBuffer*, bool blockingWrite, int bufferOffset, int numBytes, ArrayBufferView*,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBuffer(WebCLBuffer*, bool blockingWrite, int bufferOffset, int numBytes, ImageData*,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueWriteBufferRect(WebCLBuffer*, bool blockingWrite, Int32Array* bufferOrigin, Int32Array* hostOrigin,
        Int32Array* region, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch,
        ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueReadBuffer(WebCLBuffer*, bool blockingWrite, int bufferOffset, int numBytes, ImageData*,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadBuffer(WebCLBuffer*, bool blockingWrite, int bufferOffset, int numBytes, ArrayBufferView*,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueReadBufferRect(WebCLBuffer*, bool blockingWrite, Int32Array* bufferOrigin, Int32Array* hostOrigin,
        Int32Array* region, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch, int hostSlicePitch,
        ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueNDRangeKernel(WebCLKernel*, Int32Array* globalWorkOffsets, Int32Array* globalWorkSize,
        Int32Array* localWorkSize, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void finish(ExceptionCode&);
    void flush(ExceptionCode&);

    void enqueueWriteImage(WebCLImage*, bool blockingWrite,  Int32Array* origin, Int32Array* region, int inputRowPitch,
        ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueReadImage(WebCLImage*, bool blockingWrite,  Int32Array* origin, Int32Array* region, int rowPitch,
        ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyBuffer(WebCLBuffer*, WebCLBuffer*, int sourceOffset, int targetOffset, int sizeInBytes,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyBufferRect(WebCLBuffer*, WebCLBuffer*, Int32Array* sourceOrigin, Int32Array* targetOrigin,
        Int32Array* region, int sourceRowPitch, int sourceSlicePitch, int targetRowPitch, int targetSlicePitch,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyImage(WebCLImage*, WebCLImage*, Int32Array* sourceOrigin, Int32Array* targetOrigin,
        Int32Array* region, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyImageToBuffer(WebCLImage*, WebCLBuffer*, Int32Array* sourceOrigin, Int32Array* region,
        int targetOffset, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyBufferToImage(WebCLBuffer*, WebCLImage*, int sourceOffset, Int32Array* sourceOrigin, Int32Array* region,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueBarrier(ExceptionCode&);

    void enqueueMarker(WebCLEvent*, ExceptionCode&);

protected:
    WebCLCommandQueue(WebCLContext*, const RefPtr<WebCLDevice>&, CCCommandQueue);

    template <class T>
    static void createBase(WebCLContext*, int commandQueueProperty, const RefPtr<WebCLDevice>&, ExceptionCode&, RefPtr<T>& out);

    WebCLContext* m_context;

private:
    friend class WebCLEvent;
    static CCCommandQueue createBaseInternal(WebCLContext*, int commandQueueProperty, const RefPtr<WebCLDevice>&, CCerror&);

    void enqueueWriteBufferBase(WebCLBuffer*, bool blockingWrite, int, int, void*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void releasePlatformObjectImpl();

    const RefPtr<WebCLDevice> m_device;
};

template <class T>
inline void WebCLCommandQueue::createBase(WebCLContext* context, int commandQueueProperty, const RefPtr<WebCLDevice>& webCLDevice, ExceptionCode& ec, RefPtr<T>& out)
{
    out = 0;

    CCerror error = ComputeContext::SUCCESS;
    CCCommandQueue clCommandQueue = createBaseInternal(context, commandQueueProperty, webCLDevice, error);
    if (!clCommandQueue) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return;
    }

    out = adoptRef(new T(context, webCLDevice, clCommandQueue));
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCLCommandQueue_h
