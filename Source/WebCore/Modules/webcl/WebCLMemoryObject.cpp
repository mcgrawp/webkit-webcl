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

#include "WebCLMemoryObject.h"
#include "WebCL.h"
#include "WebCLGLObjectInfo.h"
/*
#include "WebCLException.h"
#include "WebCLGLObjectInfo.h"
*/
namespace WebCore {

WebCLMemoryObject::~WebCLMemoryObject()
{
    cl_int err = 0;
    ASSERT(m_cl_mem);

    err = clReleaseMemObject(m_cl_mem);
    if (err != CL_SUCCESS) 
    {
        switch (err) 
        {
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT \n");
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
    } 
    else 
    {
        /*
           unsigned int i;
           for (i = 0; i < m_mem_list.size(); i++) {
           if ((m_mem_list[i].get())->getCLMem() == m_cl_mem) {				
           m_mem_list.remove(i);
           m_num_mems = m_mem_list.size();
           break;
           }
           }
         */
    }
}

PassRefPtr<WebCLMemoryObject> WebCLMemoryObject::create(WebCL* compute_context,cl_mem mem, bool is_shared = false)
{
    return adoptRef(new WebCLMemoryObject(compute_context, mem, is_shared));
}

    WebCLMemoryObject::WebCLMemoryObject(WebCL* compute_context, cl_mem mem, bool is_shared) 
: m_context(compute_context), m_cl_mem(mem), m_shared(is_shared)
{
    //m_num_mems = 0;
}

bool WebCLMemoryObject::isShared()
{
    return m_shared;
}
PassRefPtr<WebCLGLObjectInfo> WebCLMemoryObject::getGLObjectInfo(ExceptionCode& ec)
{
    cl_int err = -1;
    unsigned int *gl_object_type = 0;
    unsigned int *gl_object_name = 0;
    if (m_cl_mem == NULL) {
        ec = WebCL::INVALID_MEM_OBJECT;
        printf("Error: Invalid CLMem\n");
        return NULL;
    }
    err = clGetGLObjectInfo(m_cl_mem, gl_object_type, gl_object_name);
    if(err != CL_SUCCESS)
    {
        switch(err)
        {
            case CL_INVALID_MEM_OBJECT:
                printf("Error: CL_INVALID_MEM_OBJECT\n");
                ec = WebCLException::INVALID_MEM_OBJECT;
                break;

            case CL_INVALID_GL_OBJECT:
                printf("Error: CL_INVALID_GL_OBJECT\n");
                ec = WebCLException::INVALID_GL_OBJECT;
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
                printf("Error: Invalid ERROR Type\n");
                ec = WebCLException::FAILURE;
                break;
        }
    }
    else
    {
        RefPtr<WebCLGLObjectInfo> cl_gl_object_info = WebCLGLObjectInfo::create(gl_object_type, gl_object_name);
        if (cl_gl_object_info != NULL) 
        {
            return cl_gl_object_info;
        }
        else 
        {
            printf("Error:: Error creating WebCLGLObjectInfo object\n");
            return NULL;
        }
    }
    return NULL;
}


WebCLGetInfo WebCLMemoryObject::getInfo(int param_name, ExceptionCode& ec)
{
    cl_int err =0;
    cl_uint uint_units = 0;
    size_t sizet_units = 0;
    cl_mem_object_type mem_type = 0;
    void* mem_ptr = NULL; 
    if (m_cl_mem == NULL) {
        ec = WebCL::INVALID_MEM_OBJECT;
        printf("Error: Invalid CLMem\n");
        return WebCLGetInfo();
    }
    switch(param_name)
    {
        case WebCL::MEM_MAP_COUNT:
            err=clGetMemObjectInfo(m_cl_mem, CL_MEM_MAP_COUNT , sizeof(cl_uint), &uint_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
            break;
        case WebCL::MEM_REFERENCE_COUNT:
            err=clGetMemObjectInfo(m_cl_mem, CL_MEM_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
            break;
        case WebCL::MEM_SIZE:
            err=clGetMemObjectInfo(m_cl_mem, CL_MEM_SIZE, sizeof(size_t), &sizet_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
            break;
        case WebCL::MEM_TYPE:			
            err=clGetMemObjectInfo(m_cl_mem, CL_MEM_TYPE, sizeof(cl_mem_object_type), &mem_type, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(mem_type));
            break;
            /*
        case WebCL::MEM_CONTEXT:			
            err=clGetMemObjectInfo(m_cl_mem, CL_MEM_CONTEXT, sizeof(cl_context), &cl_context_id, NULL);
            contextObj = WebCLContext::create(m_context, cl_context_id);
            if(contextObj == NULL)
            {
                printf("Error: CL Mem context not NULL\n");
                return WebCLGetInfo();
            }
            if (err == CL_SUCCESS)
                return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
            break;
            */
        case WebCL::MEM_HOST_PTR:			
            err=clGetMemObjectInfo(m_cl_mem, CL_MEM_HOST_PTR, sizeof(mem_ptr), &mem_ptr, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(mem_ptr);
            break;
        default:
            printf("Error: Unsupported Mem Info type\n");
            return WebCLGetInfo();
    }
    switch (err) {
        case CL_INVALID_VALUE:
            ec = WebCL::INVALID_VALUE;
            printf("Error: CL_INVALID_VALUE   \n");
            break;
            // TODO (siba samal) Handle CL_INVALID_D3D10_RESOURCE_KHR Case
            //	case CL_INVALID_D
            //	ec = WebCL::INVALID_D;
            //	printf("CL_INVALID_D3D10_RESOURCE_KHR    \n");
            //	break; 
        case CL_INVALID_MEM_OBJECT:
            ec = WebCL::INVALID_MEM_OBJECT;
            printf("Error: CL_INVALID_MEM_OBJECT    \n");
            break;
        case CL_OUT_OF_RESOURCES:
            ec = WebCL::OUT_OF_RESOURCES;
            printf("Error: CL_OUT_OF_RESOURCES   \n");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            ec = WebCL::OUT_OF_HOST_MEMORY;
            printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
            break;
        default:
            ec = WebCL::FAILURE;
            printf("Error: Invaild Error Type\n");
            break;
    }				
    return WebCLGetInfo();
}

cl_mem WebCLMemoryObject::getCLMemoryObject()
{
    return m_cl_mem;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
