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

// FIXME: it is really necessary?
#define TO_CL_BOOL(var) var ? CL_TRUE : CL_FALSE

namespace WebCore {

static cl_device_info computeDeviceInfoTypeToCL(int deviceInfoType)
{
    switch (deviceInfoType) {
    case ComputeContext::MEM_READ_ONLY:
        return CL_MEM_READ_ONLY;
    case ComputeContext::DEVICE_EXTENSIONS:
        return CL_DEVICE_EXTENSIONS;
    case ComputeContext::DEVICE_NAME:
        return CL_DEVICE_NAME;
    case ComputeContext::DEVICE_PROFILE:
        return CL_DEVICE_PROFILE;
    case ComputeContext::DEVICE_VENDOR:
        return CL_DEVICE_VENDOR;
    case ComputeContext::DEVICE_VERSION:
        return CL_DEVICE_VERSION;
    case ComputeContext::DEVICE_OPENCL_C_VERSION:
        return CL_DEVICE_OPENCL_C_VERSION;
    case ComputeContext::DRIVER_VERSION:
        return CL_DRIVER_VERSION;
    case ComputeContext::DEVICE_ADDRESS_BITS:
        return CL_DEVICE_ADDRESS_BITS;
    case ComputeContext::DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
        return CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE;
    case ComputeContext::DEVICE_MAX_CLOCK_FREQUENCY:
        return CL_DEVICE_MAX_CLOCK_FREQUENCY;
    case ComputeContext::DEVICE_MAX_CONSTANT_ARGS:
        return CL_DEVICE_MAX_CONSTANT_ARGS;
    case ComputeContext::DEVICE_MAX_READ_IMAGE_ARGS:
        return CL_DEVICE_MAX_READ_IMAGE_ARGS;
    case ComputeContext::DEVICE_MAX_SAMPLERS:
        return CL_DEVICE_MAX_SAMPLERS;
    case ComputeContext::DEVICE_MAX_WRITE_IMAGE_ARGS:
        return CL_DEVICE_MAX_WRITE_IMAGE_ARGS;
    case ComputeContext::DEVICE_MEM_BASE_ADDR_ALIGN:
        return CL_DEVICE_MEM_BASE_ADDR_ALIGN;
    case ComputeContext::DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
        return CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE;
    case ComputeContext::DEVICE_VENDOR_ID:
        return CL_DEVICE_VENDOR_ID;
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
        return CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR;
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
        return CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT;
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_INT:
        return CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT;
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
        return CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG;
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
        return CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT;
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
        return CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE;
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
        return CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT;
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
        return CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF;
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
        return CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR;
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_INT:
        return CL_DEVICE_NATIVE_VECTOR_WIDTH_INT;
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_LONG:
        return CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG;
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
        return CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT;
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
        return CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE;
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_HALF:
        return CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF;
    case ComputeContext::DEVICE_MAX_COMPUTE_UNITS:
        return CL_DEVICE_MAX_COMPUTE_UNITS;
    case ComputeContext::DEVICE_IMAGE2D_MAX_HEIGHT:
        return CL_DEVICE_IMAGE2D_MAX_HEIGHT;
    case ComputeContext::DEVICE_IMAGE2D_MAX_WIDTH:
        return CL_DEVICE_IMAGE2D_MAX_WIDTH;
    case ComputeContext::DEVICE_MAX_PARAMETER_SIZE:
        return CL_DEVICE_MAX_PARAMETER_SIZE;
    case ComputeContext::DEVICE_MAX_WORK_GROUP_SIZE:
        return CL_DEVICE_MAX_WORK_GROUP_SIZE;
    case ComputeContext::DEVICE_MAX_WORK_ITEM_DIMENSIONS:
        return CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS;
    case ComputeContext::DEVICE_PROFILING_TIMER_RESOLUTION:
        return CL_DEVICE_PROFILING_TIMER_RESOLUTION;
    case ComputeContext::DEVICE_LOCAL_MEM_SIZE:
        return CL_DEVICE_LOCAL_MEM_SIZE;
    case ComputeContext::DEVICE_MAX_CONSTANT_BUFFER_SIZE:
        return CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE;
    case ComputeContext::DEVICE_MAX_MEM_ALLOC_SIZE:
        return CL_DEVICE_MAX_MEM_ALLOC_SIZE;
    case ComputeContext::DEVICE_GLOBAL_MEM_CACHE_SIZE:
        return CL_DEVICE_GLOBAL_MEM_CACHE_SIZE;
    case ComputeContext::DEVICE_GLOBAL_MEM_SIZE:
        return CL_DEVICE_GLOBAL_MEM_SIZE;
    case ComputeContext::DEVICE_AVAILABLE:
        return CL_DEVICE_AVAILABLE;
    case ComputeContext::DEVICE_COMPILER_AVAILABLE:
        return CL_DEVICE_COMPILER_AVAILABLE;
    case ComputeContext::DEVICE_ENDIAN_LITTLE:
        return CL_DEVICE_ENDIAN_LITTLE;
    case ComputeContext::DEVICE_ERROR_CORRECTION_SUPPORT:
        return CL_DEVICE_ERROR_CORRECTION_SUPPORT;
    case ComputeContext::DEVICE_HOST_UNIFIED_MEMORY:
        return CL_DEVICE_HOST_UNIFIED_MEMORY;
    case ComputeContext::DEVICE_IMAGE_SUPPORT:
        return CL_DEVICE_IMAGE_SUPPORT;
    case ComputeContext::DEVICE_TYPE:
        return CL_DEVICE_TYPE;
    case ComputeContext::DEVICE_QUEUE_PROPERTIES:
        return CL_DEVICE_QUEUE_PROPERTIES;
    case ComputeContext::DEVICE_DOUBLE_FP_CONFIG:
        return CL_DEVICE_DOUBLE_FP_CONFIG;
    case ComputeContext::DEVICE_HALF_FP_CONFIG: // Part of cl_ext.h (which isn't available in Khronos).
        return CL_DEVICE_HALF_FP_CONFIG;
    case ComputeContext::DEVICE_SINGLE_FP_CONFIG:
        return CL_DEVICE_SINGLE_FP_CONFIG;
    // Platform ID is not supported.
    case ComputeContext::DEVICE_PLATFORM:
        return CL_DEVICE_PLATFORM;
    case ComputeContext::DEVICE_EXECUTION_CAPABILITIES:
        return CL_DEVICE_EXECUTION_CAPABILITIES;
    case ComputeContext::DEVICE_GLOBAL_MEM_CACHE_TYPE:
        return CL_DEVICE_GLOBAL_MEM_CACHE_TYPE;
    case ComputeContext::DEVICE_LOCAL_MEM_TYPE:
        return CL_DEVICE_LOCAL_MEM_TYPE;
    case ComputeContext::DEVICE_MAX_WORK_ITEM_SIZES:
        return CL_DEVICE_MAX_WORK_ITEM_SIZES;
    }
    ASSERT_NOT_REACHED();
    return CL_INVALID_VALUE;
}

static cl_platform_info computePlatformInfoTypeToCL(int platformInfoType)
{
    switch (platformInfoType) {
    case ComputeContext::PLATFORM_PROFILE:
        return CL_PLATFORM_PROFILE;
    case ComputeContext::PLATFORM_VERSION:
        return CL_PLATFORM_VERSION;
    case ComputeContext::PLATFORM_NAME:
        return CL_PLATFORM_NAME;
    case ComputeContext::PLATFORM_VENDOR:
        return CL_PLATFORM_VENDOR;
    case ComputeContext::PLATFORM_EXTENSIONS:
        return CL_PLATFORM_EXTENSIONS;
    }
    ASSERT_NOT_REACHED();
    return CL_INVALID_VALUE;
}

static cl_program_info computeProgramInfoTypeToCL(int deviceInfoType)
{
    switch (deviceInfoType) {
    case ComputeContext::PROGRAM_REFERENCE_COUNT:
        return CL_PROGRAM_REFERENCE_COUNT;
    case ComputeContext::PROGRAM_CONTEXT:
        return CL_PROGRAM_CONTEXT;
    case ComputeContext::PROGRAM_NUM_DEVICES:
        return CL_PROGRAM_NUM_DEVICES;
    case ComputeContext::PROGRAM_DEVICES:
        return CL_PROGRAM_DEVICES;
    case ComputeContext::PROGRAM_SOURCE:
        return CL_PROGRAM_SOURCE;
    case ComputeContext::PROGRAM_BINARY_SIZES:
        return CL_PROGRAM_BINARY_SIZES;
    case ComputeContext::PROGRAM_BINARIES:
        return CL_PROGRAM_BINARIES;
    }
    ASSERT_NOT_REACHED();
    return CL_INVALID_VALUE;
}

static cl_command_queue_info computeCommandQueueInfoTypeToCL(int commandQueueInfoType)
{
    switch (commandQueueInfoType) {
    case ComputeContext::QUEUE_CONTEXT:
        return CL_QUEUE_CONTEXT;
    case ComputeContext::QUEUE_DEVICE:
        return CL_QUEUE_DEVICE;
    case ComputeContext::QUEUE_REFERENCE_COUNT:
        return CL_QUEUE_REFERENCE_COUNT;
    case ComputeContext::QUEUE_PROPERTIES:
        return CL_QUEUE_PROPERTIES;
    }
    ASSERT_NOT_REACHED();
    return CL_INVALID_VALUE;
}

static CCerror clToComputeContextError(cl_int clError)
{
    int computeContextError;
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

static cl_image_format computeImageFormatToCL(const ComputeContext::ImageFormat& imageFormat)
{
    cl_image_format clImageFormat;
    switch (imageFormat.channelOrder) {
    case ComputeContext::R:
        clImageFormat.image_channel_order = CL_R;
        break;
    case ComputeContext::A:
        clImageFormat.image_channel_order = CL_A;
        break;
    case ComputeContext::RG:
        clImageFormat.image_channel_order = CL_RG;
        break;
    case ComputeContext::RA:
        clImageFormat.image_channel_order = CL_RA;
        break;
    case ComputeContext::RGB:
        clImageFormat.image_channel_order = CL_RGB;
        break;
    case ComputeContext::RGBA:
        clImageFormat.image_channel_order = CL_RGBA;
        break;
    case ComputeContext::BGRA:
        clImageFormat.image_channel_order = CL_BGRA;
        break;
    case ComputeContext::ARGB:
        clImageFormat.image_channel_order = CL_ARGB;
        break;
    case ComputeContext::INTENSITY:
        clImageFormat.image_channel_order = CL_INTENSITY;
        break;
    case ComputeContext::LUMINANCE:
        clImageFormat.image_channel_order = CL_LUMINANCE;
        break;
    case ComputeContext::Rx:
        clImageFormat.image_channel_order = CL_Rx;
        break;
    case ComputeContext::RGx:
        clImageFormat.image_channel_order = CL_RGx;
        break;
    case ComputeContext::RGBx:
        clImageFormat.image_channel_order = CL_RGBx;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    switch (imageFormat.channelDataType) {
    case ComputeContext::SNORM_INT8:
        clImageFormat.image_channel_data_type = CL_SNORM_INT8;
        break;
    case ComputeContext::SNORM_INT16:
        clImageFormat.image_channel_data_type = CL_SNORM_INT16;
        break;
    case ComputeContext::UNORM_INT8:
        clImageFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
    case ComputeContext::UNORM_INT16:
        clImageFormat.image_channel_data_type = CL_UNORM_INT16;
        break;
    case ComputeContext::UNORM_SHORT_565:
        clImageFormat.image_channel_data_type = CL_UNORM_SHORT_565;
        break;
    case ComputeContext::UNORM_SHORT_555:
        clImageFormat.image_channel_data_type = CL_UNORM_SHORT_555;
        break;
    case ComputeContext::UNORM_INT_101010:
        clImageFormat.image_channel_data_type = CL_UNORM_INT_101010;
        break;
    case ComputeContext::SIGNED_INT8:
        clImageFormat.image_channel_data_type = CL_SIGNED_INT8;
        break;
    case ComputeContext::SIGNED_INT16:
        clImageFormat.image_channel_data_type = CL_SIGNED_INT16;
        break;
    case ComputeContext::SIGNED_INT32:
        clImageFormat.image_channel_data_type = CL_SIGNED_INT32;
        break;
    case ComputeContext::UNSIGNED_INT8:
        clImageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
        break;
    case ComputeContext::UNSIGNED_INT16:
        clImageFormat.image_channel_data_type = CL_UNSIGNED_INT16;
        break;
    case ComputeContext::UNSIGNED_INT32:
        clImageFormat.image_channel_data_type = CL_UNSIGNED_INT32;
        break;
    case ComputeContext::HALF_FLOAT:
        clImageFormat.image_channel_data_type = CL_HALF_FLOAT;
        break;
    case ComputeContext::FLOAT:
        clImageFormat.image_channel_data_type = CL_FLOAT;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clImageFormat;
}

static cl_mem_flags computeMemoryTypeToCL(int memoryType)
{
    cl_int clMemoryType;
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
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clMemoryType;
}

ComputeContext::ComputeContext(CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID* devices, CCerror& error)
{
    cl_int clError;
    m_clContext = clCreateContext(contextProperties, numberDevices, devices, 0, 0, &clError);
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

PassRefPtr<ComputeContext> ComputeContext::create(CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID* devices, CCerror& error)
{
    RefPtr<ComputeContext> computeContext =  adoptRef(new ComputeContext(contextProperties, numberDevices, devices, error));
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

CCint ComputeContext::platformIDs(CCuint numberEntries, CCPlatformID* platforms, CCerror& error)
{
    cl_uint numberOfPlatforms;
    cl_int clError = clGetPlatformIDs(numberEntries, platforms, &numberOfPlatforms);

    error = clToComputeContextError(clError);
    return numberOfPlatforms;
}

CCint ComputeContext::deviceIDs(CCPlatformID platform, CCDeviceType deviceType, CCuint numberEntries, CCDeviceID* devices, CCerror& error)
{
    cl_uint numberOfDevices;
    cl_int clError = clGetDeviceIDs(platform, deviceType, numberEntries, devices, &numberOfDevices);

    error = clToComputeContextError(clError);
    return numberOfDevices;
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

PlatformComputeObject ComputeContext::createBuffer(int type, size_t size, void* data, CCerror& error)
{
    cl_mem clMemoryBuffer;
    cl_int clError;
    cl_int memoryType = computeMemoryTypeToCL(type);

    clMemoryBuffer = clCreateBuffer(m_clContext, memoryType, size, data, &clError);
    error = clToComputeContextError(clError);

    return clMemoryBuffer;
}

PlatformComputeObject ComputeContext::createImage2D(int type, int width, int height, const ImageFormat& imageFormat, void* data, CCerror& error)
{
    cl_mem clMemoryImage = 0;
    cl_int memoryImageError = CL_SUCCESS;
    cl_image_format clImageFormat = computeImageFormatToCL(imageFormat);

    // FIXME: verify if CL_MEM_USE_HOST_PTR is necessary
    cl_int memoryType = computeMemoryTypeToCL(type);

#if defined(CL_VERSION_1_2)
    cl_image_desc clImageDescriptor;
    clImageDescriptor.image_type = CL_MEM_OBJECT_IMAGE2D;
    clImageDescriptor.image_width = width;
    clImageDescriptor.image_height = height;

    // FIXME: we are hardcoding the image stride (i.e. row_pitch) as 0.
    // This is either a bug on the spec, or a bug in our implementation.
    clImageDescriptor.image_row_pitch = 0;
    clMemoryImage = clCreateImage(m_clContext, memoryType, &clImageFormat, &clImageDescriptor,
        data, &memoryImageError);
#else
    clMemoryImage = clCreateImage2D(m_clContext, memoryType, &clImageFormat, width, height, 0,
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

static cl_addressing_mode computeAddressingModeToCL(int addrMode)
{
    cl_addressing_mode clAddrMode;
    switch (addrMode) {
    case ComputeContext::ADDRESS_NONE:
        clAddrMode = CL_ADDRESS_NONE;
        break;
    case ComputeContext::ADDRESS_CLAMP_TO_EDGE:
        clAddrMode = CL_ADDRESS_CLAMP_TO_EDGE;
        break;
    case ComputeContext::ADDRESS_CLAMP:
        clAddrMode = CL_ADDRESS_CLAMP;
        break;
    case ComputeContext::ADDRESS_REPEAT:
        clAddrMode = CL_ADDRESS_REPEAT;
        break;
    case ComputeContext::ADDRESS_MIRRORED_REPEAT:
        clAddrMode = CL_ADDRESS_MIRRORED_REPEAT;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clAddrMode;
}

static cl_filter_mode computeFilterModeToCL(int filterMode)
{
    cl_filter_mode clFilterMode;
    switch (filterMode) {
    case ComputeContext::FILTER_LINEAR:
        clFilterMode = CL_FILTER_LINEAR;
        break;
    case ComputeContext::FILTER_NEAREST:
        clFilterMode = CL_FILTER_NEAREST;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clFilterMode;
}

CCSampler ComputeContext::createSampler(bool normalizedCoords, int addressingMode, int filterMode, CCerror& error)
{
    cl_addressing_mode clAddressingMode = computeAddressingModeToCL(addressingMode);
    cl_filter_mode clFilterMode = computeFilterModeToCL(filterMode);
    cl_int clError;

    cl_sampler sampler = clCreateSampler(m_clContext, TO_CL_BOOL(normalizedCoords), clAddressingMode, clFilterMode, &clError);
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

CCImageFormat* ComputeContext::supportedImageFormats(int type, int imageType, CCuint& numberOfSupportedImages, CCerror& error)
{
    cl_mem_flags memoryType = computeMemoryTypeToCL(type);

    if (imageType != ComputeContext::MEM_OBJECT_IMAGE2D) {
        error = ComputeContext::INVALID_VALUE;
        return 0;
    }

    cl_mem_object_type clImageType = CL_MEM_OBJECT_IMAGE2D;

    cl_int clError;
    clError = clGetSupportedImageFormats(m_clContext, memoryType, clImageType, 0, 0, &numberOfSupportedImages);

    if (clError != CL_SUCCESS) {
        error = clToComputeContextError(clError);
        return 0;
    }

    // FIXME: We should not use malloc
    CCImageFormat* imageFormats = (CCImageFormat*) malloc(sizeof(CCImageFormat) * numberOfSupportedImages);

    clError = clGetSupportedImageFormats(m_clContext, memoryType, clImageType, numberOfSupportedImages, imageFormats, 0);
    if (clError != CL_SUCCESS) {
        error = clToComputeContextError(clError);
        return 0;
    }

    error = clToComputeContextError(clError);
    return imageFormats;
}

CCerror ComputeContext::getDeviceInfo(CCDeviceID deviceID, int infoType, size_t sizeOfData, void* data)
{
   cl_device_info clDeviceInfoType = computeDeviceInfoTypeToCL(infoType);
   cl_int error = clGetDeviceInfo(deviceID, clDeviceInfoType, sizeOfData, data, 0 /*param_value_size_ret)*/);
   return clToComputeContextError(error);
}

CCerror ComputeContext::getPlatformInfo(CCPlatformID platformID, int infoType, size_t sizeOfData, void* data)
{
   cl_platform_info clPlatformInfoType = computePlatformInfoTypeToCL(infoType);
   cl_int error = clGetPlatformInfo(platformID, clPlatformInfoType, sizeOfData, data, 0 /*param_value_size_ret)*/);
   return clToComputeContextError(error);
}

CCerror ComputeContext::getProgramInfo(CCProgram program, int infoType, size_t sizeOfData, void* data, size_t* actualSizeOfData)
{
   cl_program_info clProgramInfoType = computeProgramInfoTypeToCL(infoType);
   cl_int error = clGetProgramInfo(program, clProgramInfoType, sizeOfData, data, actualSizeOfData);
   return clToComputeContextError(error);
}

CCerror ComputeContext::getCommandQueueInfo(CCCommandQueue queue, int infoType, size_t sizeOfData, void* data)
{
   cl_command_queue_info clCommandQueueInfoType = computeCommandQueueInfoTypeToCL(infoType);
   cl_int error = clGetCommandQueueInfo(queue, clCommandQueueInfoType, sizeOfData, data, 0);
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
