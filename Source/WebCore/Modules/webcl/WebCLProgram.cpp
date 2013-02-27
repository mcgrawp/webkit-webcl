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

namespace WebCore {

WebCLProgram::~WebCLProgram()
{
    ASSERT(m_clProgram);
    CCerror computeContextErrorCode = m_context->computeContext()->releaseProgram(m_clProgram);

    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
    m_clProgram = 0;
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

WebCLGetInfo WebCLProgram::getInfo(int infoType, ExceptionCode& ec)
{
    if (!m_clProgram) {
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }

    CCerror error = 0;
    switch (infoType) {
    case ComputeContext::PROGRAM_NUM_DEVICES: {
        CCuint uintUnits = 0;
        error = ComputeContext::getProgramInfo(m_clProgram, infoType, sizeof(CCuint), &uintUnits, 0);
        if (error == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uintUnits));
        break;
    }
    case ComputeContext::PROGRAM_SOURCE: {
        char programString[4096] = {""};
        error = ComputeContext::getProgramInfo(m_clProgram, infoType, sizeof(programString), &programString, 0);
        if (error == ComputeContext::SUCCESS)
            return WebCLGetInfo(String(programString));
        break;
    }
    case ComputeContext::PROGRAM_CONTEXT: {
        CCContext ccContextID = 0;
        error = ComputeContext::getProgramInfo(m_clProgram, infoType, sizeof(CCContext), &ccContextID, 0);
        if (error == ComputeContext::SUCCESS) {
            RefPtr<WebCLContext> contextObj = 0;
            // FIXME Need a create API taking cl_context in WebCLContext interface.
            // contextObj = WebCLContext::create(m_context->webclObject(), ccContextID, 1, 0, error);
            return WebCLGetInfo(contextObj.release());
        }
        break;
    }
    case ComputeContext::PROGRAM_DEVICES: {
        size_t devicesSize = 0;
        error = ComputeContext::getProgramInfo(m_clProgram, ComputeContext::PROGRAM_DEVICES, 0, 0, &devicesSize);
        if (error == ComputeContext::SUCCESS) {
            Vector<CCDeviceID> ccDevices(devicesSize);
            ComputeContext::getProgramInfo(m_clProgram, infoType, devicesSize, ccDevices.data(), 0);
            Vector<RefPtr<WebCLDevice> > devices;
            toWebCLDeviceArray(ccDevices, devices);
            return WebCLGetInfo(devices);
        }
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
    return WebCLGetInfo();
}


WebCLGetInfo WebCLProgram::getBuildInfo(WebCLDevice* device, int infoType, ExceptionCode& ec)
{
    if (!m_clProgram) {
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }

    CCDeviceID ccDeviceID = 0;
    if (device)
        ccDeviceID = device->getCLDevice();
    if (!ccDeviceID) {
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }

    CCerror error = 0;
    switch (infoType) {
    case ComputeContext::PROGRAM_BUILD_OPTIONS:
    case ComputeContext::PROGRAM_BUILD_LOG: {
        char buffer[WebCL::CHAR_BUFFER_SIZE];
        error = ComputeContext::getBuildInfo(m_clProgram, ccDeviceID, infoType, sizeof(char) * WebCL::CHAR_BUFFER_SIZE, buffer);
        if (error == CL_SUCCESS)
            return WebCLGetInfo(String(buffer));
        break;
    }
    case ComputeContext::PROGRAM_BUILD_STATUS: {
        CCBuildStatus buildStatus;
        error = ComputeContext::getBuildInfo(m_clProgram, ccDeviceID, infoType, sizeof(CCBuildStatus), &buildStatus);
        if (error == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(buildStatus));
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(error != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
    return WebCLGetInfo();
}

PassRefPtr<WebCLKernel> WebCLProgram::createKernel(const String& kernelName, ExceptionCode& ec)
{
    if (!m_clProgram) {
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

Vector<RefPtr<WebCLKernel> > WebCLProgram::createKernelsInProgram(ExceptionCode& ec)
{
    Vector<RefPtr<WebCLKernel> > kernels;
    if (!m_clProgram) {
        ec = WebCLException::INVALID_PROGRAM;
        return kernels;
    }

    CCerror error;
    CCuint numberOfKernels = 0;
    CCKernel* computeContextKernels = m_context->computeContext()->createKernelsInProgram(m_clProgram, numberOfKernels, error);
    // computeContextKernels can be 0 even if there was not ComputeContext error, e.g. program has 0 kernels.
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return kernels;
    }

    for (size_t i = 0 ; i < numberOfKernels; i++)
        kernels.append(WebCLKernel::create(m_context, computeContextKernels[i]));

    return kernels;
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
    if (self)
        self->m_finishCallback.get()->handleEvent(17);
    else
        printf(" ERROR:: static_cast to WebCLProgram failed\n");
    printf(" Just Finished finishCallback() call back");
}

WebCLProgram* WebCLProgram::thisPointer = 0;

void WebCLProgram::build(const Vector<WebCLDevice*>& devices, const String& options,
    PassRefPtr<WebCLFinishCallback> finishCallback, int userData, ExceptionCode& ec)
{
    WebCLProgram::thisPointer = static_cast<WebCLProgram*>(this);

    m_finishCallback = finishCallback;
    if (!m_clProgram) {
        ec = WebCLException::INVALID_PROGRAM;
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
            ec = WebCLException::INVALID_BUILD_OPTIONS;
            return;
        }
    }

    Vector<CCDeviceID> ccDevices;
    for (size_t i = 0; i < devices.size(); i++)
        ccDevices.append(devices[i]->getCLDevice());

    CCerror err;
    if (!m_finishCallback)
        err = clBuildProgram(m_clProgram, ccDevices.size(), ccDevices.data(), optionsStr, 0, &userData);
    else
        err = clBuildProgram(m_clProgram, ccDevices.size(), ccDevices.data(), optionsStr,
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
    ASSERT(m_clProgram);
    CCerror computeContextErrorCode = m_context->computeContext()->releaseProgram(m_clProgram);
    if (computeContextErrorCode == CL_SUCCESS)
        m_clProgram = 0;
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(computeContextErrorCode);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
