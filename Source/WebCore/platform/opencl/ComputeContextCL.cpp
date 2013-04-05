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
    m_clContext = clCreateContext(contextProperties, devices.size(), devices.data(), 0, 0, &error);
}

ComputeContext::ComputeContext(CCContextProperties* contextProperties, unsigned deviceType, CCerror& error)
{
    switch (deviceType) {
    case DEVICE_TYPE_GPU:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, 0, 0, &error);
        break;
    case DEVICE_TYPE_CPU:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, 0, 0, &error);
        break;
    case DEVICE_TYPE_ACCELERATOR:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ACCELERATOR, 0, 0, &error);
        break;
    case DEVICE_TYPE_DEFAULT:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_DEFAULT, 0, 0, &error);
        break;
    case DEVICE_TYPE_ALL:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ALL, 0, 0, &error);
        break;
    default:
        error = CL_INVALID_DEVICE_TYPE;
        break;
    }
}

ComputeContext::~ComputeContext()
{
    clReleaseContext(m_clContext);
}

CCerror ComputeContext::getPlatformIDs(Vector<CCPlatformID>& ccPlatforms)
{
    cl_uint numberOfPlatforms;
    cl_int clError = clGetPlatformIDs(0, 0, &numberOfPlatforms);
    if (clError != CL_SUCCESS)
        return clError;

    if (!ccPlatforms.tryReserveCapacity(numberOfPlatforms))
        return OUT_OF_HOST_MEMORY;
    ccPlatforms.resize(numberOfPlatforms);

    clError = clGetPlatformIDs(numberOfPlatforms, ccPlatforms.data(), 0);
    return clError;
}

CCerror ComputeContext::getDeviceIDs(CCPlatformID platform, CCDeviceType deviceType, Vector<CCDeviceID>& ccDevices)
{
    cl_uint numberOfDevices;
    cl_int clError = clGetDeviceIDs(platform, deviceType, 0, 0, &numberOfDevices);
    if (clError != CL_SUCCESS)
        return clError;

    if (!ccDevices.tryReserveCapacity(numberOfDevices))
        return OUT_OF_HOST_MEMORY;
    ccDevices.resize(numberOfDevices);

    clError = clGetDeviceIDs(platform, deviceType, numberOfDevices, ccDevices.data(), 0);
    return clError;
}

CCerror ComputeContext::waitForEvents(const Vector<CCEvent>& events)
{
    cl_int clError = clWaitForEvents(events.size(), events.data());
    return clError;
}

CCCommandQueue ComputeContext::createCommandQueue(CCDeviceID deviceId, int properties, CCerror& error)
{
    cl_command_queue_properties clProperties;
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
        error = CL_INVALID_QUEUE_PROPERTIES;
        return 0;
    }

    cl_command_queue clCommandQueue = clCreateCommandQueue(m_clContext, deviceId, clProperties, &error);
    return clCommandQueue;
}

CCEvent ComputeContext::createUserEvent(CCerror& error)
{
    cl_event event = clCreateUserEvent(m_clContext, &error);
    return event;
}

CCProgram ComputeContext::createProgram(const String& kernelSource, CCerror& error)
{
    const CString& kernelSourceCString = kernelSource.utf8();
    const char* kernelSourcePtr = kernelSourceCString.data();
    cl_program clProgram = clCreateProgramWithSource(m_clContext, 1, &kernelSourcePtr, 0, &error);
    return clProgram;
}

CCerror ComputeContext::buildProgram(CCProgram program, const Vector<CCDeviceID>& devices, const String& options, pfnNotify notifyFunction, void* userData)
{
    const CString& optionsCString = options.utf8();
    const char* optionsPtr = optionsCString.data();
    cl_int clError = clBuildProgram(program, devices.size(), devices.data(), optionsPtr, notifyFunction, userData);
    return clError;
}

CCerror ComputeContext::setKernelArg(CCKernel kernel, CCuint argIndex, size_t argSize, const void* argValue)
{
    cl_int clError = clSetKernelArg(kernel, argIndex, argSize, argValue);
    return clError;
}

PlatformComputeObject ComputeContext::createBuffer(int memoryFlags, size_t size, void* data, CCerror& error)
{
    cl_mem clMemoryBuffer;
    cl_int clMemoryFlags = memoryFlags;

    clMemoryBuffer = clCreateBuffer(m_clContext, clMemoryFlags, size, data, &error);
    return clMemoryBuffer;
}

PlatformComputeObject ComputeContext::createSubBuffer(PlatformComputeObject buffer, int memoryFlags, int bufferCreatetype,
    CCBufferRegion* bufferCreateInfo, CCerror& error)
{
    cl_int clMemoryFlags = memoryFlags;
    cl_buffer_create_type clBufferCreateType = bufferCreatetype;

    cl_mem clMemoryBuffer = clCreateSubBuffer(buffer, clMemoryFlags, clBufferCreateType, bufferCreateInfo, &error);
    return clMemoryBuffer;
}

// FIXME: ComputeContext::createImage
PlatformComputeObject ComputeContext::createImage2D(int memoryFlags, int width, int height, const CCImageFormat& imageFormat, void* data, CCerror& error)
{
    cl_mem clMemoryImage = 0;
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
        data, &error);
#else
    clMemoryImage = clCreateImage2D(m_clContext, clMemoryFlags, &imageFormat, width, height, 0,
        data, &error);
#endif

    return clMemoryImage;
}

PlatformComputeObject ComputeContext::createFromGLBuffer(int type, int bufferId, CCerror& error)
{
    cl_mem clMemoryImage = 0;
    cl_int memoryType = computeMemoryTypeToCL(type);

    clMemoryImage = clCreateFromGLBuffer(m_clContext, memoryType, bufferId, &error);
    return clMemoryImage;
}

PlatformComputeObject ComputeContext::createFromGLRenderbuffer(int type, GC3Dint renderbufferId, CCerror& error)
{
    cl_mem clMemory;
    cl_int memoryType = computeMemoryTypeToCL(type);

    clMemory = clCreateFromGLRenderbuffer(m_clContext, memoryType, renderbufferId, &error);
    return clMemory;
}

CCSampler ComputeContext::createSampler(bool normalizedCoords, int addressingMode, int filterMode, CCerror& error)
{
    cl_sampler sampler = clCreateSampler(m_clContext, normalizedCoords, addressingMode, filterMode, &error);
    return sampler;
}

PlatformComputeObject ComputeContext::createFromGLTexture2D(int type, GC3Denum textureTarget, GC3Dint mipLevel, GC3Duint texture, CCerror& error)
{
    cl_int memoryType = computeMemoryTypeToCL(type);
    PlatformComputeObject memory;

#if defined(CL_VERSION_1_2)
    memory = clCreateFromGLTexture(m_clContext, memoryType, textureTarget, mipLevel, texture, &error);
#else
    memory = clCreateFromGLTexture2D(m_clContext, memoryType, textureTarget, mipLevel, texture, &error);
#endif
    return memory;
}

CCerror ComputeContext::supportedImageFormats(int memoryFlags, int imageType, Vector<CCImageFormat>& imageFormatsOut)
{
    cl_uint numberOfSupportedImageFormats = 0;
    cl_int clError = clGetSupportedImageFormats(m_clContext, memoryFlags, imageType, 0, 0, &numberOfSupportedImageFormats);
    if (clError != CL_SUCCESS)
        return clError;

    if (!imageFormatsOut.tryReserveCapacity(numberOfSupportedImageFormats))
        return OUT_OF_HOST_MEMORY;
    imageFormatsOut.resize(numberOfSupportedImageFormats);
    
    clError = clGetSupportedImageFormats(m_clContext, memoryFlags, imageType, numberOfSupportedImageFormats, imageFormatsOut.data(), 0);
    return clError;
}

CCerror ComputeContext::getDeviceInfoBase(CCDeviceID deviceID, int infoType, size_t sizeOfData, void* data, size_t* retSize)
{
   return clGetDeviceInfo(deviceID, infoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::getPlatformInfoBase(CCPlatformID platformID, int infoType, size_t sizeOfData, void* data, size_t *retSize)
{
   return clGetPlatformInfo(platformID, infoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::getProgramInfoBase(CCProgram program, int infoType, size_t sizeOfData, void* data, size_t* actualSizeOfData)
{
   cl_int error = clGetProgramInfo(program, infoType, sizeOfData, data, actualSizeOfData);
   return error;
}

CCerror ComputeContext::getBuildInfo(CCProgram program, CCDeviceID device, int infoType, size_t sizeOfData, void* data)
{
    cl_program_build_info clProgramBuildInfoType = infoType;
    cl_int error = clGetProgramBuildInfo(program, device, clProgramBuildInfoType, sizeOfData, data, 0);
    return error;
}

CCerror ComputeContext::getCommandQueueInfoBase(CCCommandQueue queue, int infoType, size_t sizeOfData, void* data, size_t* retSize)
{
   return clGetCommandQueueInfo(queue, infoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::getEventInfoBase(CCEvent event, int infoType, size_t sizeOfData, void* data, size_t* retSize)
{
    return clGetEventInfo(event, infoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::getEventProfilingInfoBase(CCEvent event, int infoType, size_t sizeOfData, void* data, size_t* retSize)
{
    return clGetEventProfilingInfo(event, infoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::getImageInfoBase(PlatformComputeObject image, int infoType, size_t sizeOfData, void* data, size_t* retSize)
{
    cl_image_info clImageInfoType = infoType;
    return clGetImageInfo(image, clImageInfoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::getGLTextureInfoBase(PlatformComputeObject image, int textureInfoType, size_t sizeOfData, void* data, size_t* retSize)
{
    cl_gl_texture_info clglTextureInfoType = textureInfoType;
    return clGetGLTextureInfo(image, clglTextureInfoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::getKernelInfoBase(CCKernel kernel, int infoType, size_t sizeOfData, void* data, size_t* retSize)
{
    cl_kernel_info clKernelInfoType = infoType;
    return clGetKernelInfo(kernel, clKernelInfoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::getWorkGroupInfo(CCKernel kernel, CCDeviceID device, int infoType, size_t sizeOfData, void* data)
{
    cl_kernel_work_group_info clKernelWorkGroupInfoType = infoType;
    return clGetKernelWorkGroupInfo(kernel, device, clKernelWorkGroupInfoType, sizeOfData, data, 0);
}

CCerror ComputeContext::getSamplerInfoBase(CCSampler sampler, int infoType, size_t sizeOfData, void* data, size_t* retSize)
{
    cl_sampler_info clSamplerInfoType = infoType;
    return clGetSamplerInfo(sampler, clSamplerInfoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::getMemoryObjectInfoBase(PlatformComputeObject memObject, int infoType, size_t sizeOfData, void* data, size_t* retSize)
{
    cl_mem_info clMemInfoType = infoType;
    return clGetMemObjectInfo(memObject, clMemInfoType, sizeOfData, data, retSize);
}

CCerror ComputeContext::releaseKernel(CCKernel kernel)
{
    cl_int error = clReleaseKernel(kernel);
    return error;
}

CCerror ComputeContext::releaseEvent(CCEvent ccevent)
{
    cl_int error = clReleaseEvent(ccevent);
    return error;
}

CCerror ComputeContext::releaseSampler(CCSampler sampler)
{
    cl_int error = clReleaseSampler(sampler);
    return error;
}

CCerror ComputeContext::releaseMemoryObject(PlatformComputeObject memmory)
{
    cl_int error = clReleaseMemObject(memmory);
    return error;
}

CCerror ComputeContext::releaseProgram(CCProgram program)
{
    cl_int error = clReleaseProgram(program);
    return error;
}

CCerror ComputeContext::enqueueNDRangeKernel(CCCommandQueue commandQueue, CCKernel kernelID, int workItemDimensions,
    size_t* globalWorkOffset, size_t* globalWorkSize, size_t* localWorkSize, int eventWaitListLength, CCEvent* eventWaitList, CCEvent* event)
{
    cl_int error = clEnqueueNDRangeKernel(commandQueue, kernelID, workItemDimensions,
        globalWorkOffset, globalWorkSize, localWorkSize, eventWaitListLength, eventWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueBarrier(CCCommandQueue commandQueue)
{
    cl_int error;
#if defined(CL_VERSION_1_2)
    error = clEnqueueBarrierWithWaitList(commandQueue, 0 /*eventWaitListLength*/, 0 /*eventsWaitList*/, 0 /*event*/);
#else
    error = clEnqueueBarrier(commandQueue);
#endif
    return error;
}

CCerror ComputeContext::enqueueMarker(CCCommandQueue commandQueue, CCEvent* event)
{
    cl_int error;
#if defined(CL_VERSION_1_2)
    error = clEnqueueMarkerWithWaitList(commandQueue, 0 /*eventsWaitListLength*/, 0 /*eventsWaitList*/, event);
#else
    error = clEnqueueMarker(commandQueue, event);
#endif
    return error;
}

CCerror ComputeContext::enqueueTask(CCCommandQueue commandQueue, CCKernel kernelID, int eventsWaitListLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueTask(commandQueue, kernelID, eventsWaitListLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueWriteBuffer(CCCommandQueue commandQueue, PlatformComputeObject buffer, bool blockingWrite,
    int offset, int bufferSize, void* baseAddress, unsigned eventsWaitListLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueWriteBuffer(commandQueue, buffer, blockingWrite, offset,
        bufferSize, baseAddress, eventsWaitListLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueWriteBufferRect(CCCommandQueue commandQueue, PlatformComputeObject buffer, bool blockingWrite,
    size_t* bufferOriginArray, size_t* hostOriginArray, size_t* regionArray, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch,
    int hostSlicePitch, void* baseAddress, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueWriteBufferRect(commandQueue, buffer, blockingWrite, bufferOriginArray, hostOriginArray, regionArray,
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, baseAddress, eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueReadBuffer(CCCommandQueue commandQueue, PlatformComputeObject buffer, bool blockingRead, int offset, int bufferSize, void* baseAddress, unsigned eventsWaitListLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueReadBuffer(commandQueue, buffer, blockingRead, offset,
        bufferSize, baseAddress, eventsWaitListLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueReadBufferRect(CCCommandQueue commandQueue, PlatformComputeObject buffer, bool blockingRead,
    size_t* bufferOriginArray, size_t* hostOriginArray, size_t* regionArray, int bufferRowPitch, int bufferSlicePitch, int hostRowPitch,
    int hostSlicePitch, void* baseAddress, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueReadBufferRect(commandQueue, buffer, blockingRead, bufferOriginArray, hostOriginArray, regionArray,
        bufferRowPitch, bufferSlicePitch, hostRowPitch, hostSlicePitch, baseAddress, eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueReadImage(CCCommandQueue commandQueue, PlatformComputeObject image, bool blockingRead, size_t* originArray,
    size_t* regionArray, int rowPitch, int slicePitch, void* baseAddress, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueReadImage(commandQueue, image, blockingRead, originArray, regionArray, rowPitch, slicePitch, baseAddress, eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueWriteImage(CCCommandQueue commandQueue, PlatformComputeObject image, bool blockingWrite, size_t* originArray,
    size_t* regionArray, int rowPitch, int slicePitch, void* baseAddress, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueWriteImage(commandQueue, image, blockingWrite, originArray, regionArray, rowPitch, slicePitch, baseAddress, eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueAcquireGLObjects(CCCommandQueue commandQueue, unsigned numberOfObjects, PlatformComputeObject* objects, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueAcquireGLObjects(commandQueue, numberOfObjects, objects, eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueReleaseGLObjects(CCCommandQueue commandQueue, unsigned numberOfObjects, PlatformComputeObject* objects, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueReleaseGLObjects(commandQueue, numberOfObjects, objects, eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueCopyImage(CCCommandQueue commandQueue, PlatformComputeObject originImage, PlatformComputeObject targetImage,
    size_t* sourceOriginArray, size_t* targetOriginArray, size_t* regionArray, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyImage(commandQueue, originImage, targetImage, sourceOriginArray, targetOriginArray, regionArray,
        eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueCopyImageToBuffer(CCCommandQueue commandQueue, PlatformComputeObject sourceImage, PlatformComputeObject targetBuffer,
    size_t* sourceOriginArray, size_t* regionArray, int targetOffset, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyImageToBuffer(commandQueue, sourceImage, targetBuffer, sourceOriginArray, regionArray, targetOffset,
        eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueCopyBufferToImage(CCCommandQueue commandQueue, PlatformComputeObject sourceBuffer, PlatformComputeObject targetImage,
    int srcOffset, size_t* targetOriginArray, size_t* regionArray, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyBufferToImage(commandQueue, sourceBuffer, targetImage, srcOffset, targetOriginArray, regionArray,
        eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueCopyBuffer(CCCommandQueue commandQueue, PlatformComputeObject sourceBuffer, PlatformComputeObject targetBuffer,
    size_t sourceOffset, size_t targetOffset, size_t sizeInBytes, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyBuffer(commandQueue, sourceBuffer, targetBuffer, sourceOffset, targetOffset, sizeInBytes,
        eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::enqueueCopyBufferRect(CCCommandQueue commandQueue, PlatformComputeObject sourceBuffer, PlatformComputeObject targetBuffer,
    size_t* sourceOriginArray, size_t* targetOriginArray, size_t* regionArray, int sourceRowPitch, int sourceSlicePitch, int targetRowPitch,
    int targetSlicePitch, unsigned eventsLength, CCEvent* eventsWaitList, CCEvent* event)
{
    cl_int error = clEnqueueCopyBufferRect(commandQueue, sourceBuffer, targetBuffer, sourceOriginArray, targetOriginArray, regionArray,
        sourceRowPitch, sourceSlicePitch, targetRowPitch, targetSlicePitch, eventsLength, eventsWaitList, event);
    return error;
}

CCerror ComputeContext::releaseCommandQueue(CCCommandQueue commandQueue)
{
    cl_int error = clReleaseCommandQueue(commandQueue);
    return error;
}

CCerror ComputeContext::finishCommandQueue(CCCommandQueue commandQueue)
{
    cl_int error = clFinish(commandQueue);
    return error;
}

CCerror ComputeContext::flushCommandQueue(CCCommandQueue commandQueue)
{
    cl_int error = clFlush(commandQueue);
    return error;
}

CCKernel ComputeContext::createKernel(CCProgram program, const String& kernelName, CCerror& error)
{
    cl_kernel kernel = clCreateKernel(program, kernelName.utf8().data(), &error);
    return kernel;
}

Vector<CCKernel> ComputeContext::createKernelsInProgram(CCProgram program, CCerror& error)
{
    cl_uint numberOfKernels = 0;
    Vector<cl_kernel> kernels;
    error = clCreateKernelsInProgram(program, 0, 0, &numberOfKernels);
    if (error != CL_SUCCESS)
        return kernels;

    if (!numberOfKernels) {
        // FIXME: Having '0' kernels is an error?
        return kernels;
    }
    if (!kernels.tryReserveCapacity(numberOfKernels)) {
        error = OUT_OF_HOST_MEMORY;
        return kernels;
    }
    kernels.resize(numberOfKernels);

    error = clCreateKernelsInProgram(program, numberOfKernels, kernels.data(), 0);
    return kernels;
}

}
