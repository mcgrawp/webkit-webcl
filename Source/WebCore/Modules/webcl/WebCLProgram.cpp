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
#include "WebCLContext.h"
#include "WebCLProgram.h"
#include "WebCLGetInfo.h"
#include "WebCLKernel.h"
#include "WebCLKernelList.h"
#include "WebCLException.h"

namespace WebCore {

WebCLProgram::~WebCLProgram()
{
    cl_int err = 0;

    ASSERT(m_cl_program);

    err = clReleaseProgram(m_cl_program);
	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM  \n");
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
		for (int i = 0; i < m_num_programs; i++) {
			if ((m_program_list[i].get())->getCLProgram() == m_cl_program) {
				m_program_list.remove(i);
				m_num_programs = m_program_list.size();
				break;
			}
		}
	}
}

PassRefPtr<WebCLProgram> WebCLProgram::create(WebCLContext* context, cl_program program)
{
	return adoptRef(new WebCLProgram(context, program));
}

WebCLProgram::WebCLProgram(WebCLContext* context, cl_program program)
    : m_context(context),
      m_cl_program(program)
{
    m_num_programs = 0;
    m_num_kernels = 0;
}

WebCLGetInfo WebCLProgram::getInfo(int param_name, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_uint uint_units = 0;
    size_t sizet_units = 0;
    char program_string[4096];
    RefPtr<WebCLDeviceList> deviceList =  NULL;
    size_t szParmDataBytes = 0;
    if (m_cl_program == NULL) {
        ec = WebCLException::INVALID_PROGRAM;
        printf("Error: Invalid program object\n");
        return WebCLGetInfo();
    }

    switch(param_name) {
    case WebCL::PROGRAM_REFERENCE_COUNT:
        err=clGetProgramInfo(m_cl_program, CL_PROGRAM_REFERENCE_COUNT , sizeof(cl_uint), &uint_units, NULL);
    if (err == CL_SUCCESS)
        return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
    break;
    case WebCL::PROGRAM_NUM_DEVICES:
    err=clGetProgramInfo(m_cl_program, CL_PROGRAM_NUM_DEVICES , sizeof(cl_uint), &uint_units, NULL);
    if (err == CL_SUCCESS)
        return WebCLGetInfo(static_cast<unsigned int>(uint_units));
    break;
    case WebCL::PROGRAM_BINARY_SIZES:
        err=clGetProgramInfo(m_cl_program, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &sizet_units, NULL);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned int>(sizet_units));
    break;
    case WebCL::PROGRAM_SOURCE:
        err=clGetProgramInfo(m_cl_program, CL_PROGRAM_SOURCE, sizeof(program_string), &program_string, NULL);
    if (err == CL_SUCCESS)
        return WebCLGetInfo(String(program_string));
    break;
    case WebCL::PROGRAM_BINARIES:
        err=clGetProgramInfo(m_cl_program, CL_PROGRAM_BINARIES, sizeof(program_string), &program_string, NULL);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(String(program_string));
    break;
    /*
      case WebCL::PROGRAM_CONTEXT:
      err=clGetProgramInfo(m_cl_program, CL_PROGRAM_CONTEXT, sizeof(cl_context), &cl_context_id, NULL);
      contextObj = WebCLContext::create(m_context, cl_context_id);
      if(contextObj == NULL)
      {
      printf("Error : CL program context not NULL\n");
      return WebCLGetInfo();
      }
      if (err == CL_SUCCESS)
      return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
      break;
    */
    case WebCL::PROGRAM_DEVICES:
        cl_device_id* cdDevices;
        clGetProgramInfo(m_cl_program, CL_PROGRAM_DEVICES, 0, NULL, &szParmDataBytes);
        if (err == CL_SUCCESS) {
            int nd = szParmDataBytes / sizeof(cl_device_id);
            cdDevices = (cl_device_id*) malloc(szParmDataBytes);
            clGetProgramInfo(m_cl_program, CL_PROGRAM_DEVICES, szParmDataBytes, cdDevices, NULL);
            deviceList = WebCLDeviceList::create(cdDevices, nd);
            printf("Size Vs Size = %lu %d %d \n\n", szParmDataBytes,nd,deviceList->length());
            free(cdDevices);
            cdDevices=NULL;
            return WebCLGetInfo(PassRefPtr<WebCLDeviceList>(deviceList));
        }
    break;

    // TODO (siba samal)- Handle Array of cl_device_id 
    //case WebCL::PROGRAM_DEVICES:
    //  size_t numDevices;
    //  clGetProgramInfo( m_cl_program, CL_PROGRAM_DEVICES, 0, 0, &numDevices );
    //  cl_device_id *devices = new cl_device_id[numDevices];
    //  clGetProgramInfo( m_cl_program, CL_PROGRAM_DEVICES, numDevices, devices, &numDevices );
    //  return WebCLGetInfo(PassRefPtr<WebCLContext>(obj));
    default:
        printf("Error: UNSUPPORTED program Info type = %d ",param_name);
        ec = WebCLException::INVALID_PROGRAM;
    return WebCLGetInfo();
    }
    switch (err) {
    case CL_INVALID_PROGRAM:
    ec = WebCLException::INVALID_PROGRAM;
    printf("Error: CL_INVALID_PROGRAM \n");
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
        ec = WebCLException::INVALID_PROGRAM;
        printf("Error: Invaild Error Type\n");
        break;
    }
    return WebCLGetInfo();
}


WebCLGetInfo WebCLProgram::getBuildInfo(WebCLDevice* device, int param_name, ExceptionCode& ec)
{
	cl_device_id device_id = NULL;
	cl_uint err = 0;
	char buffer[8192];
	size_t len = 0;

	if (m_cl_program == NULL) {
			printf("Error: Invalid program object\n");
			ec = WebCLException::INVALID_PROGRAM;
			return WebCLGetInfo();
	}
	if (device != NULL) {
		device_id = device->getCLDevice();
		if (device_id == NULL) {
			ec = WebCLException::INVALID_DEVICE;
			printf("Error: device_id null\n");
			return WebCLGetInfo();
		}
	}

	switch (param_name) {
		case WebCL::PROGRAM_BUILD_LOG:
			err = clGetProgramBuildInfo(m_cl_program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(buffer));
			break;
		case WebCL::PROGRAM_BUILD_OPTIONS:
			err = clGetProgramBuildInfo(m_cl_program, device_id, CL_PROGRAM_BUILD_OPTIONS, sizeof(buffer), &buffer, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(String(buffer));
			break;
		case WebCL::PROGRAM_BUILD_STATUS:
			cl_build_status build_status;
			err = clGetProgramBuildInfo(m_cl_program, device_id, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);
			if (err == CL_SUCCESS)
				return WebCLGetInfo(static_cast<unsigned int>(build_status));
			break;
		default:
			ec = WebCLException::INVALID_PROGRAM;
			printf("Error: UNSUPPORTED Program Build Info   Type = %d ",param_name);
			return WebCLGetInfo();			
	}
	switch (err) {
		case CL_INVALID_DEVICE:
			ec = WebCLException::INVALID_DEVICE;
			printf("Error: CL_INVALID_DEVICE   \n");
			break;
		case CL_INVALID_VALUE:
			ec = WebCLException::INVALID_VALUE;
			printf("Error: CL_INVALID_VALUE \n");
			break;
		case CL_INVALID_PROGRAM:
			ec = WebCLException::INVALID_PROGRAM;
			printf("Error: CL_INVALID_PROGRAM  \n");
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

PassRefPtr<WebCLKernel> WebCLProgram::createKernel(const String& kernelName, ExceptionCode& ec)
{
    if (m_cl_program == NULL) {
        printf("Error: Invalid program object\n");
        ec = WebCLException::INVALID_PROGRAM;
        return NULL;
    }

    int error;
    CCKernel computeContextKernel  = m_context->computeContext()->createKernel(m_cl_program, kernelName, error);
    if (error != ComputeContext::SUCCESS) {
        ec = error;
        return NULL;
    }

    RefPtr<WebCLKernel> webclKernel = WebCLKernel::create(m_context, computeContextKernel);
    webclKernel->setDevice(m_device_id);
    m_kernel_list.append(webclKernel);
    m_num_kernels++;
    return webclKernel;
}

PassRefPtr<WebCLKernelList> WebCLProgram::createKernelsInProgram( ExceptionCode& ec)
{
    cl_int err = 0;
    cl_kernel* kernelBuf = NULL;
    cl_uint num = 0;
    if (m_cl_program == NULL) {
        printf("Error: Invalid program object\n");
        ec = WebCLException::INVALID_PROGRAM;
        return NULL;
    }
    err = clCreateKernelsInProgram (m_cl_program, NULL, NULL, &num);
    if (err != CL_SUCCESS) {
        //TODO (siba samal) Deatiled error check
        printf("Error: clCreateKernelsInProgram \n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    if(num == 0) {
        printf("Warning: createKernelsInProgram - Number of Kernels is 0 \n");
        ec = WebCLException::FAILURE;
        return NULL;
    }
    // TODO - Free the below malloc
    kernelBuf = (cl_kernel*)malloc (sizeof(cl_kernel) * num);
    if (!kernelBuf) {
        return NULL;
    }

    err = clCreateKernelsInProgram (m_cl_program, num, kernelBuf, NULL);
    if (err != CL_SUCCESS) {
        switch (err) {
        case CL_INVALID_PROGRAM:
            printf("Error: CL_INVALID_PROGRAM\n");
            ec = WebCLException::INVALID_PROGRAM;
            break;
        case CL_INVALID_PROGRAM_EXECUTABLE:
            printf("Error: CL_INVALID_PROGRAM_EXECUTABLE\n");
            ec = WebCLException::INVALID_PROGRAM_EXECUTABLE;
            break;
        case CL_INVALID_VALUE:
            printf("Error: CL_INVALID_VALUE\n");
            ec = WebCLException::INVALID_VALUE;
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
        RefPtr<WebCLKernelList> o = WebCLKernelList::create(m_context, kernelBuf, num);
        printf("WebCLKernelList Size = %d \n\n\n\n", num);
        //m_kernel_list = o;
        m_num_kernels = num;
        return o;
    }
    return NULL;
}

void WebCLProgram::buildProgram(int options, int pfn_notify, int user_data, ExceptionCode& ec)
{
	cl_int err = 0;
	if (m_cl_program == NULL) {
		printf("Error: Invalid program object\n");
		ec = WebCLException::INVALID_PROGRAM;
		return;
	}

	// TODO(siba samal) - needs to be addressed later
	err = clBuildProgram(m_cl_program, 0, NULL, NULL, NULL, NULL);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				ec = WebCLException::INVALID_PROGRAM;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				ec = WebCLException::INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				ec = WebCLException::INVALID_DEVICE;
				break;
			case CL_INVALID_BINARY:
				printf("Error: CL_INVALID_BINARY\n");
				ec = WebCLException::INVALID_BINARY;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				ec = WebCLException::INVALID_OPERATION;
				break;
			case CL_INVALID_BUILD_OPTIONS :
				printf("Error: CL_INVALID_BUILD_OPTIONS\n");
				ec = WebCLException::INVALID_BUILD_OPTIONS;
				break;
			case CL_COMPILER_NOT_AVAILABLE:
				printf("Error: CL_COMPILER_NOT_AVAILABLE\n");
				ec = WebCLException::COMPILER_NOT_AVAILABLE;
				break;
			case CL_BUILD_PROGRAM_FAILURE:
				printf("Error: CL_BUILD_PROGRAM_FAILURE\n");
				ec = WebCLException::BUILD_PROGRAM_FAILURE;
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
				printf("WebCL::buildProgram normal options=%d pfn_notify=%d user_data=%d\n", 
						options, pfn_notify, user_data);
				ec = WebCLException::FAILURE;
				break;

		}

	} else {
		return;
	}
	return;
}

void WebCLProgram::buildProgram(WebCLDevice* device_id,int options, 
		int pfn_notify, int user_data, ExceptionCode& ec)
{
	cl_int err = 0;
	cl_device_id cl_device = NULL;
	if (m_cl_program == NULL) {
		printf("Error: Invalid program object\n");
		ec = WebCLException::INVALID_PROGRAM;
		return;
	}
	cl_device = device_id->getCLDevice();
	if (cl_device == NULL) {
		printf("Error: devices null\n");
		ec = WebCLException::INVALID_DEVICE;
		return;
	}

	// TODO(siba samal) - NULL parameters needs to be addressed later
	err = clBuildProgram(m_cl_program, 1, (const cl_device_id*)&cl_device, NULL, NULL, NULL);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				ec = WebCLException::INVALID_PROGRAM;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				ec = WebCLException::INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				ec = WebCLException::INVALID_DEVICE;
				break;
			case CL_INVALID_BINARY:
				printf("Error: CL_INVALID_BINARY\n");
				ec = WebCLException::INVALID_BINARY;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				ec = WebCLException::INVALID_OPERATION;
				break;
			case CL_INVALID_BUILD_OPTIONS :
				printf("Error: CL_INVALID_BUILD_OPTIONS\n");
				ec = WebCLException::INVALID_BUILD_OPTIONS;
				break;
			case CL_COMPILER_NOT_AVAILABLE:
				printf("Error: CL_COMPILER_NOT_AVAILABLE\n");
				ec = WebCLException::COMPILER_NOT_AVAILABLE;
				break;
			case CL_BUILD_PROGRAM_FAILURE:
				printf("Error: CL_BUILD_PROGRAM_FAILURE\n");
				ec = WebCLException::BUILD_PROGRAM_FAILURE;
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
				printf("WebCL::buildProgram WebCLDevice options=%d pfn_notify=%d user_data=%d\n", 
						options, pfn_notify, user_data);
				ec = WebCLException::FAILURE;
				break;

		}

	} else {
		return;
	}
	return;
}

void WebCLProgram::buildProgram( WebCLDeviceList* cl_devices, int options, 
		int pfn_notify, int user_data, ExceptionCode& ec)
{
	cl_int err = 0;	
	cl_device_id* cl_device = NULL;

	if (m_cl_program == NULL) {
		printf("Error: Invalid program object\n");
		ec = WebCLException::INVALID_PROGRAM;
		return ;
	}
	if (cl_devices != NULL) {
		cl_device = cl_devices->getCLDevices();
		if (cl_device == NULL) {
			ec = WebCLException::INVALID_DEVICE;
			printf("Error: devices null\n");
			return;
		}
	} else {
		ec = WebCLException::INVALID_DEVICE;
		printf("Error: webcl_devices null\n");
		return;
	}

	// TODO(siba samal) - NULL parameters needs to be addressed later
	err = clBuildProgram(m_cl_program, cl_devices->length(), cl_device, NULL, NULL, NULL);

	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				ec = WebCLException::INVALID_PROGRAM;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				ec = WebCLException::INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				ec = WebCLException::INVALID_DEVICE;
				break;
			case CL_INVALID_BINARY:
				printf("Error: CL_INVALID_BINARY\n");
				ec = WebCLException::INVALID_BINARY;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				ec = WebCLException::INVALID_OPERATION;
				break;
			case CL_INVALID_BUILD_OPTIONS :
				printf("Error: CL_INVALID_BUILD_OPTIONS\n");
				ec = WebCLException::INVALID_BUILD_OPTIONS;
				break;
			case CL_COMPILER_NOT_AVAILABLE:
				printf("Error: CL_COMPILER_NOT_AVAILABLE\n");
				ec = WebCLException::COMPILER_NOT_AVAILABLE;
				break;
			case CL_BUILD_PROGRAM_FAILURE:
				printf("Error: CL_BUILD_PROGRAM_FAILURE\n");
				ec = WebCLException::BUILD_PROGRAM_FAILURE;
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
				printf("WebCL::buildProgram WebCLDeviceList  options=%d pfn_notify=%d user_data=%d\n", 
						options, pfn_notify, user_data);
				ec = WebCLException::FAILURE;
				break;

		}

	} else {
		return;
	}
	return;
}

void WebCLProgram::finishCallBack(cl_program program,void *user_data)
{

	printf(" inside finishCallBack() call back \n ");

	if(program == NULL)
	{
		printf(" program is NULL \n ");
	}

	if(user_data == NULL)
	{
		printf(" user_data is NULL \n ");
	}

	WebCLProgram* self = static_cast<WebCLProgram*>(WebCLProgram::this_pointer);
	self->m_finishCallback.get()->handleEvent(17);

	printf(" Just Finished finishCallBack() call back ");


}

WebCLProgram* WebCLProgram::this_pointer = NULL;

void WebCLProgram::build( WebCLDeviceList* cl_devices, const String& options, PassRefPtr<WebCLFinishCallback> finishCallBack, int user_data, ExceptionCode& ec)
{
	cl_int err = 0;
	cl_device_id* cl_device = NULL;

	WebCLProgram::this_pointer =dynamic_cast<WebCLProgram*>(this);

	m_finishCallback = finishCallBack;

	if (m_cl_program == NULL) {
		printf("Error: Invalid program object\n");
		ec = WebCLException::INVALID_PROGRAM;
		return ;
	}
	if (cl_devices != NULL) {
		cl_device = cl_devices->getCLDevices();
		if (cl_device == NULL) {
			ec = WebCLException::INVALID_DEVICE;
			printf("Error: devices null\n");
			return;
		}
	} else {
		ec = WebCLException::INVALID_DEVICE;
		printf("Error: webcl_devices null\n");
		return;
	}

	const char* options_str = strdup(options.utf8().data());


	// TODO(siba samal) - NULL parameters needs to be addressed later

	if(m_finishCallback == NULL)
	{
		err = clBuildProgram(m_cl_program, cl_devices->length(), cl_device ,options_str, NULL, &user_data);
	}
	else
	{
		err = clBuildProgram(m_cl_program, cl_devices->length(), cl_device ,options_str, &(WebCLProgram::finishCallBack), &user_data);
	}



	if (err != CL_SUCCESS) {
		switch (err) {
			case CL_INVALID_PROGRAM:
				printf("Error: CL_INVALID_PROGRAM\n");
				ec = WebCLException::INVALID_PROGRAM;
				break;
			case CL_INVALID_VALUE:
				printf("Error: CL_INVALID_VALUE\n");
				ec = WebCLException::INVALID_VALUE;
				break;
			case CL_INVALID_DEVICE:
				printf("Error: CL_INVALID_DEVICE\n");
				ec = WebCLException::INVALID_DEVICE;
				break;
			case CL_INVALID_BINARY:
				printf("Error: CL_INVALID_BINARY\n");
				ec = WebCLException::INVALID_BINARY;
				break;
			case CL_INVALID_OPERATION:
				printf("Error: CL_INVALID_OPERATION\n");
				ec = WebCLException::INVALID_OPERATION;
				break;
			case CL_INVALID_BUILD_OPTIONS :
				printf("Error: CL_INVALID_BUILD_OPTIONS\n");
				ec = WebCLException::INVALID_BUILD_OPTIONS;
				break;
			case CL_COMPILER_NOT_AVAILABLE:
				printf("Error: CL_COMPILER_NOT_AVAILABLE\n");
				ec = WebCLException::COMPILER_NOT_AVAILABLE;
				break;
			case CL_BUILD_PROGRAM_FAILURE:
				printf("Error: CL_BUILD_PROGRAM_FAILURE\n");
				ec = WebCLException::BUILD_PROGRAM_FAILURE;
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
				//printf("WebCL::buildProgram WebCLDeviceList  options=%d pfn_notify=%d user_data=%d\n",
				//		options, pfn_notify, user_data);
				ec = WebCLException::FAILURE;
				break;

		}

	} else {
		return;
	}
	return;
}
	
void WebCLProgram::setDevice(RefPtr<WebCLDevice> m_device_id_)
{
	m_device_id = m_device_id_;
}

cl_program WebCLProgram::getCLProgram()
{
	return m_cl_program;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
