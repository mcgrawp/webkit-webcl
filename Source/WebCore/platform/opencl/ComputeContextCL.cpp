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

#include "ComputeCommandQueue.h"
#include "ComputeDevice.h"
#include "ComputeEvent.h"
#include "ComputeMemoryObject.h"
#include "ComputePlatform.h"
#include "ComputeProgram.h"
#include "ComputeSampler.h"

#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

#if PLATFORM(MAC)
#include <OpenGL/OpenGL.h>
#elif USE(GLX)
#include <GL/glx.h>
#include "GLContext.h"
#endif

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace WebCore {

#define COMPILE_ASSERT_MATCHING_ENUM(computeContextName, openCLName) \
    COMPILE_ASSERT(static_cast<int>(computeContextName) == static_cast<int>(openCLName), mismatchingEnums)
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SUCCESS, CL_SUCCESS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NOT_FOUND, CL_DEVICE_NOT_FOUND);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NOT_AVAILABLE, CL_DEVICE_NOT_AVAILABLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMPILER_NOT_AVAILABLE, CL_COMPILER_NOT_AVAILABLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_OBJECT_ALLOCATION_FAILURE, CL_MEM_OBJECT_ALLOCATION_FAILURE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::OUT_OF_RESOURCES, CL_OUT_OF_RESOURCES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::OUT_OF_HOST_MEMORY, CL_OUT_OF_HOST_MEMORY);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROFILING_INFO_NOT_AVAILABLE, CL_PROFILING_INFO_NOT_AVAILABLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_COPY_OVERLAP, CL_MEM_COPY_OVERLAP);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::IMAGE_FORMAT_MISMATCH, CL_IMAGE_FORMAT_MISMATCH);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::IMAGE_FORMAT_NOT_SUPPORTED, CL_IMAGE_FORMAT_NOT_SUPPORTED);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::BUILD_PROGRAM_FAILURE, CL_BUILD_PROGRAM_FAILURE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MAP_FAILURE, CL_MAP_FAILURE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MISALIGNED_SUB_BUFFER_OFFSET, CL_MISALIGNED_SUB_BUFFER_OFFSET);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_VALUE, CL_INVALID_VALUE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_DEVICE_TYPE, CL_INVALID_DEVICE_TYPE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_PLATFORM, CL_INVALID_PLATFORM);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_DEVICE, CL_INVALID_DEVICE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_CONTEXT, CL_INVALID_CONTEXT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_QUEUE_PROPERTIES, CL_INVALID_QUEUE_PROPERTIES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_COMMAND_QUEUE, CL_INVALID_COMMAND_QUEUE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_HOST_PTR, CL_INVALID_HOST_PTR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_MEM_OBJECT, CL_INVALID_MEM_OBJECT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_IMAGE_FORMAT_DESCRIPTOR, CL_INVALID_IMAGE_FORMAT_DESCRIPTOR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_IMAGE_SIZE, CL_INVALID_IMAGE_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_SAMPLER, CL_INVALID_SAMPLER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_BINARY, CL_INVALID_BINARY);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_BUILD_OPTIONS, CL_INVALID_BUILD_OPTIONS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_PROGRAM, CL_INVALID_PROGRAM);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_PROGRAM_EXECUTABLE, CL_INVALID_PROGRAM_EXECUTABLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_KERNEL_NAME, CL_INVALID_KERNEL_NAME);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_KERNEL_DEFINITION, CL_INVALID_KERNEL_DEFINITION);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_KERNEL, CL_INVALID_KERNEL);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_ARG_INDEX, CL_INVALID_ARG_INDEX);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_ARG_VALUE, CL_INVALID_ARG_VALUE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_ARG_SIZE, CL_INVALID_ARG_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_KERNEL_ARGS, CL_INVALID_KERNEL_ARGS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_WORK_DIMENSION, CL_INVALID_WORK_DIMENSION);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_WORK_GROUP_SIZE, CL_INVALID_WORK_GROUP_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_WORK_ITEM_SIZE, CL_INVALID_WORK_ITEM_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_GLOBAL_OFFSET, CL_INVALID_GLOBAL_OFFSET);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_EVENT_WAIT_LIST, CL_INVALID_EVENT_WAIT_LIST);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_EVENT, CL_INVALID_EVENT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_OPERATION, CL_INVALID_OPERATION);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_GL_OBJECT, CL_INVALID_GL_OBJECT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_BUFFER_SIZE, CL_INVALID_BUFFER_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_MIP_LEVEL, CL_INVALID_MIP_LEVEL);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_GLOBAL_WORK_SIZE, CL_INVALID_GLOBAL_WORK_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INVALID_PROPERTY, CL_INVALID_PROPERTY);

COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::VERSION_1_0, CL_VERSION_1_0);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::VERSION_1_1, CL_VERSION_1_1);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PLATFORM_PROFILE, CL_PLATFORM_PROFILE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PLATFORM_VERSION, CL_PLATFORM_VERSION);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PLATFORM_NAME, CL_PLATFORM_NAME);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PLATFORM_VENDOR, CL_PLATFORM_VENDOR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PLATFORM_EXTENSIONS, CL_PLATFORM_EXTENSIONS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_TYPE_DEFAULT, CL_DEVICE_TYPE_DEFAULT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_TYPE_CPU, CL_DEVICE_TYPE_CPU);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_TYPE_GPU, CL_DEVICE_TYPE_GPU);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_TYPE_ACCELERATOR, CL_DEVICE_TYPE_ACCELERATOR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_TYPE_ALL, CL_DEVICE_TYPE_ALL);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_TYPE, CL_DEVICE_TYPE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_VENDOR_ID, CL_DEVICE_VENDOR_ID);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_COMPUTE_UNITS, CL_DEVICE_MAX_COMPUTE_UNITS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_WORK_ITEM_DIMENSIONS, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_WORK_GROUP_SIZE, CL_DEVICE_MAX_WORK_GROUP_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_WORK_ITEM_SIZES, CL_DEVICE_MAX_WORK_ITEM_SIZES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_INT, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_LONG, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_CLOCK_FREQUENCY, CL_DEVICE_MAX_CLOCK_FREQUENCY);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_ADDRESS_BITS, CL_DEVICE_ADDRESS_BITS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_READ_IMAGE_ARGS, CL_DEVICE_MAX_READ_IMAGE_ARGS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_WRITE_IMAGE_ARGS, CL_DEVICE_MAX_WRITE_IMAGE_ARGS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_MEM_ALLOC_SIZE, CL_DEVICE_MAX_MEM_ALLOC_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_IMAGE2D_MAX_WIDTH, CL_DEVICE_IMAGE2D_MAX_WIDTH);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_IMAGE2D_MAX_HEIGHT, CL_DEVICE_IMAGE2D_MAX_HEIGHT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_IMAGE3D_MAX_WIDTH, CL_DEVICE_IMAGE3D_MAX_WIDTH);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_IMAGE3D_MAX_HEIGHT, CL_DEVICE_IMAGE3D_MAX_HEIGHT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_IMAGE3D_MAX_WIDTH, CL_DEVICE_IMAGE3D_MAX_WIDTH);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_IMAGE_SUPPORT, CL_DEVICE_IMAGE_SUPPORT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_PARAMETER_SIZE, CL_DEVICE_MAX_PARAMETER_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_SAMPLERS, CL_DEVICE_MAX_SAMPLERS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MEM_BASE_ADDR_ALIGN, CL_DEVICE_MEM_BASE_ADDR_ALIGN);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_SINGLE_FP_CONFIG, CL_DEVICE_SINGLE_FP_CONFIG);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_GLOBAL_MEM_CACHE_TYPE, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_GLOBAL_MEM_CACHELINE_SIZE, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_GLOBAL_MEM_CACHE_SIZE, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_GLOBAL_MEM_SIZE, CL_DEVICE_GLOBAL_MEM_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_CONSTANT_BUFFER_SIZE, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_MAX_CONSTANT_ARGS, CL_DEVICE_MAX_CONSTANT_ARGS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_LOCAL_MEM_TYPE, CL_DEVICE_LOCAL_MEM_TYPE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_LOCAL_MEM_SIZE, CL_DEVICE_LOCAL_MEM_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_ERROR_CORRECTION_SUPPORT, CL_DEVICE_ERROR_CORRECTION_SUPPORT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PROFILING_TIMER_RESOLUTION, CL_DEVICE_PROFILING_TIMER_RESOLUTION);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_ENDIAN_LITTLE, CL_DEVICE_ENDIAN_LITTLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_AVAILABLE, CL_DEVICE_AVAILABLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_COMPILER_AVAILABLE, CL_DEVICE_COMPILER_AVAILABLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_EXECUTION_CAPABILITIES, CL_DEVICE_EXECUTION_CAPABILITIES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_QUEUE_PROPERTIES, CL_DEVICE_QUEUE_PROPERTIES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NAME, CL_DEVICE_NAME);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_VENDOR, CL_DEVICE_VENDOR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DRIVER_VERSION, CL_DRIVER_VERSION);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PROFILE, CL_DEVICE_PROFILE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_VERSION, CL_DEVICE_VERSION);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_EXTENSIONS, CL_DEVICE_EXTENSIONS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PLATFORM, CL_DEVICE_PLATFORM);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_DOUBLE_FP_CONFIG, CL_DEVICE_DOUBLE_FP_CONFIG);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_HALF_FP_CONFIG, CL_DEVICE_HALF_FP_CONFIG);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_HALF, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_HOST_UNIFIED_MEMORY, CL_DEVICE_HOST_UNIFIED_MEMORY);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_CHAR, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_SHORT, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_INT, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_LONG, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_HALF, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::DEVICE_OPENCL_C_VERSION, CL_DEVICE_OPENCL_C_VERSION);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FP_DENORM, CL_FP_DENORM);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FP_INF_NAN, CL_FP_INF_NAN);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FP_ROUND_TO_NEAREST, CL_FP_ROUND_TO_NEAREST);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FP_ROUND_TO_ZERO, CL_FP_ROUND_TO_ZERO);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FP_ROUND_TO_INF, CL_FP_ROUND_TO_INF);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FP_FMA, CL_FP_FMA);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FP_SOFT_FLOAT, CL_FP_SOFT_FLOAT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::NONE, CL_NONE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::READ_ONLY_CACHE, CL_READ_ONLY_CACHE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::READ_WRITE_CACHE, CL_READ_WRITE_CACHE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::LOCAL, CL_LOCAL);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::GLOBAL, CL_GLOBAL);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::EXEC_KERNEL, CL_EXEC_KERNEL);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::EXEC_NATIVE_KERNEL, CL_EXEC_NATIVE_KERNEL);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::QUEUE_PROFILING_ENABLE, CL_QUEUE_PROFILING_ENABLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::CONTEXT_REFERENCE_COUNT, CL_CONTEXT_REFERENCE_COUNT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::CONTEXT_DEVICES, CL_CONTEXT_DEVICES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::CONTEXT_PROPERTIES, CL_CONTEXT_PROPERTIES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::CONTEXT_NUM_DEVICES, CL_CONTEXT_NUM_DEVICES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::CONTEXT_PLATFORM, CL_CONTEXT_PLATFORM);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::QUEUE_CONTEXT, CL_QUEUE_CONTEXT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::QUEUE_DEVICE, CL_QUEUE_DEVICE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::QUEUE_REFERENCE_COUNT, CL_QUEUE_REFERENCE_COUNT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::QUEUE_PROPERTIES, CL_QUEUE_PROPERTIES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_READ_WRITE, CL_MEM_READ_WRITE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_WRITE_ONLY, CL_MEM_WRITE_ONLY);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_READ_ONLY, CL_MEM_READ_ONLY);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_USE_HOST_PTR, CL_MEM_USE_HOST_PTR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_ALLOC_HOST_PTR, CL_MEM_ALLOC_HOST_PTR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_COPY_HOST_PTR, CL_MEM_COPY_HOST_PTR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::R, CL_R);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::A, CL_A);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::RG, CL_RG);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::RA, CL_RA);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::RGB, CL_RGB);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::RGBA, CL_RGBA);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::BGRA, CL_BGRA);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::ARGB, CL_ARGB);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::INTENSITY, CL_INTENSITY);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::LUMINANCE, CL_LUMINANCE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::Rx, CL_Rx);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::RGx, CL_RGx);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::RGBx, CL_RGBx);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SNORM_INT8, CL_SNORM_INT8);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SNORM_INT16, CL_SNORM_INT16);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::UNORM_INT8, CL_UNORM_INT8);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::UNORM_INT16, CL_UNORM_INT16);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::UNORM_SHORT_565, CL_UNORM_SHORT_565);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::UNORM_SHORT_555, CL_UNORM_SHORT_555);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::UNORM_INT_101010, CL_UNORM_INT_101010);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SIGNED_INT8, CL_SIGNED_INT8);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SIGNED_INT16, CL_SIGNED_INT16);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SIGNED_INT32, CL_SIGNED_INT32);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::UNSIGNED_INT8, CL_UNSIGNED_INT8);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::UNSIGNED_INT16, CL_UNSIGNED_INT16);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::UNSIGNED_INT32, CL_UNSIGNED_INT32);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::HALF_FLOAT, CL_HALF_FLOAT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FLOAT, CL_FLOAT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_OBJECT_BUFFER, CL_MEM_OBJECT_BUFFER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_OBJECT_IMAGE2D, CL_MEM_OBJECT_IMAGE2D);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_OBJECT_IMAGE3D, CL_MEM_OBJECT_IMAGE3D);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_TYPE, CL_MEM_TYPE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_FLAGS, CL_MEM_FLAGS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_SIZE, CL_MEM_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_HOST_PTR, CL_MEM_HOST_PTR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_MAP_COUNT, CL_MEM_MAP_COUNT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_REFERENCE_COUNT, CL_MEM_REFERENCE_COUNT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_CONTEXT, CL_MEM_CONTEXT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_ASSOCIATED_MEMOBJECT, CL_MEM_ASSOCIATED_MEMOBJECT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MEM_OFFSET, CL_MEM_OFFSET);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::IMAGE_FORMAT, CL_IMAGE_FORMAT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::IMAGE_ELEMENT_SIZE, CL_IMAGE_ELEMENT_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::IMAGE_ROW_PITCH, CL_IMAGE_ROW_PITCH);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::IMAGE_WIDTH, CL_IMAGE_WIDTH);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::IMAGE_HEIGHT, CL_IMAGE_HEIGHT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::ADDRESS_NONE, CL_ADDRESS_NONE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::ADDRESS_CLAMP_TO_EDGE, CL_ADDRESS_CLAMP_TO_EDGE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::ADDRESS_CLAMP, CL_ADDRESS_CLAMP);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::ADDRESS_REPEAT, CL_ADDRESS_REPEAT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::ADDRESS_MIRRORED_REPEAT, CL_ADDRESS_MIRRORED_REPEAT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FILTER_NEAREST, CL_FILTER_NEAREST);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::FILTER_LINEAR, CL_FILTER_LINEAR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SAMPLER_REFERENCE_COUNT, CL_SAMPLER_REFERENCE_COUNT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SAMPLER_CONTEXT, CL_SAMPLER_CONTEXT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SAMPLER_NORMALIZED_COORDS, CL_SAMPLER_NORMALIZED_COORDS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SAMPLER_ADDRESSING_MODE, CL_SAMPLER_ADDRESSING_MODE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SAMPLER_FILTER_MODE, CL_SAMPLER_FILTER_MODE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MAP_READ, CL_MAP_READ);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::MAP_WRITE, CL_MAP_WRITE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROGRAM_REFERENCE_COUNT, CL_PROGRAM_REFERENCE_COUNT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROGRAM_CONTEXT, CL_PROGRAM_CONTEXT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROGRAM_NUM_DEVICES, CL_PROGRAM_NUM_DEVICES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROGRAM_DEVICES, CL_PROGRAM_DEVICES);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROGRAM_SOURCE, CL_PROGRAM_SOURCE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROGRAM_BUILD_STATUS, CL_PROGRAM_BUILD_STATUS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROGRAM_BUILD_OPTIONS, CL_PROGRAM_BUILD_OPTIONS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROGRAM_BUILD_LOG, CL_PROGRAM_BUILD_LOG);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::BUILD_SUCCESS, CL_BUILD_SUCCESS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::BUILD_NONE, CL_BUILD_NONE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::BUILD_ERROR, CL_BUILD_ERROR);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::BUILD_IN_PROGRESS, CL_BUILD_IN_PROGRESS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_FUNCTION_NAME, CL_KERNEL_FUNCTION_NAME);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_NUM_ARGS, CL_KERNEL_NUM_ARGS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_REFERENCE_COUNT, CL_KERNEL_REFERENCE_COUNT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_CONTEXT, CL_KERNEL_CONTEXT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_PROGRAM, CL_KERNEL_PROGRAM);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_WORK_GROUP_SIZE, CL_KERNEL_WORK_GROUP_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_COMPILE_WORK_GROUP_SIZE, CL_KERNEL_COMPILE_WORK_GROUP_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_LOCAL_MEM_SIZE, CL_KERNEL_LOCAL_MEM_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::KERNEL_PRIVATE_MEM_SIZE, CL_KERNEL_PRIVATE_MEM_SIZE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::EVENT_COMMAND_QUEUE, CL_EVENT_COMMAND_QUEUE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::EVENT_COMMAND_TYPE, CL_EVENT_COMMAND_TYPE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::EVENT_REFERENCE_COUNT, CL_EVENT_REFERENCE_COUNT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::EVENT_COMMAND_EXECUTION_STATUS, CL_EVENT_COMMAND_EXECUTION_STATUS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::EVENT_CONTEXT, CL_EVENT_CONTEXT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_NDRANGE_KERNEL, CL_COMMAND_NDRANGE_KERNEL);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_TASK, CL_COMMAND_TASK);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_NATIVE_KERNEL, CL_COMMAND_NATIVE_KERNEL);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_READ_BUFFER, CL_COMMAND_READ_BUFFER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_WRITE_BUFFER, CL_COMMAND_WRITE_BUFFER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_COPY_BUFFER, CL_COMMAND_COPY_BUFFER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_READ_IMAGE, CL_COMMAND_READ_IMAGE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_WRITE_IMAGE, CL_COMMAND_WRITE_IMAGE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_COPY_IMAGE, CL_COMMAND_COPY_IMAGE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_COPY_IMAGE_TO_BUFFER, CL_COMMAND_COPY_IMAGE_TO_BUFFER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_COPY_BUFFER_TO_IMAGE, CL_COMMAND_COPY_BUFFER_TO_IMAGE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_MAP_BUFFER, CL_COMMAND_MAP_BUFFER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_MAP_IMAGE, CL_COMMAND_MAP_IMAGE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_UNMAP_MEM_OBJECT, CL_COMMAND_UNMAP_MEM_OBJECT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_MARKER, CL_COMMAND_MARKER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_ACQUIRE_GL_OBJECTS, CL_COMMAND_ACQUIRE_GL_OBJECTS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_RELEASE_GL_OBJECTS, CL_COMMAND_RELEASE_GL_OBJECTS);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_READ_BUFFER_RECT, CL_COMMAND_READ_BUFFER_RECT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_WRITE_BUFFER_RECT, CL_COMMAND_WRITE_BUFFER_RECT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_COPY_BUFFER_RECT, CL_COMMAND_COPY_BUFFER_RECT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMMAND_USER, CL_COMMAND_USER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::COMPLETE, CL_COMPLETE);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::RUNNING, CL_RUNNING);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::SUBMITTED, CL_SUBMITTED);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::QUEUED, CL_QUEUED);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::BUFFER_CREATE_TYPE_REGION, CL_BUFFER_CREATE_TYPE_REGION);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROFILING_COMMAND_QUEUED, CL_PROFILING_COMMAND_QUEUED);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROFILING_COMMAND_SUBMIT, CL_PROFILING_COMMAND_SUBMIT);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROFILING_COMMAND_START, CL_PROFILING_COMMAND_START);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::PROFILING_COMMAND_END, CL_PROFILING_COMMAND_END);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::GL_OBJECT_BUFFER, CL_GL_OBJECT_BUFFER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::GL_OBJECT_TEXTURE2D, CL_GL_OBJECT_TEXTURE2D);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::GL_OBJECT_RENDERBUFFER, CL_GL_OBJECT_RENDERBUFFER);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::GL_TEXTURE_TARGET, CL_GL_TEXTURE_TARGET);
COMPILE_ASSERT_MATCHING_ENUM(ComputeContext::GL_MIPMAP_LEVEL, CL_GL_MIPMAP_LEVEL);

static void setUpComputeContextProperties(ComputePlatform* platform, GraphicsContext3D* glContext, Vector<CCContextProperties>& properties)
{
    if (platform) {
        properties.append(ComputeContext::CONTEXT_PLATFORM);
        properties.append(reinterpret_cast<CCContextProperties>(platform->platform()));
    }

    if (glContext)
        ComputeContext::populatePropertiesForInteroperabilityWithGL(properties, glContext->platformGraphicsContext3D());

    // FIXME: If no valid platform or glContext is passed, context create fails.
    // It does work with a literal {0} though.
    properties.append(0);
}

PassRefPtr<ComputeContext> ComputeContext::create(const Vector<ComputeDevice*>& devices, ComputePlatform* platform, GraphicsContext3D* context3D, CCerror& error)
{
    return adoptRef(new ComputeContext(devices, platform, context3D, error));
}

ComputeContext::ComputeContext(const Vector<ComputeDevice*>& devices, ComputePlatform* platform, GraphicsContext3D* context3D, CCerror& error)
{
    Vector<CCDeviceID> clDevices;
    for (size_t i = 0; i < devices.size(); ++i)
        clDevices.append(devices[i]->device());

    Vector<CCContextProperties> contextProperties;
    setUpComputeContextProperties(platform, context3D, contextProperties);
    m_clContext = clCreateContext(contextProperties.data(), devices.size(), clDevices.data(), 0, 0, &error);
}

ComputeContext::~ComputeContext()
{
    clReleaseContext(m_clContext);
}

// FIXME: Move to ComputeEvent class.
CCerror ComputeContext::waitForEvents(const Vector<ComputeEvent*>& events)
{
    Vector<CCEvent> ccEvents;

    for (size_t i = 0; i < events.size(); ++i) {
        ASSERT(events[i]->event());
        ccEvents.append(events[i]->event());
    }
    CCint clError = clWaitForEvents(events.size(), ccEvents.data());
    return clError;
}

PassRefPtr<ComputeCommandQueue> ComputeContext::createCommandQueue(ComputeDevice* device, CCCommandQueueProperties properties, CCerror& error)
{
    return ComputeCommandQueue::create(this, device, properties, error);
}

PassRefPtr<ComputeEvent> ComputeContext::createUserEvent(CCerror& error)
{
    return ComputeEvent::create(this, error);
}

PassRefPtr<ComputeProgram> ComputeContext::createProgram(const String& programSource, CCerror& error)
{
    return ComputeProgram::create(this, programSource, error);
}

PassRefPtr<ComputeMemoryObject> ComputeContext::createBuffer(CCMemoryFlags type, size_t size, void* data, CCerror& error)
{
    return ComputeMemoryObject::create(this, type, size, data, error);
}

PassRefPtr<ComputeMemoryObject> ComputeContext::createImage2D(CCMemoryFlags flags, size_t width, size_t height, CCuint rowPitch, const CCImageFormat& imageFormat, void* data, CCerror& error)
{
    return ComputeMemoryObject::create(this, flags, width, height, rowPitch, imageFormat, data, error);
}

PassRefPtr<ComputeMemoryObject> ComputeContext::createFromGLBuffer(CCMemoryFlags flags, GC3Duint bufferId, CCerror& error)
{
    return ComputeMemoryObject::create(this, flags, bufferId, GLBuffer, error);
}

PassRefPtr<ComputeMemoryObject> ComputeContext::createFromGLRenderbuffer(CCMemoryFlags flags, GC3Duint renderbufferId, CCerror& error)
{
    return ComputeMemoryObject::create(this, flags, renderbufferId, GLRenderbuffer, error);
}

PassRefPtr<ComputeMemoryObject> ComputeContext::createFromGLTexture2D(CCMemoryFlags flags, GC3Denum textureTarget, GC3Dint mipLevel, GC3Duint texture, CCerror& error)
{
    return ComputeMemoryObject::create(this, flags, textureTarget, mipLevel, texture, error);
}

PassRefPtr<ComputeSampler> ComputeContext::createSampler(CCbool normalizedCoords, CCAddressingMode addressingMode, CCFilterMode filterMode, CCerror& error)
{
    return ComputeSampler::create(this, normalizedCoords, addressingMode, filterMode, error);
}

CCerror ComputeContext::supportedImageFormats(CCMemoryFlags type, CCMemoryObjectType imageType, Vector<CCImageFormat>& imageFormatsOut)
{
    CCuint numberOfSupportedImageFormats = 0;
    CCint clError = clGetSupportedImageFormats(m_clContext, type, imageType, 0, 0, &numberOfSupportedImageFormats);
    if (clError != CL_SUCCESS)
        return clError;

    imageFormatsOut.reserveCapacity(numberOfSupportedImageFormats);
    imageFormatsOut.resize(numberOfSupportedImageFormats);

    clError = clGetSupportedImageFormats(m_clContext, type, imageType, numberOfSupportedImageFormats, imageFormatsOut.data(), 0);
    return clError;
}

void ComputeContext::populatePropertiesForInteroperabilityWithGL(Vector<CCContextProperties>& properties, PlatformGraphicsContext3D context3D)
{
#if PLATFORM(MAC)
    CGLContextObj kCGLContext = context3D;
    CGLShareGroupObj kCGLShareGroup = CGLGetShareGroup(kCGLContext);
    properties.append(CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE);
    properties.append(reinterpret_cast<CCContextProperties>(kCGLShareGroup));
#elif USE(GLX)
    PlatformGraphicsContext3D glContext = context3D;
    properties.append(CL_GL_CONTEXT_KHR);
    properties.append(reinterpret_cast<CCContextProperties>(glContext));
    properties.append(CL_GLX_DISPLAY_KHR);
    properties.append(reinterpret_cast<CCContextProperties>(GLContext::getCurrent()->sharedX11Display()));
#endif
}

CCerror ComputeContext::CCPackImageData(Image* image, GraphicsContext3D::ImageHtmlDomSource domSource, unsigned width, unsigned height, Vector<uint8_t>& data)
{
    GraphicsContext3D::ImageExtractor imageExtractor(image, domSource, false /* non-premultiplied alpha*/, false);
    if (!imageExtractor.extractSucceeded())
        return INVALID_HOST_PTR;

    GraphicsContext3D::DataFormat sourceDataFormat = imageExtractor.imageSourceFormat();
    GraphicsContext3D::AlphaOp alphaOp = imageExtractor.imageAlphaOp();
    const void* imagePixelData = imageExtractor.imagePixelData();
    unsigned imageSourceUnpackAlignment = imageExtractor.imageSourceUnpackAlignment();

    if (GraphicsContext3D::packImageData(image, imagePixelData, GraphicsContext3D::RGBA, GraphicsContext3D::UNSIGNED_BYTE, false /* flipY */, alphaOp, sourceDataFormat, width, height, imageSourceUnpackAlignment, data))
        return SUCCESS;
    return INVALID_HOST_PTR;
}

}
