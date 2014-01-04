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

#include "config.h"

#include "ComputeCommandQueue.h"

#include "ComputeContext.h"
#include "ComputeKernel.h"

namespace WebCore {

ComputeCommandQueue::ComputeCommandQueue(ComputeContext* context, CCDeviceID deviceId, CCCommandQueueProperties properties, CCerror& error)
{
    m_commandQueue = clCreateCommandQueue(context->context(), deviceId, properties, &error);
}

ComputeCommandQueue::~ComputeCommandQueue()
{
    clReleaseCommandQueue(m_commandQueue);
}

CCerror ComputeCommandQueue::enqueueNDRangeKernel(ComputeKernel* kernel, CCuint workItemDimensions,
    const Vector<size_t>& globalWorkOffset, const Vector<size_t>& globalWorkSize, const Vector<size_t>& localWorkSize,
    const Vector<CCEvent>& eventWaitList, CCEvent* event)
{
    return clEnqueueNDRangeKernel(m_commandQueue, kernel->kernel(), workItemDimensions,
        globalWorkOffset.data(), globalWorkSize.data(), localWorkSize.data(), eventWaitList.size(), eventWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueBarrier()
{
    CCint error;
#if defined(CL_VERSION_1_2)
    error = clEnqueueBarrierWithWaitList(m_commandQueue, 0 /*eventsWaitListLength*/, 0 /*eventsWaitList*/, 0 /*event*/);
#else
    error = clEnqueueBarrier(m_commandQueue);
#endif
    return error;
}

CCerror ComputeCommandQueue::enqueueMarker(CCEvent* event)
{
    CCint error;
#if defined(CL_VERSION_1_2)
    error = clEnqueueMarkerWithWaitList(m_commandQueue, 0 /*eventsWaitListLength*/, 0 /*eventsWaitList*/, event);
#else
    error = clEnqueueMarker(m_commandQueue, event);
#endif
    return error;
}

CCerror ComputeCommandQueue::enqueueTask(CCKernel kernelID, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueTask(m_commandQueue, kernelID, eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueWriteBuffer(PlatformComputeObject buffer, CCbool blockingWrite,
    size_t offset, size_t bufferSize, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueWriteBuffer(m_commandQueue, buffer, blockingWrite, offset,
        bufferSize, baseAddress, eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueWriteBufferRect(PlatformComputeObject buffer, CCbool blockingWrite,
    const Vector<size_t>& bufferOriginArray, const Vector<size_t>& hostOriginArray, const Vector<size_t>& regionArray, size_t bufferRowPitch,
    size_t bufferSlicePitch, size_t hostRowPitch, size_t hostSlicePitch, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueWriteBufferRect(m_commandQueue, buffer, blockingWrite, bufferOriginArray.data(), hostOriginArray.data(), regionArray.data(),
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, baseAddress, eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueReadBuffer(PlatformComputeObject buffer, CCbool blockingRead, size_t offset, size_t bufferSize, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueReadBuffer(m_commandQueue, buffer, blockingRead, offset,
        bufferSize, baseAddress, eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueReadBufferRect(PlatformComputeObject buffer, CCbool blockingRead,
    const Vector<size_t>& bufferOriginArray, const Vector<size_t>& hostOriginArray, const Vector<size_t>& regionArray, size_t bufferRowPitch,
    size_t bufferSlicePitch, size_t hostRowPitch, size_t hostSlicePitch, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueReadBufferRect(m_commandQueue, buffer, blockingRead, bufferOriginArray.data(), hostOriginArray.data(), regionArray.data(),
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, baseAddress, eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueReadImage(PlatformComputeObject image, CCbool blockingRead, const Vector<size_t>& originArray,
    const Vector<size_t>& regionArray, size_t rowPitch, size_t slicePitch, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueReadImage(m_commandQueue, image, blockingRead, originArray.data(), regionArray.data(), rowPitch, slicePitch, baseAddress,
        eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueWriteImage(PlatformComputeObject image, CCbool blockingWrite, const Vector<size_t>& originArray,
    const Vector<size_t>& regionArray, size_t rowPitch, size_t slicePitch, void* baseAddress, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueWriteImage(m_commandQueue, image, blockingWrite, originArray.data(), regionArray.data(), rowPitch, slicePitch, baseAddress,
        eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueAcquireGLObjects(const Vector<PlatformComputeObject>& objects, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueAcquireGLObjects(m_commandQueue, objects.size(), objects.data(), eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueReleaseGLObjects(const Vector<PlatformComputeObject>& objects, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueReleaseGLObjects(m_commandQueue, objects.size(), objects.data(), eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueCopyImage(PlatformComputeObject originImage, PlatformComputeObject targetImage,
    const Vector<size_t>& sourceOriginArray, const Vector<size_t>& targetOriginArray, const Vector<size_t>& regionArray,
    const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueCopyImage(m_commandQueue, originImage, targetImage, sourceOriginArray.data(), targetOriginArray.data(), regionArray.data(),
        eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueCopyImageToBuffer(PlatformComputeObject sourceImage, PlatformComputeObject targetBuffer,
    const Vector<size_t>& sourceOriginArray, const Vector<size_t>& regionArray, size_t targetOffset, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueCopyImageToBuffer(m_commandQueue, sourceImage, targetBuffer, sourceOriginArray.data(), regionArray.data(), targetOffset,
        eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueCopyBufferToImage(PlatformComputeObject sourceBuffer, PlatformComputeObject targetImage,
    size_t srcOffset, const Vector<size_t>& targetOriginArray, const Vector<size_t>& regionArray, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueCopyBufferToImage(m_commandQueue, sourceBuffer, targetImage, srcOffset, targetOriginArray.data(), regionArray.data(),
        eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueCopyBuffer(PlatformComputeObject sourceBuffer, PlatformComputeObject targetBuffer,
    size_t sourceOffset, size_t targetOffset, size_t sizeInBytes, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueCopyBuffer(m_commandQueue, sourceBuffer, targetBuffer, sourceOffset, targetOffset, sizeInBytes,
        eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::enqueueCopyBufferRect(PlatformComputeObject sourceBuffer, PlatformComputeObject targetBuffer,
    const Vector<size_t>& sourceOriginArray, const Vector<size_t>& targetOriginArray, const Vector<size_t>& regionArray, size_t sourceRowPitch,
    size_t sourceSlicePitch, size_t targetRowPitch, size_t targetSlicePitch, const Vector<CCEvent>& eventsWaitList, CCEvent* event)
{
    return clEnqueueCopyBufferRect(m_commandQueue, sourceBuffer, targetBuffer, sourceOriginArray.data(), targetOriginArray.data(), regionArray.data(),
        sourceRowPitch, sourceSlicePitch, targetRowPitch, targetSlicePitch, eventsWaitList.size(), eventsWaitList.data(), event);
}

CCerror ComputeCommandQueue::releaseCommandQueue()
{
    return clReleaseCommandQueue(m_commandQueue);
}

CCerror ComputeCommandQueue::finish()
{
    return clFinish(m_commandQueue);
}

CCerror ComputeCommandQueue::flush()
{
    return clFlush(m_commandQueue);
}

CCerror ComputeCommandQueue::getCommandQueueInfoBase(CCCommandQueue queue, CCCommandQueueInfoType infoType, size_t sizeOfData, void* data, size_t* retSize)
{
    return clGetCommandQueueInfo(queue, infoType, sizeOfData, data, retSize);
}


}
