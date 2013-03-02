/*
 * Copyright (C) 2012, 2013 Samsung Electronics Corporation. All rights reserved.
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

#include "ComputeContext.h"

#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

static CCerror clToComputeContextError(cl_int clError)
{
    int computeContextError = CL_INVALID_VALUE;
    switch (clError) {
    case CL_SUCCESS:
        computeContextError = ComputeContext::SUCCESS;
        break;
    case CL_INVALID_PLATFORM:
        computeContextError = ComputeContext::INVALID_PLATFORM;
        break;
    case CL_INVALID_DEVICE:
        computeContextError = ComputeContext::INVALID_DEVICE;
        break;
    case CL_INVALID_OPERATION:
        computeContextError = ComputeContext::INVALID_OPERATION;
        break;
    case CL_DEVICE_NOT_AVAILABLE:
        computeContextError = ComputeContext::DEVICE_NOT_AVAILABLE;
        break;
    case CL_OUT_OF_RESOURCES:
        computeContextError = ComputeContext::OUT_OF_RESOURCES;
        break;
    case CL_OUT_OF_HOST_MEMORY:
        computeContextError = ComputeContext::OUT_OF_HOST_MEMORY;
        break;
    case CL_INVALID_CONTEXT:
        computeContextError = ComputeContext::INVALID_CONTEXT;
        break;
    case CL_INVALID_VALUE:
        computeContextError = ComputeContext::INVALID_VALUE;
        break;
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        computeContextError = ComputeContext::INVALID_IMAGE_FORMAT_DESCRIPTOR;
        break;
    case CL_INVALID_IMAGE_SIZE:
        computeContextError = ComputeContext::INVALID_IMAGE_SIZE;
        break;
    case CL_INVALID_HOST_PTR:
        computeContextError = ComputeContext::INVALID_HOST_PTR;
        break;
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        computeContextError = ComputeContext::IMAGE_FORMAT_NOT_SUPPORTED;
        break;
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        computeContextError = ComputeContext::MEM_OBJECT_ALLOCATION_FAILURE;
        break;
    case CL_INVALID_PROGRAM:
        computeContextError = ComputeContext::INVALID_PROGRAM;
        break;
    case CL_INVALID_PROGRAM_EXECUTABLE:
        computeContextError = ComputeContext::INVALID_PROGRAM_EXECUTABLE;
        break;
    case CL_INVALID_KERNEL_NAME:
        computeContextError = ComputeContext::INVALID_KERNEL_NAME;
        break;
    case CL_INVALID_KERNEL_DEFINITION:
        computeContextError = ComputeContext::INVALID_KERNEL_DEFINITION;
        break;
    case CL_INVALID_EVENT:
        computeContextError = ComputeContext::INVALID_EVENT;
    default:
        ASSERT_NOT_REACHED();
        break;
    }
    return computeContextError;
}

// FIXME: Remove it when the CL<->GL interoperability turns to be an extension.
static cl_mem_flags computeMemoryTypeToCL(int memoryType)
{
    cl_int clMemoryType = CL_INVALID_VALUE;
    switch (memoryType) {
    case ComputeContext::MEM_READ_ONLY:
        clMemoryType = CL_MEM_READ_ONLY;
        break;
    case ComputeContext::MEM_WRITE_ONLY:
        clMemoryType = CL_MEM_WRITE_ONLY;
        break;
    case ComputeContext::MEM_READ_WRITE:
        clMemoryType = CL_MEM_READ_WRITE;
        break;
    case ComputeContext::MEM_USE_HOST_PTR:
        clMemoryType = CL_MEM_USE_HOST_PTR;
        break;
    case ComputeContext::MEM_ALLOC_HOST_PTR:
        clMemoryType = CL_MEM_ALLOC_HOST_PTR;
        break;
    case ComputeContext::MEM_COPY_HOST_PTR:
        clMemoryType = CL_MEM_COPY_HOST_PTR;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clMemoryType;
}

ComputeContext::ComputeContext(CCContextProperties* contextProperties, const Vector<CCDeviceID>& devices, CCerror& error)
{
    cl_int clError;
    m_clContext = clCreateContext(contextProperties, devices.size(), devices.data(), 0, 0, &clError);
    error = clToComputeContextError(clError);
}

ComputeContext::ComputeContext(CCContextProperties* contextProperties, unsigned deviceType, CCerror& error)
{
    cl_int clError;

    switch (deviceType) {
    case DEVICE_TYPE_GPU:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, 0, 0, &clError);
        break;
    case DEVICE_TYPE_CPU:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, 0, 0, &clError);
        break;
    case DEVICE_TYPE_ACCELERATOR:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ACCELERATOR, 0, 0, &clError);
        break;
    case DEVICE_TYPE_DEFAULT:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_DEFAULT, 0, 0, &clError);
        break;
    case DEVICE_TYPE_ALL:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ALL, 0, 0, &clError);
        break;
    default:
        clError = CL_INVALID_DEVICE_TYPE;
        break;
    }

    error = clToComputeContextError(clError);
}

ComputeContext::~ComputeContext()
{
    clReleaseContext(m_clContext);
}

PassRefPtr<ComputeContext> ComputeContext::create(CCContextProperties* contextProperties, const Vector<CCDeviceID>& devices, CCerror& error)
{
    RefPtr<ComputeContext> computeContext =  adoptRef(new ComputeContext(contextProperties, devices, error));
    if (!computeContext) {
        ASSERT(error != ComputeContext::SUCCESS);
        return 0;
    }
    return computeContext.release();
}

PassRefPtr<ComputeContext> ComputeContext::create(CCContextProperties* contextProperties, unsigned deviceType, CCerror& error)
{
    RefPtr<ComputeContext> computeContext = adoptRef(new ComputeContext(contextProperties, deviceType, error));
    if (!computeContext) {
        ASSERT(error != ComputeContext::SUCCESS);
        return 0;
    }
    return computeContext.release();
}

CCerror ComputeContext::getPlatformIDs(Vector<CCPlatformID>& ccPlatforms)
{
    cl_uint numberOfPlatforms;
    cl_int clError = clGetPlatformIDs(0, 0, &numberOfPlatforms);
    if (clError != CL_SUCCESS)
        return clToComputeContextError(clError);

    if (!ccPlatforms.tryReserveCapacity(numberOfPlatforms))
        return OUT_OF_HOST_MEMORY;
    ccPlatforms.resize(numberOfPlatforms);

    clError = clGetPlatformIDs(numberOfPlatforms, ccPlatforms.data(), 0);
    return clToComputeContextError(clError);
}

CCerror ComputeContext::getDeviceIDs(CCPlatformID platform, CCDeviceType deviceType, Vector<CCDeviceID>& ccDevices)
{
    cl_uint numberOfDevices;
    cl_int clError = clGetDeviceIDs(platform, deviceType, 0, 0, &numberOfDevices);
    if (clError != CL_SUCCESS)
        return clToComputeContextError(clError);

    if (!ccDevices.tryReserveCapacity(numberOfDevices))
        return OUT_OF_HOST_MEMORY;
    ccDevices.resize(numberOfDevices);

    clError = clGetDeviceIDs(platform, deviceType, numberOfDevices, ccDevices.data(), 0);
    return clToComputeContextError(clError);
}

CCerror ComputeContext::waitForEvents(const Vector<CCEvent>& events)
{
    cl_int clError = clWaitForEvents(events.size(), events.data());
    return clToComputeContextError(clError);
}

CCCommandQueue ComputeContext::createCommandQueue(CCDeviceID deviceId, int properties, CCerror& error)
{
    cl_command_queue_properties clProperties;
    cl_int clError;
    switch (properties) {
    case QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
        clProperties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
        break;
    case QUEUE_PROFILING_ENABLE:
        clProperties = CL_QUEUE_PROFILING_ENABLE;
        break;
    case 0:
        clProperties = 0;
        break;
    default:
        clError = CL_INVALID_QUEUE_PROPERTIES;
        error = clToComputeContextError(clError);
        return 0;
    }

    cl_command_queue clCommandQueue = clCreateCommandQueue(m_clContext, deviceId, clProperties, &clError);

    error = clToComputeContextError(clError);
    return clCommandQueue;
}

CCProgram ComputeContext::createProgram(const String& kernelSource, CCerror& error)
{
    cl_program clProgram;
    cl_int clError;
    const char* kernelSourcePtr = kernelSource.utf8().data();

    clProgram = clCreateProgramWithSource(m_clContext, 1, &kernelSourcePtr, 0, &clError);
    error = clToComputeContextError(clError);

    return clProgram;
}

PlatformComputeObject ComputeContext::createBuffer(int memoryFlags, size_t size, void* data, CCerror& error)
{
    cl_mem clMemoryBuffer;
    cl_int clError;
    cl_int clMemoryFlags = memoryFlags;

    clMemoryBuffer = clCreateBuffer(m_clContext, clMemoryFlags, size, data, &clError);
    error = clToComputeContextError(clError);

    return clMemoryBuffer;
}

PlatformComputeObject ComputeContext::createSubBuffer(PlatformComputeObject buffer, int memoryFlags, int bufferCreatetype,
    CCBufferRegion* bufferCreateInfo, CCerror& error)
{
    cl_int clMemoryFlags = memoryFlags;
    cl_buffer_create_type clBufferCreateType = bufferCreatetype;
    cl_int clError = 0;

    cl_mem clMemoryBuffer = clCreateSubBuffer(buffer, clMemoryFlags, clBufferCreateType, bufferCreateInfo, &clError);
    error = clToComputeContextError(clError);

    return clMemoryBuffer;
}

// FIXME: ComputeContext::createImage
PlatformComputeObject ComputeContext::createImage2D(int memoryFlags, int width, int height, const CCImageFormat& imageFormat, void* data, CCerror& error)
{
    cl_mem clMemoryImage = 0;
    cl_int memoryImageError = CL_SUCCESS;
    cl_int clMemoryFlags = memoryFlags;

#if defined(CL_VERSION_1_2)
    cl_image_desc clImageDescriptor;
    clImageDescriptor.image_type = CL_MEM_OBJECT_IMAGE2D;
    clImageDescriptor.image_width = width;
    clImageDescriptor.image_height = height;

    // FIXME: we are hardcoding the image stride (i.e. row_pitch) as 0.
    // This is either a bug on the spec, or a bug in our implementation.
    clImageDescriptor.image_row_pitch = 0;
    clMemoryImage = clCreateImage(m_clContext, clMemoryFlags, &imageFormat, &clImageDescriptor,
        data, &memoryImageError);
#else
    clMemoryImage = clCreateImage2D(m_clContext, clMemoryFlags, &imageFormat, width, height, 0,
        data, &memoryImageError);
#endif

    error = clToComputeContextError(memoryImageError);
    return clMemoryImage;
}

PlatformComputeObject ComputeContext::createFromGLBuffer(int type, int bufferId, CCerror& error)
{
    cl_mem clMemoryImage = 0;
    cl_int clError;
    cl_int memoryType = computeMemoryTypeToCL(type);

    clMemoryImage = clCreateFromGLBuffer(m_clContext, memoryType, bufferId, &clError);
    error = clToComputeContextError(clError);

    return clMemoryImage;
}

PlatformComputeObject ComputeContext::createFromGLRenderbuffer(int type, GC3Dint renderbufferId, CCerror& error)
{
    cl_mem clMemory;
    cl_int clError;
    cl_int memoryType = computeMemoryTypeToCL(type);

    clMemory = clCreateFromGLRenderbuffer(m_clContext, memoryType, renderbufferId, &clError);
    error = clToComputeContextError(clError);

    return clMemory;
}

CCSampler ComputeContext::createSampler(bool normalizedCoords, int addressingMode, int filterMode, CCerror& error)
{
    cl_int clError;
    cl_sampler sampler = clCreateSampler(m_clContext, normalizedCoords, addressingMode, filterMode, &clError);
    error = clToComputeContextError(clError);

    return sampler;
}

PlatformComputeObject ComputeContext::createFromGLTexture2D(int type, GC3Denum textureTarget, GC3Dint mipLevel, GC3Duint texture, CCerror& error)
{
    cl_int memoryType = computeMemoryTypeToCL(type);
    cl_int clError;
    PlatformComputeObject memory;

#if defined(CL_VERSION_1_2)
    memory = clCreateFromGLTexture(m_clContext, memoryType, textureTarget, mipLevel, texture, &clError);
#else
    memory = clCreateFromGLTexture2D(m_clContext, memoryType, textureTarget, mipLevel, texture, &clError);
#endif

    error = clToComputeContextError(clError);

    return memory;
}

CCImageFormat* ComputeContext::supportedImageFormats(int memoryFlags, int imageType, CCuint& numberOfSupportedImages, CCerror& error)
{
    cl_int clError;
    clError = clGetSupportedImageFormats(m_clContext, memoryFlags, imageType, 0, 0, &numberOfSupportedImages);
    if (clError != CL_SUCCESS) {
        error = clToComputeContextError(clError);
        return 0;
    }

    // FIXME: We should not use malloc
    CCImageFormat* imageFormats = (CCImageFormat*) malloc(sizeof(CCImageFormat) * numberOfSupportedImages);

    clError = clGetSupportedImageFormats(m_clContext, memoryFlags, imageType, numberOfSupportedImages, imageFormats, 0);
    error = clToComputeContextError(clError);
    return clError == CL_SUCCESS ? imageFormats : 0;
}

CCerror ComputeContext::getDeviceInfo(CCDeviceID deviceID, int infoType, size_t sizeOfData, void* data)
{
   cl_int error = clGetDeviceInfo(deviceID, infoType, sizeOfData, data, 0 /*param_value_size_ret)*/);
   return clToComputeContextError(error);
}

CCerror ComputeContext::getPlatformInfo(CCPlatformID platformID, int infoType, size_t sizeOfData, void* data)
{
   cl_int error = clGetPlatformInfo(platformID, infoType, sizeOfData, data, 0 /*param_value_size_ret)*/);
   return clToComputeContextError(error);
}

CCerror ComputeContext::getProgramInfo(CCProgram program, int infoType, size_t sizeOfData, void* data, size_t* actualSizeOfData)
{
   cl_int error = clGetProgramInfo(program, infoType, sizeOfData, data, actualSizeOfData);
   return clToComputeContextError(error);
}

CCerror ComputeContext::getBuildInfo(CCProgram program, CCDeviceID device, int infoType, size_t sizeOfData, void* data)
{
    cl_program_build_info clProgramBuildInfoType = infoType;
    cl_int error = clGetProgramBuildInfo(program, device, clProgramBuildInfoType, sizeOfData, data, 0);
    return clToComputeContextError(error);
}

CCerror ComputeContext::getCommandQueueInfo(CCCommandQueue queue, int infoType, size_t sizeOfData, void* data)
{
   cl_int error = clGetCommandQueueInfo(queue, infoType, sizeOfData, data, 0);
   return clToComputeContextError(error);
}

CCerror ComputeContext::getEventInfo(CCEvent event, int infoType, size_t sizeOfData, void* data)
{
    cl_int error = clGetEventInfo(event, infoType, sizeOfData, data, 0);
    return clToComputeContextError(error);
}

CCerror ComputeContext::getEventProfilingInfo(CCEvent event, int infoType, size_t sizeOfData, void* data)
{
    cl_int error = clGetEventProfilingInfo(event, infoType, sizeOfData, data, 0);
    return clToComputeContextError(error);
}

CCerror ComputeContext::getImageInfo(PlatformComputeObject image, int infoType, size_t sizeOfData, void* data)
{
    cl_image_info clImageInfoType = infoType;
    cl_int error = clGetImageInfo(image, clImageInfoType, sizeOfData, data, 0);
    return clToComputeContextError(error);
}

CCerror ComputeContext::getGLtextureInfo(PlatformComputeObject image, int textureInfoType, size_t sizeOfData, void* data)
{
    cl_gl_texture_info clglTextureInfoType = textureInfoType;
    cl_int error = clGetGLTextureInfo(image, clglTextureInfoType, sizeOfData, data, 0);
    return clToComputeContextError(error);
}

CCerror ComputeContext::getKernelInfo(CCKernel kernel, int infoType, size_t sizeOfData, void* data)
{
    cl_kernel_info clKernelInfoType = infoType;
    cl_int error = clGetKernelInfo(kernel, clKernelInfoType, sizeOfData, data, 0);
    return clToComputeContextError(error);
}

CCerror ComputeContext::getWorkGroupInfo(CCKernel kernel, CCDeviceID device, int infoType, size_t sizeOfData, void* data)
{
    cl_kernel_work_group_info clKernelWorkGroupInfoType = infoType;
    cl_int error = clGetKernelWorkGroupInfo(kernel, device, clKernelWorkGroupInfoType, sizeOfData, data, 0);
    return clToComputeContextError(error);
}

CCerror ComputeContext::getSamplerInfo(CCSampler sampler, int infoType, size_t sizeOfData, void* data)
{
    cl_sampler_info clSamplerInfoType = infoType;
    cl_int error = clGetSamplerInfo(sampler, clSamplerInfoType, sizeOfData, data, 0);
    return clToComputeContextError(error);
}
CCerror ComputeContext::getMemoryObjectInfo(PlatformComputeObject memObject, int infoType, size_t sizeOfData, void* data)
{
    cl_mem_info clMemInfoType = infoType;
    cl_int error = clGetMemObjectInfo(memObject, clMemInfoType, sizeOfData, data, 0);
    return clToComputeContextError(error);
}

CCerror ComputeContext::releaseKernel(CCKernel kernel)
{
    cl_int error = clReleaseKernel(kernel);
    return clToComputeContextError(error);
}

CCerror ComputeContext::releaseEvent(CCEvent ccevent)
{
    cl_int error = clReleaseEvent(ccevent);
    return clToComputeContextError(error);
}

CCerror ComputeContext::releaseSampler(CCSampler sampler)
{
    cl_int error = clReleaseSampler(sampler);
    return clToComputeContextError(error);
}

CCerror ComputeContext::releaseMemoryObject(PlatformComputeObject memmory)
{
    cl_int error = clReleaseMemObject(memmory);
    return clToComputeContextError(error);
}

CCerror ComputeContext::releaseProgram(CCProgram program)
{
    cl_int error = clReleaseProgram(program);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueNDRangeKernel(CCCommandQueue commandQueue, CCKernel kernelID, int workItemDimensions,
    size_t* globalWorkOffset, size_t* globalWorkSize, size_t* localWorkSize, int eventWaitListLength, CCEvent* eventWaitList, CCEvent* event)
{
    cl_int error = clEnqueueNDRangeKernel(commandQueue, kernelID, workItemDimensions,
        globalWorkOffset, globalWorkSize, localWorkSize, eventWaitListLength, eventWaitList, event);

    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueBarrier(CCCommandQueue commandQueue)
{
    cl_int error;

#if defined(CL_VERSION_1_2)
    error = clEnqueueBarrierWithWaitList(commandQueue, 0 /*eventWaitListLength*/, 0 /*eventsWaitList*/, 0 /*event*/);
#else
    error = clEnqueueBarrier(commandQueue);
#endif

    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueMarker(CCCommandQueue commandQueue, CCEvent* event)
{
    cl_int error;

#if defined(CL_VERSION_1_2)
    error = clEnqueueMarkerWithWaitList(commandQueue, 0 /*eventsWaitListLength*/, 0 /*eventsWaitList*/, event);
#else
    error = clEnqueueMarker(commandQueue, event);
#endif

    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueTask(CCCommandQueue commandQueue, CCKernel kernelID, int eventsWaitListLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueTask(commandQueue, kernelID, eventsWaitListLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueWriteBuffer(CCCommandQueue commandQueue, PlatformComputeObject buffer, bool blockingWrite,
    int offset, int bufferSize, void* baseAddress, unsigned eventsWaitListLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueWriteBuffer(commandQueue, buffer, blockingWrite, offset,
        bufferSize, baseAddress, eventsWaitListLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueWriteBufferRect(CCCommandQueue commandQueue, PlatformComputeObject buffer, bool blockingWrite,
    size_t* bufferOriginArray, size_t* hostOriginArray, size_t* regionArray, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch,
    int hostSlicePitch, void* baseAddress, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueWriteBufferRect(commandQueue, buffer, blockingWrite, bufferOriginArray, hostOriginArray, regionArray,
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, baseAddress, eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueReadBuffer(CCCommandQueue commandQueue, PlatformComputeObject buffer, bool blockingRead, int offset, int bufferSize, void* baseAddress, unsigned eventsWaitListLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueReadBuffer(commandQueue, buffer, blockingRead, offset,
        bufferSize, baseAddress, eventsWaitListLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueReadBufferRect(CCCommandQueue commandQueue, PlatformComputeObject buffer, bool blockingRead,
    size_t* bufferOriginArray, size_t* hostOriginArray, size_t* regionArray, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch,
    int hostSlicePitch, void* baseAddress, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueReadBufferRect(commandQueue, buffer, blockingRead, bufferOriginArray, hostOriginArray, regionArray,
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, baseAddress, eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueReadImage(CCCommandQueue commandQueue, PlatformComputeObject image, bool blockingRead, size_t* originArray,
    size_t* regionArray, int rowPitch, int slicePitch, void* baseAddress, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueReadImage(commandQueue, image, blockingRead, originArray, regionArray, rowPitch, slicePitch, baseAddress, eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueWriteImage(CCCommandQueue commandQueue, PlatformComputeObject image, bool blockingWrite, size_t* originArray,
    size_t* regionArray, int rowPitch, int slicePitch, void* baseAddress, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueWriteImage(commandQueue, image, blockingWrite, originArray, regionArray, rowPitch, slicePitch, baseAddress, eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueAcquireGLObjects(CCCommandQueue commandQueue, unsigned numberOfObjects, PlatformComputeObject* objects, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueAcquireGLObjects(commandQueue, numberOfObjects, objects, eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueReleaseGLObjects(CCCommandQueue commandQueue, unsigned numberOfObjects, PlatformComputeObject* objects, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueReleaseGLObjects(commandQueue, numberOfObjects, objects, eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueCopyImage(CCCommandQueue commandQueue, PlatformComputeObject originImage, PlatformComputeObject targetImage,
    size_t* sourceOriginArray, size_t* targetOriginArray, size_t* regionArray, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyImage(commandQueue, originImage, targetImage, sourceOriginArray, targetOriginArray, regionArray,
        eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueCopyImageToBuffer(CCCommandQueue commandQueue, PlatformComputeObject sourceImage, PlatformComputeObject targetBuffer,
    size_t* sourceOriginArray, size_t* regionArray, int targetOffset, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyImageToBuffer(commandQueue, sourceImage, targetBuffer, sourceOriginArray, regionArray, targetOffset,
        eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueCopyBufferToImage(CCCommandQueue commandQueue, PlatformComputeObject sourceBuffer, PlatformComputeObject targetImage,
    int srcOffset, size_t* targetOriginArray, size_t* regionArray, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyBufferToImage(commandQueue, sourceBuffer, targetImage, srcOffset, targetOriginArray, regionArray,
        eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueCopyBuffer(CCCommandQueue commandQueue, PlatformComputeObject sourceBuffer, PlatformComputeObject targetBuffer,
    size_t sourceOffset, size_t targetOffset, size_t sizeInBytes, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyBuffer(commandQueue, sourceBuffer, targetBuffer, sourceOffset, targetOffset, sizeInBytes,
        eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::enqueueCopyBufferRect(CCCommandQueue commandQueue, PlatformComputeObject sourceBuffer, PlatformComputeObject targetBuffer,
    size_t* sourceOriginArray, size_t* targetOriginArray, size_t* regionArray, int sourceRowPitch, int sourceSlicePitch, int targetRowPitch,
    int targetSlicePitch, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyBufferRect(commandQueue, sourceBuffer, targetBuffer, sourceOriginArray, targetOriginArray, regionArray,
        sourceRowPitch, sourceSlicePitch, targetRowPitch, targetSlicePitch, eventsLength, eventsWaitList, event);
    return clToComputeContextError(error);
}

CCerror ComputeContext::releaseCommandQueue(CCCommandQueue commandQueue)
{
    cl_int error = clReleaseCommandQueue(commandQueue);
    return clToComputeContextError(error);
}

CCerror ComputeContext::finishCommandQueue(CCCommandQueue commandQueue)
{
    cl_int error = clFinish(commandQueue);
    return clToComputeContextError(error);
}

CCerror ComputeContext::flushCommandQueue(CCCommandQueue commandQueue)
{
    cl_int error = clFlush(commandQueue);
    return clToComputeContextError(error);
}

CCKernel ComputeContext::createKernel(CCProgram program, const String& kernelName, CCerror& error)
{
    cl_int clError;
    cl_kernel kernel = clCreateKernel(program, kernelName.utf8().data(), &clError);

    error = clToComputeContextError(clError);
    return kernel;
}

CCKernel* ComputeContext::createKernelsInProgram(CCProgram program, CCuint& numberOfKernels, CCerror& error)
{
    cl_kernel* kernels = 0;
    numberOfKernels = 0;

    cl_int clError = clCreateKernelsInProgram(program, 0, 0, &numberOfKernels);
    if (clError != CL_SUCCESS) {
        error = clToComputeContextError(clError);
        return 0;
    }

    if (!numberOfKernels) {
        // FIXME: Having '0' kernels is an error?
        error = clToComputeContextError(clError);
        return 0;
    }

    kernels = (cl_kernel*) malloc(sizeof(cl_kernel) * numberOfKernels);
    if (!kernels) {
        error = ComputeContext::MEM_OBJECT_ALLOCATION_FAILURE;
        return 0;
    }

    clError = clCreateKernelsInProgram(program, numberOfKernels, kernels, 0);
    error = clToComputeContextError(clError);
    return kernels;
}

}
