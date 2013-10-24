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

#if ENABLE(WEBCL)

#include "WebCLInputChecker.h"
#include "WebCLKernel.h"

namespace WebCore {
namespace WebCLInputChecker {

bool isValidDeviceType(unsigned long deviceType)
{
    switch (deviceType) {
    case ComputeContext::DEVICE_TYPE_CPU:
    case ComputeContext::DEVICE_TYPE_GPU:
    case ComputeContext::DEVICE_TYPE_ACCELERATOR:
    case ComputeContext::DEVICE_TYPE_DEFAULT:
    case ComputeContext::DEVICE_TYPE_ALL:
        return true;
    }
    return false;
}

bool isValidDeviceInfoType(unsigned long infoType)
{
    switch (infoType) {
    case ComputeContext::DEVICE_EXTENSIONS:
    case ComputeContext::DEVICE_PROFILE:
    case ComputeContext::DEVICE_VERSION:
    case ComputeContext::DEVICE_OPENCL_C_VERSION:
    case ComputeContext::DEVICE_ADDRESS_BITS:
    case ComputeContext::DEVICE_MAX_CONSTANT_ARGS:
    case ComputeContext::DEVICE_MAX_READ_IMAGE_ARGS:
    case ComputeContext::DEVICE_MAX_SAMPLERS:
    case ComputeContext::DEVICE_MAX_WRITE_IMAGE_ARGS:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_INT:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
    case ComputeContext::DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_INT:
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_LONG:
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
    case ComputeContext::DEVICE_NATIVE_VECTOR_WIDTH_HALF:
    case ComputeContext::DEVICE_IMAGE2D_MAX_HEIGHT:
    case ComputeContext::DEVICE_IMAGE2D_MAX_WIDTH:
    case ComputeContext::DEVICE_MAX_PARAMETER_SIZE:
    case ComputeContext::DEVICE_MAX_WORK_GROUP_SIZE:
    case ComputeContext::DEVICE_MAX_WORK_ITEM_DIMENSIONS:
    case ComputeContext::DEVICE_LOCAL_MEM_SIZE:
    case ComputeContext::DEVICE_MAX_CONSTANT_BUFFER_SIZE:
    case ComputeContext::DEVICE_MAX_MEM_ALLOC_SIZE:
    case ComputeContext::DEVICE_AVAILABLE:
    case ComputeContext::DEVICE_ENDIAN_LITTLE:
    case ComputeContext::DEVICE_HOST_UNIFIED_MEMORY:
    case ComputeContext::DEVICE_IMAGE_SUPPORT:
    case ComputeContext::DEVICE_TYPE:
    case ComputeContext::DEVICE_QUEUE_PROPERTIES:
    case ComputeContext::DEVICE_PLATFORM: // Platform ID is not supported.
    case ComputeContext::DEVICE_LOCAL_MEM_TYPE:
    case ComputeContext::DEVICE_MAX_WORK_ITEM_SIZES:
    case ComputeContext::DEVICE_MAX_COMPUTE_UNITS:
    case ComputeContext::DEVICE_GLOBAL_MEM_SIZE:
    case ComputeContext::DEVICE_MEM_BASE_ADDR_ALIGN:
    case ComputeContext::DEVICE_SINGLE_FP_CONFIG:
    case ComputeContext::DEVICE_COMPILER_AVAILABLE:
    case ComputeContext::DEVICE_EXECUTION_CAPABILITIES:
    case ComputeContext::DEVICE_ERROR_CORRECTION_SUPPORT:
    case ComputeContext::DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
    case ComputeContext::DEVICE_GLOBAL_MEM_CACHE_TYPE:
    case ComputeContext::DEVICE_PROFILING_TIMER_RESOLUTION:
    case ComputeContext::DEVICE_GLOBAL_MEM_CACHE_SIZE:
        return true;
    }
    return false;
}

bool isValidMemoryObjectFlag(unsigned long memoryObjectFlag)
{
    switch (memoryObjectFlag) {
    case ComputeContext::MEM_READ_ONLY:
    case ComputeContext::MEM_WRITE_ONLY:
    case ComputeContext::MEM_READ_WRITE:
        return true;
    }

    return false;
}

bool isValidAddressingMode(unsigned long value)
{
    switch(value) {
    case ComputeContext::ADDRESS_CLAMP_TO_EDGE:
    case ComputeContext::ADDRESS_CLAMP:
    case ComputeContext::ADDRESS_REPEAT:
    case ComputeContext::ADDRESS_MIRRORED_REPEAT:
        return true;
    }

    return false;
}

bool isValidFilterMode(unsigned long value)
{
    switch(value) {
    case ComputeContext::FILTER_NEAREST:
    case ComputeContext::FILTER_LINEAR:
        return true;
    }

    return false;

}

bool isValidChannelOrder(unsigned long value)
{
    switch (value) {
    case ComputeContext::R:
    case ComputeContext::A:
    case ComputeContext::RG:
    case ComputeContext::RA:
    case ComputeContext::RGB:
    case ComputeContext::RGBA:
    case ComputeContext::BGRA:
    case ComputeContext::ARGB:
    case ComputeContext::INTENSITY:
    case ComputeContext::LUMINANCE:
    case ComputeContext::Rx:
    case ComputeContext::RGx:
    case ComputeContext::RGBx:
        return true;
    }

    return false;
}

bool isValidChannelType(unsigned long value)
{
    switch (value) {
    case ComputeContext::SNORM_INT8:
    case ComputeContext::SNORM_INT16:
    case ComputeContext::UNORM_INT8:
    case ComputeContext::UNORM_INT16:
    case ComputeContext::UNORM_SHORT_565:
    case ComputeContext::UNORM_SHORT_555:
    case ComputeContext::UNORM_INT_101010:
    case ComputeContext::SIGNED_INT8:
    case ComputeContext::SIGNED_INT16:
    case ComputeContext::SIGNED_INT32:
    case ComputeContext::UNSIGNED_INT8:
    case ComputeContext::UNSIGNED_INT16:
    case ComputeContext::UNSIGNED_INT32:
    case ComputeContext::HALF_FLOAT:
    case ComputeContext::FLOAT:
        return true;
    }

    return false;
}

bool isValidCommandQueueProperty(unsigned long value)
{
    switch (value) {
    case 0: // 0 as integer value CommandQueueProperty is optional.
    case ComputeContext::QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
    case ComputeContext::QUEUE_PROFILING_ENABLE:
        return true;
    }
    return false;
}

bool isValidGLTextureInfo(unsigned long value)
{
    switch (value) {
    case ComputeContext::GL_TEXTURE_TARGET:
    case ComputeContext::GL_MIPMAP_LEVEL:
        return true;
    }
    return false;
}

bool isValidKernelArgIndex(WebCLKernel* kernel, unsigned index)
{
    ASSERT(kernel);
    return index < kernel->numberOfArguments();
}

}
}

#endif
