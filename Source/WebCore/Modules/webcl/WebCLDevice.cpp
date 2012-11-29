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

#include "config.h"

#if ENABLE(WEBCL)

#include "WebCL.h"
#include "WebCLDevice.h"
#include "WebCLException.h"

namespace WebCore {

WebCLDevice::~WebCLDevice()
{
}

PassRefPtr<WebCLDevice> WebCLDevice::create(cl_device_id device_id)
{
    return adoptRef(new WebCLDevice(device_id));
}

WebCLDevice::WebCLDevice(cl_device_id device_id)
    : m_cl_device_id(device_id)
{
}

WebCLGetInfo  WebCLDevice::getInfo(int device_type, ExceptionCode& ec)
{
    cl_int err = 0;
    char device_string[1024];
    cl_uint uint_units = 0;
    size_t sizet_units = 0;
    size_t sizet_array[1024] = {0};
    cl_ulong  ulong_units = 0;
    cl_bool bool_units = false;
    cl_device_type type = 0;
    cl_device_mem_cache_type global_type = 0;
    cl_command_queue_properties queue_properties = 0;
    cl_device_exec_capabilities exec = NULL;
    cl_device_local_mem_type local_type = 0;
    cl_device_fp_config fp_config;
    //cl_platform_id platform_id;
    char platform_id[1024];
    if (m_cl_device_id == NULL) {
        printf("Error: Invalid Device ID\n");
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }

    switch(device_type)
    {

        case WebCL::MEM_READ_ONLY:
            err=clGetDeviceInfo(m_cl_device_id, CL_MEM_READ_ONLY, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_EXTENSIONS:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_EXTENSIONS, sizeof(device_string), &device_string, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(String(device_string));	
            break;
        case WebCL::DEVICE_NAME:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(String(device_string));	
            break;
        case WebCL::DEVICE_PROFILE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PROFILE, sizeof(device_string), &device_string, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(String(device_string));	
            break;
        case WebCL::DEVICE_VENDOR:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_VENDOR, sizeof(device_string), &device_string, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(String(device_string));
            break;
        case WebCL::DEVICE_VERSION:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_VERSION, sizeof(device_string), &device_string, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(String(device_string));
            break;
        case WebCL::DEVICE_OPENCL_C_VERSION:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_OPENCL_C_VERSION, sizeof(device_string), &device_string, NULL);
            if (err == CL_SUCCESS)
            return WebCLGetInfo(String(device_string));
            break;
                            
        case WebCL::DRIVER_VERSION:
            err=clGetDeviceInfo(m_cl_device_id, CL_DRIVER_VERSION, sizeof(device_string), &device_string, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(String(device_string));	
            break;
        case WebCL::DEVICE_ADDRESS_BITS:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_ADDRESS_BITS , sizeof(cl_uint), &uint_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_MAX_CLOCK_FREQUENCY:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
            break;
        case WebCL::DEVICE_MAX_CONSTANT_ARGS:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_MAX_READ_IMAGE_ARGS:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
            break;
        case WebCL::DEVICE_MAX_SAMPLERS:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_SAMPLERS, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_MAX_WRITE_IMAGE_ARGS:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_MEM_BASE_ADDR_ALIGN:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_VENDOR_ID:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &uint_units,NULL);

            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &uint_units,NULL);

            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_INT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
            
            //Saurabh
        case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT, sizeof(cl_uint), &uint_units,NULL);
            
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
            
        case WebCL::DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
            
                    
        case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
           
        case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_INT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_LONG:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_NATIVE_VECTOR_WIDTH_HALF:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF, sizeof(cl_uint), &uint_units,NULL);
            if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
                            

        case WebCL::DEVICE_MAX_COMPUTE_UNITS:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_COMPUTE_UNITS , sizeof(cl_uint), &uint_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::DEVICE_IMAGE2D_MAX_HEIGHT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::DEVICE_IMAGE2D_MAX_WIDTH:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::DEVICE_IMAGE3D_MAX_DEPTH:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::DEVICE_IMAGE3D_MAX_HEIGHT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::DEVICE_IMAGE3D_MAX_WIDTH:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::DEVICE_MAX_PARAMETER_SIZE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_PARAMETER_SIZE, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::DEVICE_MAX_WORK_GROUP_SIZE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::DEVICE_MAX_WORK_ITEM_SIZES:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &sizet_units, NULL);
            if(err == CL_SUCCESS) {
                err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, 1024, &sizet_array, NULL);
                // TODO (siba samal) Check support for SizeTArray/Int16Array in WebCLGetInfo
                if (err == CL_SUCCESS) {
                    int values[3] = {0,0,0};
                    for(int i=0; i<((int)sizet_units); i++)
                    {
                        printf("%d\n", values[i]);
                        values[i] = (int)sizet_array[i];
                    }
                    return WebCLGetInfo(Int32Array::create(values, 3));


                }
            }
            break;
        case WebCL::DEVICE_PROFILING_TIMER_RESOLUTION:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PROFILING_TIMER_RESOLUTION, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::DEVICE_MAX_WORK_ITEM_DIMENSIONS:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::DEVICE_LOCAL_MEM_SIZE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
            break;
        case WebCL::DEVICE_MAX_CONSTANT_BUFFER_SIZE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
            break;
        case WebCL::DEVICE_MAX_MEM_ALLOC_SIZE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
            break;
        case WebCL::DEVICE_GLOBAL_MEM_CACHE_SIZE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(ulong_units));
            break;
        case WebCL::DEVICE_GLOBAL_MEM_SIZE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(ulong_units));
            break;
        case WebCL::DEVICE_AVAILABLE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_AVAILABLE , sizeof(cl_bool), &bool_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<bool>(bool_units));
            break;
        case WebCL::DEVICE_COMPILER_AVAILABLE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_COMPILER_AVAILABLE , sizeof(cl_bool), &bool_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<bool>(bool_units));
            break;
        case WebCL::DEVICE_ENDIAN_LITTLE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_ENDIAN_LITTLE, sizeof(cl_bool), &bool_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<bool>(bool_units));
            break;
        case WebCL::DEVICE_ERROR_CORRECTION_SUPPORT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_ERROR_CORRECTION_SUPPORT, sizeof(cl_bool), &bool_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<bool>(bool_units));
            break;
        case WebCL::DEVICE_HOST_UNIFIED_MEMORY:
            err=clGetDeviceInfo(m_cl_device_id,CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(cl_bool), &bool_units, NULL);
            if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<bool>(bool_units));
            break;
           
        case WebCL::DEVICE_IMAGE_SUPPORT:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &bool_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<bool>(bool_units));
            break;
        case WebCL::DEVICE_TYPE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(type));
            break;
        case WebCL::DEVICE_QUEUE_PROPERTIES:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_QUEUE_PROPERTIES, 
                    sizeof(cl_command_queue_properties), &queue_properties, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(queue_properties));
            break;


        case WebCL::DEVICE_DOUBLE_FP_CONFIG:
                   fp_config = 0;
        //Part of cl_ext.h (which isn't available in Khronos)
        err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(cl_device_fp_config), &fp_config, NULL);

        if (err == CL_SUCCESS)
        return WebCLGetInfo(static_cast<unsigned int>(fp_config));
        break;		
            
        case WebCL::DEVICE_HALF_FP_CONFIG:
        fp_config = 0;
        //Part of cl_ext.h (which isn't available in Khronos)
        clGetDeviceInfo(m_cl_device_id, CL_DEVICE_HALF_FP_CONFIG, sizeof(cl_device_fp_config), &fp_config, NULL);
        if (err == CL_SUCCESS)
        return WebCLGetInfo(static_cast<unsigned int>(fp_config));
        break;

        case WebCL::DEVICE_SINGLE_FP_CONFIG:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_SINGLE_FP_CONFIG, sizeof(cl_device_fp_config), &fp_config, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(fp_config));
            break;

            //Platform ID is not Supported
        case WebCL::DEVICE_PLATFORM:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_PLATFORM, sizeof(platform_id), &platform_id, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo((String)platform_id);
            break;

        case WebCL::DEVICE_EXECUTION_CAPABILITIES:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(cl_device_exec_capabilities), &exec, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(exec));
            break;
        case WebCL::DEVICE_GLOBAL_MEM_CACHE_TYPE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(cl_device_mem_cache_type), &global_type, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(global_type));
            break;
        case WebCL::DEVICE_LOCAL_MEM_TYPE:
            err=clGetDeviceInfo(m_cl_device_id, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(cl_device_local_mem_type), &local_type, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(local_type));
            break;
        default:
            ec = WebCLException::FAILURE;
            printf("Error:UNSUPPORTED DEVICE TYPE = %d ",device_type);
            return WebCLGetInfo();
    }

    switch (err) {
        case CL_INVALID_DEVICE:
            ec = WebCLException::INVALID_DEVICE;
            printf("Error: CL_INVALID_DEVICE \n");
            break;
        case CL_INVALID_VALUE:
            ec = WebCLException::INVALID_VALUE;
            printf("Error: CL_INVALID_VALUE\n");
            break;
        case CL_OUT_OF_RESOURCES:
            ec = WebCLException::OUT_OF_RESOURCES;
            printf("Error: CL_OUT_OF_RESOURCES \n");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            ec = WebCLException::OUT_OF_HOST_MEMORY;
            printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
            break;
        default:
            printf("Error: Invaild Error Type\n");
            ec = WebCLException::FAILURE;
            break;
    }				
    return WebCLGetInfo();
}

Vector<String> WebCLDevice::getSupportedExtensions(ExceptionCode& ec )
{
    char device_string[1024] = "";
    char extensions[16][64];	
    Vector<String> result;
    int count = 0;
    int word_length = 0;
    int i =0;

    if (m_cl_device_id == NULL) {
        printf("Error: Invalid Device ID\n");
        ec = WebCLException::INVALID_DEVICE;
        return result;
    }
    cl_int err = clGetDeviceInfo(m_cl_device_id, CL_DEVICE_EXTENSIONS, sizeof(device_string), &device_string, NULL);
    if (err != CL_SUCCESS) {
        ec = WebCLException::INVALID_DEVICE;
        return result;	
    }

    while(device_string[i] != '\0')
    {
        while(device_string[i] == ' ')
            ++i;
        while(device_string[i] !=  ' ' && device_string[i] != '\0')
            extensions[count][word_length++] = device_string[i++];
        extensions[count++][word_length] = '\0';  /* Append terminator         */
        word_length = 0;
    }
    for(i = 0 ; i<count ; i++) {
        printf("CL_DEVICE_EXTENSIONS: %s\n",extensions[i]);
        result.append(String(extensions[i]));
    }

    return result;

}


cl_device_id WebCLDevice::getCLDevice()
{
    return m_cl_device_id;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
