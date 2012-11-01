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

#include "WebCLKernel.h"
#include "WebCL.h"
#include "WebCLException.h"



namespace WebCore {

WebCLKernel::~WebCLKernel()
{
    cl_int err = 0;

    ASSERT(m_cl_kernel);

    err = clReleaseKernel(m_cl_kernel);
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_KERNEL :
                printf("Error: CL_INVALID_KERNEL \n");
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES  \n");
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
                break;
            default:
                printf("Error: Invaild Error Type\n");
                break;
        }
    } else {
        for (int i = 0; i < m_num_kernels; i++) {
            if ((m_kernel_list[i].get())->getCLKernel() == m_cl_kernel) {
                m_kernel_list.remove(i);
                m_num_kernels = m_kernel_list.size();
                break;
            }
        }
    }
}

PassRefPtr<WebCLKernel> WebCLKernel::create(WebCLContext* context, cl_kernel kernel)
{
    return adoptRef(new WebCLKernel(context, kernel));
}

WebCLKernel::WebCLKernel(WebCLContext* context, cl_kernel kernel)
    : m_context(context),
      m_cl_kernel(kernel)
{
    m_num_kernels = 0;
}

WebCLGetInfo WebCLKernel::getInfo (int kernel_info, ExceptionCode& ec)
{
    cl_int err = 0;
    char function_name[1024];
    cl_uint uint_units = 0;
    cl_program cl_program_id = NULL;
    RefPtr<WebCLProgram> programObj = NULL;

    if (m_cl_kernel == NULL) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    switch(kernel_info) {
        case WebCL::KERNEL_FUNCTION_NAME:
            err=clGetKernelInfo(m_cl_kernel, CL_KERNEL_FUNCTION_NAME, sizeof(function_name), &function_name, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(String(function_name));
            break;
        case WebCL::KERNEL_NUM_ARGS:
            err=clGetKernelInfo(m_cl_kernel, CL_KERNEL_NUM_ARGS , sizeof(cl_uint), &uint_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
            break;
        case WebCL::KERNEL_REFERENCE_COUNT:
            err=clGetKernelInfo(m_cl_kernel, CL_KERNEL_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));
            break;
        case WebCL::KERNEL_PROGRAM:
            err=clGetKernelInfo(m_cl_kernel, CL_KERNEL_PROGRAM, sizeof(cl_program_id), &cl_program_id, NULL);
            programObj = WebCLProgram::create(m_context, cl_program_id);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(PassRefPtr<WebCLProgram>(programObj));
            break;
            // FIXME: we should not create a context here
            /*        case WebCL::KERNEL_CONTEXT:
            err=clGetKernelInfo(m_cl_kernel, CL_KERNEL_CONTEXT, sizeof(cl_context), &cl_context_id, NULL);
            contextObj = WebCLContext::create(m_context, cl_context_id);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
                break;*/
        default:
            ec = WebCLException::FAILURE;
            printf("Error: Invaild Kernel_info \n");
            return WebCLGetInfo();
    }
    switch (err) {
        case CL_INVALID_KERNEL:
            ec = WebCLException::INVALID_KERNEL;
            printf("Error: CL_INVALID_KERNEL  \n");
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
            ec = WebCLException::FAILURE;
            printf("Error: Invaild Error Type\n");
            break;
    }
    return WebCLGetInfo();
}

WebCLGetInfo WebCLKernel::getWorkGroupInfo( WebCLDevice* device, int param_name, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_device_id cl_device = NULL;
    size_t sizet_units = 0;
    cl_ulong  ulong_units = 0;

    if (m_cl_kernel == NULL) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    if (device != NULL) {
        // FIXME: s/getCLDevice/getComputeContextDevice
        cl_device = device->getCLDevice();
        if (cl_device == NULL) {
            printf("Error: cl_device null\n");
            ec = WebCLException::INVALID_DEVICE;
            return  WebCLGetInfo();
        }
    }
    switch (param_name) {

        case WebCL::KERNEL_WORK_GROUP_SIZE:
            err = clGetKernelWorkGroupInfo(m_cl_kernel, cl_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::KERNEL_COMPILE_WORK_GROUP_SIZE:
            err =clGetKernelWorkGroupInfo(m_cl_kernel, cl_device, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t), &sizet_units, NULL);
            return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::KERNEL_LOCAL_MEM_SIZE:
            err =clGetKernelWorkGroupInfo(m_cl_kernel, cl_device, CL_KERNEL_LOCAL_MEM_SIZE, sizeof(cl_ulong), &ulong_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned long>(ulong_units));
            break;
        default:
            printf("Error: Unsupported Kernrl Info type\n");
            ec = WebCLException::INVALID_VALUE;
            return WebCLGetInfo();
    }

    printf("Error: clGetKerelWorkGroupInfo\n");
    switch (err) {
        case CL_INVALID_DEVICE:
            ec = WebCLException::INVALID_DEVICE;
            printf("Error: CL_INVALID_DEVICE\n");
            break;
        case CL_INVALID_VALUE:
            ec = WebCLException::INVALID_VALUE;
            printf("Error: CL_INVALID_VALUE\n");
            break;
        case CL_INVALID_KERNEL:
            ec = WebCLException::INVALID_KERNEL;
            printf("Error: CL_INVALID_KERNEL\n");
            break;
        default:
            ec = WebCLException::FAILURE;
            printf("Error: Invaild Error Type\n");
            break;
    }
    return WebCLGetInfo();

}


void WebCLKernel::setKernelArg(unsigned int argIndex,
        PassRefPtr<WebCLKernelTypeValue> kernelObject, int argType , ExceptionCode& ec)
{
    cl_int err = 0;
    RefPtr<WebCLKernelTypeVector> array = NULL;
    char nodeIdCh = NULL;
    unsigned char nodeIduCh = NULL;
    short nodeIdSh = 0;
    unsigned short nodeIduSh = 0;
    int nodeIdInt = 0;
    unsigned int nodeIduInt = 0;
    long nodeIdLong = 0;
    long nodeIduLong = 0;
    float nodeIdFloat = 0;

    if (m_cl_kernel == NULL) {
        ec = WebCLException::INVALID_KERNEL;
        printf("Error: Invalid kernel\n");
        return;
    }	
    // TODO (siba samal) argType & argIndex Validation
    if(kernelObject == NULL)
    {
        ec = WebCLException::INVALID_KERNEL;
        printf("Error: kernelObject null\n");
        return ;
    }
    switch (argType) {
        case WebCL::KERNEL_ARG_CHAR:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdCh,
                    argIndex, sizeof(cl_char));		
            break;
        case WebCL::KERNEL_ARG_UCHAR:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduCh,
                    argIndex, sizeof(cl_uchar));								
            break;
        case WebCL::KERNEL_ARG_SHORT:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdSh,
                    argIndex, sizeof(cl_short));				
            break;
        case WebCL::KERNEL_ARG_USHORT:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduSh,
                    argIndex, sizeof(cl_ushort));				
            break;
        case WebCL::KERNEL_ARG_INT:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdInt,
                    argIndex, sizeof(cl_int));							
            break;
        case WebCL::KERNEL_ARG_UINT:

            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduInt,
                    argIndex, sizeof(cl_uint));				
            break;
        case WebCL::KERNEL_ARG_LONG:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdLong,
                    argIndex, sizeof(cl_long));
            break;
        case WebCL::KERNEL_ARG_ULONG:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduLong,
                    argIndex, sizeof(cl_ulong));
            break;
        case WebCL::KERNEL_ARG_FLOAT:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdFloat,
                    argIndex, sizeof(cl_float));									
            break;
        case WebCL::KERNEL_ARG_CHAR2:				
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char2), 2);									
            break;
        case WebCL::KERNEL_ARG_UCHAR2:				
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar2), 2);									
            break;
        case WebCL::KERNEL_ARG_SHORT2:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short2), 2);		
            break;
        case WebCL::KERNEL_ARG_USHORT2:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort2), 2);							
            break;
        case WebCL::KERNEL_ARG_INT2:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int2), 2);	
            break;
        case WebCL::KERNEL_ARG_UINT2:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint2), 2);					
            break;
        case WebCL::KERNEL_ARG_LONG2:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long2), 2);					
            break;
        case WebCL::KERNEL_ARG_ULONG2:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong2), 2);	
            break;
        case WebCL::KERNEL_ARG_FLOAT2:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float2), 2);
            break;
        case WebCL::KERNEL_ARG_CHAR3:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char4), 3);
            break;
        case WebCL::KERNEL_ARG_UCHAR3:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar4), 3);					
            break;
        case WebCL::KERNEL_ARG_SHORT3:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short4), 3);		
            break;
        case WebCL::KERNEL_ARG_USHORT3:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort4), 3);							
            break;
        case WebCL::KERNEL_ARG_INT3:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int4), 3);	
            break;
        case WebCL::KERNEL_ARG_UINT3:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint4), 3);									
            break;
        case WebCL::KERNEL_ARG_LONG3:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long4), 3);					
            break;
        case WebCL::KERNEL_ARG_ULONG3:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong4), 3);					
            break;
        case WebCL::KERNEL_ARG_FLOAT3:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float4), 3);
            break;	
        case WebCL::KERNEL_ARG_CHAR4:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char4), 4);
            break;
        case WebCL::KERNEL_ARG_UCHAR4:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar4), 4);					
            break;
        case WebCL::KERNEL_ARG_SHORT4:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short4), 4);		
            break;
        case WebCL::KERNEL_ARG_USHORT4:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort4), 4);							
            break;
        case WebCL::KERNEL_ARG_INT4:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int4), 4);	
            break;
        case WebCL::KERNEL_ARG_UINT4:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint4), 4);									
            break;
        case WebCL::KERNEL_ARG_LONG4:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long4), 4);					
            break;
        case WebCL::KERNEL_ARG_ULONG4:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong4), 4);					
            break;
        case WebCL::KERNEL_ARG_FLOAT4:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float4), 4);
            break;		
        case WebCL::KERNEL_ARG_CHAR8:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char8), 8);
            break;
        case WebCL::KERNEL_ARG_UCHAR8:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar8), 8);					
            break;
        case WebCL::KERNEL_ARG_SHORT8:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short8), 8);		
            break;
        case WebCL::KERNEL_ARG_USHORT8:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort8), 8);							
            break;
        case WebCL::KERNEL_ARG_INT8:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int8), 8);	
            break;
        case WebCL::KERNEL_ARG_UINT8:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint8), 8);									
            break;
        case WebCL::KERNEL_ARG_LONG8:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long8), 8);					
            break;
        case WebCL::KERNEL_ARG_ULONG8:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong8), 8);					
            break;
        case WebCL::KERNEL_ARG_FLOAT8:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float8), 8);
            break;	
        case WebCL::KERNEL_ARG_CHAR16:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char16), 16);
            break;
        case WebCL::KERNEL_ARG_UCHAR16:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar16), 16);					
            break;
        case WebCL::KERNEL_ARG_SHORT16:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short16), 16);		
            break;
        case WebCL::KERNEL_ARG_USHORT16:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort16), 16);							
            break;
        case WebCL::KERNEL_ARG_INT16:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int16), 16);	
            break;
        case WebCL::KERNEL_ARG_UINT16:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint16), 16);									
            break;
        case WebCL::KERNEL_ARG_LONG16:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long16), 16);					
            break;
        case WebCL::KERNEL_ARG_ULONG16:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong16), 16);					
            break;
        case WebCL::KERNEL_ARG_FLOAT16:
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float16), 16);
            break;	
        case WebCL::KERNEL_ARG_SAMPLER:
            printf("CL_KERNEL_ARG_SAMPLER - Not Handled\n");
            break;			
        default:
            ec = WebCLException::INVALID_ARG_VALUE;
            printf("Error: Invaild Kernel Argument Type\n");
            return;
    }
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_KERNEL:
                ec = WebCLException::INVALID_KERNEL;
                printf("Error: CL_INVALID_KERNEL \n");
                break;
            case CL_INVALID_ARG_INDEX:
                ec = WebCLException::INVALID_ARG_INDEX;
                printf("Error: CL_INVALID_ARG_INDEX \n");
                break;
            case CL_INVALID_ARG_VALUE:
                ec = WebCLException::INVALID_ARG_VALUE;
                printf("Error: CL_INVALID_ARG_VALUE \n");
                break;
            case CL_INVALID_MEM_OBJECT:
                ec = WebCLException::INVALID_MEM_OBJECT;
                printf("Error: CL_INVALID_MEM_OBJECT  \n");
                break;
            case CL_INVALID_SAMPLER:
                ec = WebCLException::INVALID_SAMPLER;
                printf("Error: CL_INVALID_SAMPLER  \n");
                break;
            case CL_INVALID_ARG_SIZE:
                ec = WebCLException::INVALID_ARG_SIZE;
                printf("Error: CL_INVALID_ARG_SIZE  \n");
                break;
            case CL_OUT_OF_RESOURCES:
                ec = WebCLException::OUT_OF_RESOURCES;
                printf("Error: CL_OUT_OF_RESOURCES\n");
                break;
            case CL_OUT_OF_HOST_MEMORY:
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                break;
            default:
                ec = WebCLException::FAILURE;
                printf("Error: Invaild Error Type\n");
                break;
        }
    }
    else
    {
    }
}

void WebCLKernel::setKernelArgGlobal(unsigned int arg_index, WebCLMemoryObject* arg_value, ExceptionCode& ec)
{
    cl_int err = 0;	
    cl_mem cl_mem_id = NULL;

    if (m_cl_kernel == NULL) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    if (arg_value != NULL) {
        cl_mem_id = arg_value->getCLMemoryObject();
        if (cl_mem_id == NULL) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error: cl_mem_id null\n");		
            return;
        }
    }
    err = clSetKernelArg(m_cl_kernel, arg_index, sizeof(cl_mem), &cl_mem_id);
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_KERNEL:
                printf("Error: CL_INVALID_KERNEL \n");
                ec = WebCLException::INVALID_KERNEL;
                break;
            case CL_INVALID_ARG_INDEX:
                printf("Error: CL_INVALID_ARG_INDEX \n");
                ec = WebCLException::INVALID_ARG_INDEX;
                break;
            case CL_INVALID_ARG_VALUE:
                printf("Error: CL_INVALID_ARG_VALUE \n");
                ec = WebCLException::INVALID_ARG_VALUE;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT  \n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_SAMPLER:
                printf("Error: CL_INVALID_SAMPLER  \n");
                ec = WebCLException::INVALID_SAMPLER;
                break;
            case CL_INVALID_ARG_SIZE:
                printf("Error: CL_INVALID_ARG_SIZE  \n");
                ec = WebCLException::INVALID_ARG_SIZE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }

    } else {
        return;
    }
    return;
}

void WebCLKernel::setKernelArgConstant(unsigned int arg_index, WebCLMemoryObject* arg_value, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_mem cl_mem_id = NULL;
    cl_device_id cl_device = NULL;

    if (m_cl_kernel == NULL) {
        ec = WebCLException::INVALID_KERNEL;
        printf("Error: Invalid kernel\n");
        return;
    }
    if (arg_value != NULL) {
        cl_mem_id = arg_value->getCLMemoryObject();
        if (cl_mem_id == NULL) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error: cl_mem_id null\n");	
            return;
        }
    }
    if (m_device_id != NULL) {
        cl_device = m_device_id->getCLDevice();
        cl_ulong max_buffer_size = 0;
        clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &max_buffer_size, NULL);
        cl_uint max_args = 0;
        clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &max_args,NULL);
        // Check for __constant qualifier restrictions
        if ( (sizeof(cl_mem) <= max_buffer_size)  && (arg_index <= max_args))
        {
            err = clSetKernelArg(m_cl_kernel, arg_index, sizeof(cl_mem), &cl_mem_id);
            if (err != CL_SUCCESS) {
                switch (err) {
                    case CL_INVALID_KERNEL:
                        printf("Error: CL_INVALID_KERNEL \n");
                        ec = WebCLException::INVALID_KERNEL;
                        break;
                    case CL_INVALID_ARG_INDEX:
                        printf("Error: CL_INVALID_ARG_INDEX \n");
                        ec = WebCLException::INVALID_ARG_INDEX;
                        break;
                    case CL_INVALID_ARG_VALUE:
                        printf("Error: CL_INVALID_ARG_VALUE \n");
                        ec = WebCLException::INVALID_ARG_VALUE;
                        break;
                    case CL_INVALID_MEM_OBJECT:
                        printf("Error: CL_INVALID_MEM_OBJECT  \n");
                        ec = WebCLException::INVALID_MEM_OBJECT;
                        break;
                    case CL_INVALID_SAMPLER:
                        printf("Error: CL_INVALID_SAMPLER  \n");
                        ec = WebCLException::INVALID_SAMPLER;
                        break;
                    case CL_INVALID_ARG_SIZE:
                        printf("Error: CL_INVALID_ARG_SIZE  \n");
                        ec = WebCLException::INVALID_ARG_SIZE;
                        break;
                    case CL_OUT_OF_RESOURCES:
                        printf("Error: CL_OUT_OF_RESOURCES\n");
                        ec = WebCLException::OUT_OF_RESOURCES;
                        break;
                    case CL_OUT_OF_HOST_MEMORY:
                        printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                        ec = WebCLException::OUT_OF_HOST_MEMORY;
                        break;
                    default:
                        printf("Error: Invaild Error Type\n");
                        ec = WebCLException::FAILURE;
                        break;
                }

                return;
            } else {
                return;
            }
        }
    }
    return;
}

void WebCLKernel::setKernelArgLocal(unsigned int arg_index, unsigned int arg_size, ExceptionCode& ec)
{
    cl_int err = 0;
    if (m_cl_kernel == NULL) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    err = clSetKernelArg(m_cl_kernel, arg_index, arg_size, NULL);
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_KERNEL:
                printf("Error: CL_INVALID_KERNEL \n");
                ec = WebCLException::INVALID_KERNEL;
                break;
            case CL_INVALID_ARG_INDEX:
                printf("Error: CL_INVALID_ARG_INDEX \n");
                ec = WebCLException::INVALID_ARG_INDEX;
                break;
            case CL_INVALID_ARG_VALUE:
                printf("Error: CL_INVALID_ARG_VALUE \n");
                ec = WebCLException::INVALID_ARG_VALUE;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT  \n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_SAMPLER:
                printf("Error: CL_INVALID_SAMPLER  \n");
                ec = WebCLException::INVALID_SAMPLER;
                break;
            case CL_INVALID_ARG_SIZE:
                printf("Error: CL_INVALID_ARG_SIZE  \n");
                ec = WebCLException::INVALID_ARG_SIZE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }

        return;
    } else {
        return;
    }
}

void WebCLKernel::setArg(unsigned int arg_index, unsigned int arg_size, ExceptionCode& ec)
{
    cl_int err = 0;
    if (m_cl_kernel == NULL) {
        printf("Error: Invalid kernel\n");
        ec = WebCLException::INVALID_KERNEL;
        return;
    }
    err = clSetKernelArg(m_cl_kernel, arg_index, arg_size, NULL);
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_KERNEL:
                printf("Error: CL_INVALID_KERNEL \n");
                ec = WebCLException::INVALID_KERNEL;
                break;
            case CL_INVALID_ARG_INDEX:
                printf("Error: CL_INVALID_ARG_INDEX \n");
                ec = WebCLException::INVALID_ARG_INDEX;
                break;
            case CL_INVALID_ARG_VALUE:
                printf("Error: CL_INVALID_ARG_VALUE \n");
                ec = WebCLException::INVALID_ARG_VALUE;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT  \n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_SAMPLER:
                printf("Error: CL_INVALID_SAMPLER  \n");
                ec = WebCLException::INVALID_SAMPLER;
                break;
            case CL_INVALID_ARG_SIZE:
                printf("Error: CL_INVALID_ARG_SIZE  \n");
                ec = WebCLException::INVALID_ARG_SIZE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }

        return;
    } else {
        return;
    }

}

void WebCLKernel::setArg(unsigned int argIndex, PassRefPtr<WebCLKernelTypeValue> kernelObject, unsigned int argType , ExceptionCode& ec)
{
    cl_int err = 0;
    RefPtr<WebCLKernelTypeVector> array = NULL;
    char nodeIdCh = NULL;
    unsigned char nodeIduCh = NULL;
    short nodeIdSh = 0;
    unsigned short nodeIduSh = 0;
    int nodeIdInt = 0;
    unsigned int nodeIduInt = 0;
    long nodeIdLong = 0;
    long nodeIduLong = 0;
    float nodeIdFloat = 0;

    if (m_cl_kernel == NULL) {
        ec = WebCLException::INVALID_KERNEL;
        printf("Error: Invalid kernel\n");
        return;
    }
    // TODO (siba samal) argType & argIndex Validation
    if(kernelObject == NULL)
    {
        ec = WebCLException::INVALID_KERNEL;
        printf("Error: kernelObject null\n");
        return ;
    }


    switch (argType) {
        case WebCL::CHAR:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdCh,
                    argIndex, sizeof(cl_char));
            break;
        case WebCL::UCHAR:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduCh,
                    argIndex, sizeof(cl_uchar));
            break;
        case WebCL::SHORT:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdSh,
                    argIndex, sizeof(cl_short));
            break;
        case WebCL::USHORT:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduSh,
                    argIndex, sizeof(cl_ushort));
            break;
        case WebCL::INT:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdInt,
                    argIndex, sizeof(cl_int));
            break;
        case WebCL::UINT:

            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduInt,
                    argIndex, sizeof(cl_uint));
            break;
        case WebCL::LONG:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdLong,
                    argIndex, sizeof(cl_long));
            break;
        case WebCL::ULONG:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIduLong,
                    argIndex, sizeof(cl_ulong));
            break;
        case WebCL::FLOAT_KERNAL_ARG :
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdFloat,
                    argIndex, sizeof(cl_float));
            break;
        case WebCL::HALF:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdFloat,
                    argIndex, sizeof(cl_half));
            break;
        case WebCL::DOUBLE:
            err = clSetKernelArgPrimitiveType(m_cl_kernel, kernelObject, nodeIdFloat,
                    argIndex, sizeof(cl_double));
            break;

            /*******************************************************************************************************************************************************************/

        case (WebCL::CHAR | WebCL::VEC2) :
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char2), 2);
            break;
        case (WebCL::UCHAR | WebCL::VEC2):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar2), 2);
            break;
        case (WebCL::SHORT | WebCL::VEC2):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short2), 2);
            break;
        case (WebCL::USHORT | WebCL::VEC2):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort2), 2);
            break;
        case (WebCL::INT | WebCL::VEC2):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int2), 2);
            break;
        case (WebCL::UINT | WebCL::VEC2):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint2), 2);
            break;
        case (WebCL::LONG | WebCL::VEC2):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long2), 2);
            break;
        case (WebCL::ULONG | WebCL::VEC2):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong2), 2);
            break;
        case (WebCL::FLOAT_KERNAL_ARG | WebCL::VEC2):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float2), 2);
            break;
            /*case (WebCL::DOUBLE | WebCL::VEC2):
              err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
              argIndex, sizeof(cl_double2), 2);
              break;
              case (WebCL::HALF | WebCL::VEC2):
              err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
              argIndex, sizeof(cl_half2), 2);
              break;*/

            /*****************************************************************************************************************************************************************/


            /*		case (WebCL::CHAR | WebCL::VEC3) :
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char3), 3);
                    break;
                    case (WebCL::UCHAR | WebCL::VEC3):
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar3), 3);
                    break;
                    case (WebCL::SHORT | WebCL::VEC3):
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short3), 3);
                    break;
                    case (WebCL::USHORT | WebCL::VEC3):
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort3), 3);
                    break;
                    case (WebCL::INT | WebCL::VEC3):
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int3), 3);
                    break;
                    case (WebCL::UINT | WebCL::VEC3):
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint3), 3);
                    break;
                    case (WebCL::LONG | WebCL::VEC3):
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long3), 3);
                    break;
                    case (WebCL::ULONG | WebCL::VEC3):
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong3), 3);
                    break;
                    case (WebCL::FLOAT_KERNAL_ARG | WebCL::VEC3):
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float3), 3);
                    break;
                    case (WebCL::DOUBLE | WebCL::VEC3):
                    err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_double2), 2);
                    break;
                    */
            /*case (WebCL::HALF | WebCL::VEC3):
              err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
              argIndex, sizeof(cl_half2), 2);
              break;*/


            /*******************************************************************************************************************************************************************/



        case (WebCL::CHAR | WebCL::VEC4) :
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char4), 4);
            break;
        case (WebCL::UCHAR | WebCL::VEC4):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar4), 4);
            break;
        case (WebCL::SHORT | WebCL::VEC4):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short4), 4);
            break;
        case (WebCL::USHORT | WebCL::VEC4):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort4), 4);
            break;
        case (WebCL::INT | WebCL::VEC4):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int4), 4);
            break;
        case (WebCL::UINT | WebCL::VEC4):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint4), 4);
            break;
        case (WebCL::LONG | WebCL::VEC4):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long4), 4);
            break;
        case (WebCL::ULONG | WebCL::VEC4):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong4), 4);
            break;
        case (WebCL::FLOAT_KERNAL_ARG | WebCL::VEC4):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float4), 4);
            break;
        case (WebCL::DOUBLE | WebCL::VEC4):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_double4), 4);
            break;
            /*case (WebCL::HALF | WebCL::VEC4):
              err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
              argIndex, sizeof(cl_half4), 4);
              break;*/

            /******************************************************************************************************************************************************************/

        case (WebCL::CHAR | WebCL::VEC8) :
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char8), 8);
            break;
        case (WebCL::UCHAR | WebCL::VEC8):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar4), 4);
            break;
        case (WebCL::SHORT | WebCL::VEC8):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short8), 8);
            break;
        case (WebCL::USHORT | WebCL::VEC8):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort8), 8);
            break;
        case (WebCL::INT | WebCL::VEC8):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int8), 8);
            break;
        case (WebCL::UINT | WebCL::VEC8):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint8), 8);
            break;
        case (WebCL::LONG | WebCL::VEC8):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long8), 8);
            break;
        case (WebCL::ULONG | WebCL::VEC8):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong8), 8);
            break;
        case (WebCL::FLOAT_KERNAL_ARG | WebCL::VEC8):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float8), 8);
            break;
        case (WebCL::DOUBLE | WebCL::VEC8):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_double8), 8);
            break;
            /*case (WebCL::HALF | WebCL::VEC8):
              err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
              argIndex, sizeof(cl_half8), 8);
              break;*/


            /*********************************************************************************************************************************************************/




        case (WebCL::CHAR | WebCL::VEC16) :
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_char8), 8);
            break;
        case (WebCL::UCHAR | WebCL::VEC16):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uchar16), 16);
            break;
        case (WebCL::SHORT | WebCL::VEC16):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_short16), 16);
            break;
        case (WebCL::USHORT | WebCL::VEC16):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ushort16), 16);
            break;
        case (WebCL::INT | WebCL::VEC16):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_int16), 16);
            break;
        case (WebCL::UINT | WebCL::VEC16):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_uint16), 16);
            break;
        case (WebCL::LONG | WebCL::VEC16):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_long16), 16);
            break;
        case (WebCL::ULONG | WebCL::VEC16):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_ulong16), 16);
            break;
        case (WebCL::FLOAT_KERNAL_ARG | WebCL::VEC16):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_float16), 16);
            break;
        case (WebCL::DOUBLE | WebCL::VEC16):
            err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
                    argIndex, sizeof(cl_double16), 16);
            break;
            /*case (WebCL::HALF | WebCL::VEC8):
              err = clSetKernelArgVectorType(m_cl_kernel, kernelObject, array,
              argIndex, sizeof(cl_half16), 16);
              break;*/



        case WebCL::KERNEL_ARG_SAMPLER:
            printf("CL_KERNEL_ARG_SAMPLER - Not Handled\n");
            break;
        default:
            ec = WebCLException::INVALID_ARG_VALUE;
            printf("Error: Invaild Kernel Argument Type\n");
            return;
    }
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_KERNEL:
                ec = WebCLException::INVALID_KERNEL;
                printf("Error: CL_INVALID_KERNEL \n");
                break;
            case CL_INVALID_ARG_INDEX:
                ec = WebCLException::INVALID_ARG_INDEX;
                printf("Error: CL_INVALID_ARG_INDEX \n");
                break;
            case CL_INVALID_ARG_VALUE:
                ec = WebCLException::INVALID_ARG_VALUE;
                printf("Error: CL_INVALID_ARG_VALUE \n");
                break;
            case CL_INVALID_MEM_OBJECT:
                ec = WebCLException::INVALID_MEM_OBJECT;
                printf("Error: CL_INVALID_MEM_OBJECT  \n");
                break;
            case CL_INVALID_SAMPLER:
                ec = WebCLException::INVALID_SAMPLER;
                printf("Error: CL_INVALID_SAMPLER  \n");
                break;
            case CL_INVALID_ARG_SIZE:
                ec = WebCLException::INVALID_ARG_SIZE;
                printf("Error: CL_INVALID_ARG_SIZE  \n");
                break;
            case CL_OUT_OF_RESOURCES:
                ec = WebCLException::OUT_OF_RESOURCES;
                printf("Error: CL_OUT_OF_RESOURCES\n");
                break;
            case CL_OUT_OF_HOST_MEMORY:
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                break;
            default:
                ec = WebCLException::FAILURE;
                printf("Error: Invaild Error Type\n");
                break;
        }
    }
    else
    {
    }
}

void WebCLKernel::setArg(unsigned int arg_index, WebCLMemoryObject* arg_value, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_mem cl_mem_id = NULL;
    cl_device_id cl_device = NULL;

    if (m_cl_kernel == NULL) {
        ec = WebCLException::INVALID_KERNEL;
        printf("Error: Invalid kernel\n");
        return;
    }
    if (arg_value != NULL) {
        cl_mem_id = arg_value->getCLMemoryObject();
        if (cl_mem_id == NULL) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            printf("Error: cl_mem_id null\n");
            return;
        }
    }
    if (m_device_id != NULL) {
        cl_device = m_device_id->getCLDevice();
        cl_ulong max_buffer_size = 0;
        clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &max_buffer_size, NULL);
        cl_uint max_args = 0;
        clGetDeviceInfo(cl_device, CL_DEVICE_MAX_CONSTANT_ARGS, sizeof(cl_uint), &max_args,NULL);
        // Check for __constant qualifier restrictions
        if ( (sizeof(cl_mem) <= max_buffer_size)  && (arg_index <= max_args))
        {
            err = clSetKernelArg(m_cl_kernel, arg_index, sizeof(cl_mem), &cl_mem_id);

        }
        else
        {
            err = clSetKernelArg(m_cl_kernel, arg_index, sizeof(cl_mem), &cl_mem_id);
        }
    }
    else
    {

        err = clSetKernelArg(m_cl_kernel, arg_index, sizeof(cl_mem), &cl_mem_id);

    }

    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_KERNEL:
                printf("Error: CL_INVALID_KERNEL \n");
                ec = WebCLException::INVALID_KERNEL;
                break;
            case CL_INVALID_ARG_INDEX:
                printf("Error: CL_INVALID_ARG_INDEX \n");
                ec = WebCLException::INVALID_ARG_INDEX;
                break;
            case CL_INVALID_ARG_VALUE:
                printf("Error: CL_INVALID_ARG_VALUE \n");
                ec = WebCLException::INVALID_ARG_VALUE;
                break;
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT  \n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;
            case CL_INVALID_SAMPLER:
                printf("Error: CL_INVALID_SAMPLER  \n");
                ec = WebCLException::INVALID_SAMPLER;
                break;
            case CL_INVALID_ARG_SIZE:
                printf("Error: CL_INVALID_ARG_SIZE  \n");
                ec = WebCLException::INVALID_ARG_SIZE;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES\n");
                ec = WebCLException::OUT_OF_RESOURCES;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }

    } else {
        return;
    }


    return;
}





/*
// TODO (siba samal) Is this API is needed??
unsigned long WebCLKernel::getKernelWorkGroupInfo(WebCLDeviceList* devices, int param_name)
{
cl_int err = 0;
cl_device_id cl_device = NULL;
size_t ret = 0;
if (m_cl_kernel == NULL) {
printf("Error: Invalid kernel\n");
return NULL;
}
if (devices != NULL) {
cl_device = devices->getCLDevices();
if (cl_device == NULL) {
printf("Error: cl_device null\n");
return NULL;
}
}
switch (param_name) {
case KERNEL_WORK_GROUP_SIZE:
err = clGetKernelWorkGroupInfo(m_cl_kernel, cl_device, CL_KERNEL_WORK_GROUP_SIZE, sizeof(ret), &ret, NULL);
if (err != CL_SUCCESS) {
switch (err) {
case CL_INVALID_DEVICE:
printf("Error: CL_INVALID_DEVICE\n");
break;
case CL_INVALID_VALUE:
printf("Error: CL_INVALID_VALUE\n");
break;
case CL_INVALID_KERNEL:
printf("Error: CL_INVALID_KERNEL\n");
break;
default:
printf("Error: Invaild Error Type\n");
break;
}

} else {

return ret;
}
break;
default:
break;
}
return NULL;
}
*/

template<class T> inline unsigned int WebCLKernel::
clSetKernelArgPrimitiveType(cl_kernel cl_kernel_id,
        PassRefPtr<WebCLKernelTypeValue> kernelObject,
        T nodeId, unsigned int argIndex, int size)
{			
    if(!kernelObject->asNumber(&nodeId))
    {
        printf("Error: WebCL Kernel Type Value Not Proper\n");
        return -1;	
    }
    cl_int err  =  clSetKernelArg(cl_kernel_id, argIndex, size, &nodeId);		
    return(err);
}

inline unsigned int WebCLKernel::
clSetKernelArgVectorType(cl_kernel cl_kernel_id,
        PassRefPtr<WebCLKernelTypeValue> kernelObject,
        RefPtr<WebCLKernelTypeVector> array , unsigned int argIndex,
        int size,unsigned int length)
{		
    if ((!kernelObject->asVector(&array)) || (length != array->length()))
    {
        printf("Error: Invalid WebCL Kernel Type %d\n", array->length());
        return -1;	
    }
    cl_int err = clSetKernelArg(cl_kernel_id, argIndex, size, &array);			
    return(err);
}

void WebCLKernel::setDevice(RefPtr<WebCLDevice> m_device_id_)
{
    m_device_id = m_device_id_;
}

cl_kernel WebCLKernel::getCLKernel()
{
    return m_cl_kernel;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
