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
        Vector<char, WebCL::CHAR_BUFFER_SIZE> buffer;
        error = ComputeContext::getBuildInfo(m_clProgram, ccDeviceID, infoType, sizeof(buffer), buffer.data());
        if (error == CL_SUCCESS)
            return WebCLGetInfo(String(buffer.data()));
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

    return WebCLKernel::create(m_context, this, kernelName, ec);
}

Vector<RefPtr<WebCLKernel> > WebCLProgram::createKernelsInProgram(ExceptionCode& ec)
{

    if (!m_clProgram) {
        ec = WebCLException::INVALID_PROGRAM;
        return Vector<RefPtr<WebCLKernel> >();
    }

    return WebCLKernel::createKernelsInProgram(m_context, this, ec);
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

    // FIXME: This should not be here.
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

    pfnNotify callback = m_finishCallback ? &WebCLProgram::finishCallback : 0;
    CCerror err =  m_context->computeContext()->buildProgram(m_clProgram, ccDevices, buildOptions, callback, &userData);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

CCProgram WebCLProgram::getCLProgram()
{
    return m_clProgram;
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
