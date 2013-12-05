/*
 * Copyright (C) 2013 Samsung Electronics Corporation. All rights reserved.
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

#ifndef ComputeCommandQueue_h
#define ComputeCommandQueue_h

#include "ComputeTypes.h"
#include "ComputeTypesTraits.h"

#include <wtf/PassRefPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

class ComputeContext;

class ComputeCommandQueue {
public:
    ComputeCommandQueue(ComputeContext*, CCDeviceID, CCCommandQueueProperties, CCerror&);
    ~ComputeCommandQueue();

    CCerror enqueueNDRangeKernel(CCKernel, CCuint globalWorkItemDimensions, const Vector<size_t>& globalWorkOffset,
        const Vector<size_t>& globalWorkSize, const Vector<size_t>& localWorkSize, const Vector<CCEvent>& eventsWaitList, CCEvent*);

    CCerror enqueueBarrier();
    CCerror enqueueMarker(CCEvent*);
    CCerror enqueueTask(CCKernel, const Vector<CCEvent>& eventsWaitList, CCEvent*);

    CCerror enqueueWriteBuffer(PlatformComputeObject buffer, CCbool blockingWrite, size_t offset, size_t bufferSize,
        void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueWriteBufferRect(PlatformComputeObject, CCbool blockingWrite, const Vector<size_t>& bufferOriginArray,
        const Vector<size_t>& hostOriginArray, const Vector<size_t>& regionArray, size_t bufferRowPitch, size_t bufferSlicePitch, size_t hostRowPitch,
        size_t hostSlicePitch, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueReadBuffer(PlatformComputeObject buffer, CCbool blockingRead, size_t offset, size_t bufferSize,
        void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueReadBufferRect(PlatformComputeObject buffer, CCbool blockingRead, const Vector<size_t>& bufferOriginArray,
        const Vector<size_t>& hostOriginArray, const Vector<size_t>& regionArray, size_t bufferRowPitch, size_t bufferSlicePitch, size_t hostRowPitch,
        size_t hostSlicePitch, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueReadImage(PlatformComputeObject image, CCbool blockingRead, const Vector<size_t>& originArray, const Vector<size_t>& regionArray,
        size_t rowPitch, size_t slicePitch, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueWriteImage(PlatformComputeObject image, CCbool blockingWrite, const Vector<size_t>& originArray, const Vector<size_t>& regionArray,
        size_t rowPitch, size_t slicePitch, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueAcquireGLObjects(const Vector<PlatformComputeObject>&, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueReleaseGLObjects(const Vector<PlatformComputeObject>&, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueCopyImage(PlatformComputeObject originImage, PlatformComputeObject targetImage, const Vector<size_t>& sourceOriginArray,
        const Vector<size_t>& targetOriginArray, const Vector<size_t>& regionArray, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueCopyImageToBuffer(PlatformComputeObject sourceImage, PlatformComputeObject targetBuffer,
        const Vector<size_t>& sourceOriginArray, const Vector<size_t>& regionArray, size_t targetOffset, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueCopyBufferToImage(PlatformComputeObject sourceBuffer, PlatformComputeObject targetImage, size_t srcOffset,
        const Vector<size_t>& targetOriginArray, const Vector<size_t>& regionArray, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueCopyBuffer(PlatformComputeObject sourceBuffer, PlatformComputeObject targetBuffer,
        size_t sourceOffset, size_t targetOffset, size_t sizeInBytes, const Vector<CCEvent>& eventsWaitList, CCEvent*);
    CCerror enqueueCopyBufferRect(PlatformComputeObject sourceBuffer, PlatformComputeObject targetBuffer,
        const Vector<size_t>& sourceOriginArray, const Vector<size_t>& targetOriginArray, const Vector<size_t>& regionArray, size_t sourceRowPitch, size_t sourceSlicePitch,
        size_t targetRowPitch, size_t targetSlicePitch, const Vector<CCEvent>& eventsWaitList, CCEvent*);

    template <typename T>
    CCerror commandQueueInfo(CCCommandQueueInfoType infoType, T* data)
    {
        return getInfoHelper(ComputeCommandQueue::getCommandQueueInfoBase, m_commandQueue, infoType, data);
    }

    CCerror finish();
    CCerror flush();

    CCerror releaseCommandQueue();
private:
    static CCerror getCommandQueueInfoBase(CCCommandQueue, CCCommandQueueInfoType, size_t, void *data, size_t* actualSize);

    CCCommandQueue m_commandQueue;
};

}

#endif
