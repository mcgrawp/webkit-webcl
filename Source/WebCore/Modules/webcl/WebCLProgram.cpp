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

    PassRefPtr<WebCLProgram> WebCLProgram::create(WebCLContext* context, const String& kernelSource, ExceptionCode& ec)
{
    CCerror error = 0;
    CCProgram clProgram = context->computeContext()->createProgram(kernelSource, error);
    if (!clProgram) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    return adoptRef(new WebCLProgram(context, clProgram, kernelSource));
}

WebCLProgram::WebCLProgram(WebCLContext* context, cl_program program, const String& kernelSource)
    : m_context(context)
    , m_clProgram(program)
    , m_kernelSource(kernelSource)
{
}

WebCLGetInfo WebCLProgram::getInfo(int infoType, ExceptionCode& ec)
{
    if (!m_clProgram) {
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }

    switch (infoType) {
    case ComputeContext::PROGRAM_NUM_DEVICES:
        return m_context->getInfo(ComputeContext::CONTEXT_NUM_DEVICES, ec);
    case ComputeContext::PROGRAM_SOURCE:
        return WebCLGetInfo(m_kernelSource);
    case ComputeContext::PROGRAM_CONTEXT:
        return WebCLGetInfo(m_context);
    case ComputeContext::PROGRAM_DEVICES:
        return m_context->getInfo(ComputeContext::CONTEXT_DEVICES, ec);
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT_NOT_REACHED();
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

void WebCLProgram::build(const Vector<RefPtr<WebCLDevice> >& devices, const String& buildOptions,
    PassRefPtr<WebCLFinishCallback> finishCallback, int userData, ExceptionCode& ec)
{
    if (!m_clProgram) {
        ec = WebCLException::INVALID_PROGRAM;
        return;
    }
    WebCLProgram::thisPointer = static_cast<WebCLProgram*>(this);

    m_finishCallback = finishCallback;

    if (buildOptions.length() > 0) {
        if (!((buildOptions == "-cl-single-precision-constant")
        || (buildOptions == "-cl-denorms-are-zero")
        || (buildOptions == "-cl-opt-disable")
        || (buildOptions == "-cl-mad-enable")
        || (buildOptions == "-cl-no-signed-zeros")
        || (buildOptions == "-cl-unsafe-math-optimizations")
        || (buildOptions == "-cl-finite-math-only")
        || (buildOptions == "-cl-fast-relaxed-math")
        || (buildOptions == "-w")
        || (buildOptions == "-Werror")
        || (buildOptions == "-cl-std="))) {
            ec = WebCLException::INVALID_BUILD_OPTIONS;
            return;
        }
    }

    Vector<CCDeviceID> ccDevices;
    for (size_t i = 0; i < devices.size(); i++)
        ccDevices.append(devices[i]->getCLDevice());

    CCerror err;
    if (!m_finishCallback)
        err =  m_context->computeContext()->buildProgram(m_clProgram, ccDevices, buildOptions, 0, &userData);
    else
        err = m_context->computeContext()->buildProgram(m_clProgram, ccDevices, buildOptions, &(WebCLProgram::finishCallback),
            &userData);

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
