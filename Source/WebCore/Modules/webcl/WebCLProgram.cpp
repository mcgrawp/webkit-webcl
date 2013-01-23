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
#include "WebCLProgram.h"

#include "WebCL.h"
#include "WebCLGetInfo.h"
#include "WebCLKernelList.h"

namespace WebCore {

WebCLProgram::~WebCLProgram()
{
    /* FIXME Causing crashes. Need to investigate.
    cl_int err = 0;
    ASSERT(m_clProgram);

    err = clReleaseProgram(m_clProgram);
    if (err != CL_SUCCESS)
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
            if ((m_program_list[i].get())->getCLProgram() == m_clProgram) {
                m_program_list.remove(i);
                m_num_programs = m_program_list.size();
                break;
            }
        }
    }*/
}

PassRefPtr<WebCLProgram> WebCLProgram::create(WebCLContext* context, cl_program program)
{
    return adoptRef(new WebCLProgram(context, program));
}

WebCLProgram::WebCLProgram(WebCLContext* context, cl_program program)
    : m_context(context)
    , m_clProgram(program)
{
}

WebCLGetInfo WebCLProgram::getInfo(int paramName, ExceptionCode& ec)
{
    if (!m_clProgram) {
        ec = WebCLException::INVALID_PROGRAM;
        printf("Error: Invalid program object\n");
        return WebCLGetInfo();
    }

    cl_int err = 0;
    cl_uint uintUnits = 0;
    size_t sizetUnits = 0;
    char programString[4096] = {""};
    cl_context clContextID = 0;
    RefPtr<WebCLContext> contextObj  = 0;
    RefPtr<WebCLDeviceList> deviceList =  0;
    size_t szParmDataBytes = 0;

    switch (paramName) {
    case WebCL::PROGRAM_REFERENCE_COUNT:
    case WebCL::PROGRAM_NUM_DEVICES:
        err = ComputeContext::getProgramInfo(m_clProgram, paramName, sizeof(cl_uint), &uintUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uintUnits));
        break;
    case WebCL::PROGRAM_BINARY_SIZES:
        err = ComputeContext::getProgramInfo(m_clProgram, CL_PROGRAM_BINARY_SIZES, sizeof(size_t), &sizetUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(sizetUnits));
        break;
    case WebCL::PROGRAM_SOURCE:
    case WebCL::PROGRAM_BINARIES:
        err = ComputeContext::getProgramInfo(m_clProgram, paramName, sizeof(programString), &programString, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(String(programString));
        break;
    case WebCL::PROGRAM_CONTEXT:
        {
            err = ComputeContext::getProgramInfo(m_clProgram, CL_PROGRAM_CONTEXT, sizeof(cl_context), &clContextID, 0);
            // FIXME Need a create API taking cl_context in WebCLContext interface.
            // contextObj = WebCLContext::create(m_context->webclObject(), clContextID, 1, 0, err );
            if (!contextObj) {
                printf("Error : CL program context not NULL\n");
                return WebCLGetInfo();
            }
            if (err == CL_SUCCESS)
                return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
        }
        break;
    case WebCL::PROGRAM_DEVICES:
        {
            err = ComputeContext::getProgramInfo(m_clProgram, CL_PROGRAM_DEVICES, 0, 0, &szParmDataBytes);
            if (err == CL_SUCCESS) {
                Vector<CCDeviceID> devices(szParmDataBytes);
                ComputeContext::getProgramInfo(m_clProgram, CL_PROGRAM_DEVICES, szParmDataBytes, devices.data(), 0);
                deviceList = WebCLDeviceList::create(devices);
                return WebCLGetInfo(PassRefPtr<WebCLDeviceList>(deviceList));
            }
        }
        break;
    default:
        // FIXME: Is this really the best exception?
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }

    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}


WebCLGetInfo WebCLProgram::getBuildInfo(WebCLDevice* device, int paramName, ExceptionCode& ec)
{
    cl_device_id deviceID = 0;
    cl_uint err = 0;
    char buffer[8192];
    size_t len = 0;

    if (!m_clProgram) {
        printf("Error: Invalid program object\n");
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }
    if (device) {
        deviceID = device->getCLDevice();
        if (!deviceID) {
            ec = WebCLException::INVALID_DEVICE;
            printf("Error: device id null\n");
            return WebCLGetInfo();
        }
    }

    switch (paramName) {
    case WebCL::PROGRAM_BUILD_LOG:
        err = clGetProgramBuildInfo(m_clProgram, deviceID, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(String(buffer));
        break;
    case WebCL::PROGRAM_BUILD_OPTIONS:
        err = clGetProgramBuildInfo(m_clProgram, deviceID, CL_PROGRAM_BUILD_OPTIONS, sizeof(buffer), &buffer, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(String(buffer));
        break;
    case WebCL::PROGRAM_BUILD_STATUS:
        cl_build_status buildStatus;
        err = clGetProgramBuildInfo(m_clProgram, deviceID, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status),
            &buildStatus, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(buildStatus));
        break;
    default:
        ec = WebCLException::INVALID_PROGRAM;
        printf("Error: UNSUPPORTED Program Build Info   Type = %d ", paramName);
        return WebCLGetInfo();
    }

    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

PassRefPtr<WebCLKernel> WebCLProgram::createKernel(const String& kernelName, ExceptionCode& ec)
{
    if (!m_clProgram) {
        printf("Error: Invalid program object\n");
        ec = WebCLException::INVALID_PROGRAM;
        return 0;
    }

    CCerror error;
    CCKernel computeContextKernel = m_context->computeContext()->createKernel(m_clProgram, kernelName, error);
    if (!computeContextKernel) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLKernel> webclKernel = WebCLKernel::create(m_context, computeContextKernel);
    return webclKernel;
}

PassRefPtr<WebCLKernelList> WebCLProgram::createKernelsInProgram(ExceptionCode& ec)
{
    if (!m_clProgram) {
        printf("Error: Invalid program object\n");
        ec = WebCLException::INVALID_PROGRAM;
        return 0;
    }

    CCerror error = 0;
    CCuint numberOfKernels = 0;
    CCKernel* computeContextKernels = m_context->computeContext()->createKernelsInProgram(m_clProgram, numberOfKernels, error);
    // computeContextKernels can be 0 even if there was not ComputeContext error, e.g. program has 0 kernels.
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    RefPtr<WebCLKernelList> kernalListObject = WebCLKernelList::create(m_context, computeContextKernels, numberOfKernels);
    return kernalListObject;
}

void WebCLProgram::finishCallback(cl_program program, void* userData)
{
    // FIXME :: Test and clean.
    printf(" inside finishCallback() call back \n ");
    if (!program)
        printf(" program is NULL \n ");

    if (!userData)
        printf(" userData is NULL \n ");

    WebCLProgram* self = static_cast<WebCLProgram*>(WebCLProgram::thisPointer);
    self->m_finishCallback.get()->handleEvent(17);
    printf(" Just Finished finishCallback() call back");
}

WebCLProgram* WebCLProgram::thisPointer = 0;

void WebCLProgram::build(WebCLDeviceList* clDevices, const String& options,
    PassRefPtr<WebCLFinishCallback> finishCallback, int userData, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_device_id* clDevice = 0;

    m_finishCallback = finishCallback;
    if (!m_clProgram) {
        printf("Error: Invalid program object\n");
        ec = WebCLException::INVALID_PROGRAM;
        return;
    }
    if (clDevices)
        clDevice = clDevices->getCLDevices();
    if (!clDevice) {
        ec = WebCLException::INVALID_DEVICE;
        printf("Error: devices null\n");
        return;
    }

    char* optionsStr = 0;
    if (options.utf8().length() > 0) {
        optionsStr = strdup(options.utf8().data());
        if (!(!strcmp(optionsStr, "-cl-single-precision-constant")
        || !strcmp(optionsStr, "-cl-denorms-are-zero")
        || !strcmp(optionsStr, "-cl-opt-disable")
        || !strcmp(optionsStr, "-cl-mad-enable")
        || !strcmp(optionsStr, "-cl-no-signed-zeros")
        || !strcmp(optionsStr, "-cl-unsafe-math-optimizations")
        || !strcmp(optionsStr, "-cl-finite-math-only")
        || !strcmp(optionsStr, "-cl-fast-relaxed-math")
        || !strcmp(optionsStr, "-w")
        || !strcmp(optionsStr, "-Werror")
        || !strcmp(optionsStr, "-cl-std="))) {
            printf("Error: CL_INVALID_BUILD_OPTIONS\n");
            ec = WebCLException::INVALID_BUILD_OPTIONS;
            return;
        }
    }

    if (!m_finishCallback)
        err = clBuildProgram(m_clProgram, clDevices->length(), clDevice, optionsStr, 0, &userData);
    else
        err = clBuildProgram(m_clProgram, clDevices->length(), clDevice, optionsStr,
            &(WebCLProgram::finishCallback), &userData);

    // Free memory allocated by strdup.
    if (optionsStr)
        free((char *)optionsStr);
    optionsStr = 0;

    if (err != CL_SUCCESS)
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLProgram::releaseProgram(ExceptionCode& ec)
{
    cl_int err = 0;
    if (!m_clProgram) {
        printf("Error: Invalid program object\n");
        ec = WebCLException::INVALID_PROGRAM;
        return;
    }
    err = clReleaseProgram(m_clProgram);
    if (err != CL_SUCCESS) {
        switch (err) {
        case CL_INVALID_PROGRAM:
            printf("Error: CL_INVALID_PROGRAM\n");
            ec = WebCLException::INVALID_PROGRAM;
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
    } else
        printf(" SUCCESS:: clReleaseProgram was successfull\n");
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
