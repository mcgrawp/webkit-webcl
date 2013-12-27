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

#include "ComputeCommandQueue.h"
#include "WebCLObject.h"

namespace WebCore {

class ImageData;
class HTMLCanvasElement;
class HTMLImageElement;
class WebCLBuffer;
class WebCLContext;
class WebCLDevice;
class WebCLEvent;
class WebCLGetInfo;
class WebCLImage;
class WebCLKernel;
class WebCLMemoryObject;

typedef ComputeCommandQueue* ComputeCommandQueuePtr;
class WebCLCommandQueue : public WebCLObjectImpl<ComputeCommandQueuePtr> {
public:
    ~WebCLCommandQueue();
    static PassRefPtr<WebCLCommandQueue> create(WebCLContext*, CCenum queueProperties, WebCLDevice*, ExceptionCode&);
    WebCLGetInfo getInfo(CCenum, ExceptionCode&);

    void enqueueWriteBuffer(WebCLBuffer*, CCbool blockingWrite, CCuint bufferOffset, CCuint numBytes, ArrayBufferView*,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBuffer(WebCLBuffer*, CCbool blockingWrite, CCuint bufferOffset, ImageData*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBuffer(WebCLBuffer*, CCbool blockingWrite, CCuint bufferOffset, HTMLCanvasElement*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBuffer(WebCLBuffer*, CCbool blockingWrite, CCuint bufferOffset, HTMLImageElement*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueWriteBufferRect(WebCLBuffer*, CCbool blockingWrite, const Vector<CCuint>&, const Vector<CCuint>&, const Vector<CCuint>&,
        CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch, ArrayBufferView*,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBufferRect(WebCLBuffer*, CCbool blockingWrite, const Vector<CCuint>&, const Vector<CCuint>&, const Vector<CCuint>&,
        CCuint bufferRowPitch, CCuint bufferSlicePitch, ImageData*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBufferRect(WebCLBuffer*, CCbool blockingWrite, const Vector<CCuint>&, const Vector<CCuint>&, const Vector<CCuint>&,
        CCuint bufferRowPitch, CCuint bufferSlicePitch, HTMLCanvasElement*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteBufferRect(WebCLBuffer*, CCbool blockingWrite, const Vector<CCuint>&, const Vector<CCuint>&, const Vector<CCuint>&,
        CCuint bufferRowPitch, CCuint bufferSlicePitch, HTMLImageElement*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueReadBuffer(WebCLBuffer*, CCbool blockingWrite, CCuint bufferOffset, CCuint numBytes, ArrayBufferView*,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadBuffer(WebCLBuffer*, CCbool blockingWrite, CCuint bufferOffset, CCuint numBytes, HTMLCanvasElement*,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);


    void enqueueReadBufferRect(WebCLBuffer*, CCbool blockingWrite, const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin,
        const Vector<CCuint>& region, CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch,
        ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadBufferRect(WebCLBuffer*, CCbool blockingWrite, const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin,
        const Vector<CCuint>& region, CCuint bufferRowPitch, HTMLCanvasElement*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueReadImage(WebCLImage*, CCbool blockingWrite,  const Vector<CCuint>& origin, const Vector<CCuint>& region, CCuint rowPitch,
        ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadImage(WebCLImage*, CCbool blockingWrite,  const Vector<CCuint>& origin, const Vector<CCuint>& region, HTMLCanvasElement*,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueWriteImage(WebCLImage*, CCbool blockingWrite,  const Vector<unsigned>& origin, const Vector<unsigned>& region, CCuint hostRowPitch,
        ArrayBufferView*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteImage(WebCLImage*, CCbool blockingWrite,  const Vector<unsigned>& origin, const Vector<unsigned>& region,
        ImageData*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteImage(WebCLImage*, CCbool blockingWrite,  const Vector<unsigned>& origin, const Vector<unsigned>& region,
        HTMLCanvasElement*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteImage(WebCLImage*, CCbool blockingWrite,  const Vector<unsigned>& origin, const Vector<unsigned>& region,
        HTMLImageElement*, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyBuffer(WebCLBuffer*, WebCLBuffer*, CCuint sourceOffset, CCuint targetOffset, CCuint sizeInBytes,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyBufferRect(WebCLBuffer*, WebCLBuffer*, const Vector<unsigned>& sourceOrigin, const Vector<unsigned>& targetOrigin,
        const Vector<unsigned>& region, CCuint sourceRowPitch, CCuint sourceSlicePitch, CCuint targetRowPitch, CCuint targetSlicePitch,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyImage(WebCLImage*, WebCLImage*, const Vector<unsigned>& sourceOrigin, const Vector<unsigned>& targetOrigin,
        const Vector<unsigned>& region, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyImageToBuffer(WebCLImage*, WebCLBuffer*, const Vector<unsigned>& sourceOrigin, const Vector<unsigned>& region,
        CCuint targetOffset, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueCopyBufferToImage(WebCLBuffer*, WebCLImage*, CCuint sourceOffset, const Vector<unsigned>& sourceOrigin, const Vector<unsigned>& region,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void enqueueNDRangeKernel(WebCLKernel*, CCuint workDim, const Vector<unsigned>& globalWorkOffsets, const Vector<unsigned>& globalWorkSize,
        const Vector<unsigned>& localWorkSize, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void finish(ExceptionCode&);
    void flush(ExceptionCode&);

    void enqueueBarrier(ExceptionCode&);

    void enqueueMarker(WebCLEvent*, ExceptionCode&);

#if ENABLE(WEBGL)
    void enqueueAcquireGLObjects(const Vector<RefPtr<WebCLMemoryObject> >&, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReleaseGLObjects(const Vector<RefPtr<WebCLMemoryObject> >&, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
#endif

private:
    WebCLCommandQueue(WebCLContext*, ComputeCommandQueue*, WebCLDevice*);

    friend class WebCLEvent;

    void enqueueWriteBufferBase(WebCLBuffer*, CCbool blockingWrite, CCuint, CCuint, void* hostPtr, size_t hostPtrLength,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadBufferBase(WebCLBuffer*, CCbool blockingRead, CCuint, CCuint, void* hostPtr, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadBufferRectBase(WebCLBuffer*, CCbool blockingRead, const Vector<CCuint>&, const Vector<CCuint>&, const Vector<CCuint>&, CCuint, CCuint,
        CCuint, CCuint, void* hostPtr, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReadImageBase(WebCLImage*, CCbool blockingRead, const Vector<CCuint>&, const Vector<CCuint>&, CCuint, void* hostPtr, const Vector<RefPtr<WebCLEvent> >&,
        WebCLEvent*, ExceptionCode&);
    void enqueueWriteBufferRectBase(WebCLBuffer*, CCbool blockingWrite, const Vector<CCuint>& bufferOrigin, const Vector<CCuint>& hostOrigin,
        const Vector<CCuint>& region, CCuint bufferRowPitch, CCuint bufferSlicePitch, CCuint hostRowPitch, CCuint hostSlicePitch,
        void* hostPtr, size_t hostPtrLength, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueWriteImageBase(WebCLImage*, CCbool blockingWrite, const Vector<unsigned>&, const Vector<unsigned>&, CCuint hostRowPitch, void* hostPtr, size_t length,
        const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

    void releasePlatformObjectImpl();

    CCEvent* ccEventFromWebCLEvent(WebCLEvent*, ExceptionCode&);

    RefPtr<WebCLContext> m_context;
    RefPtr<WebCLDevice> m_device;
};

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCLCommandQueue_h
