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

#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLFinishCallback.h"
#include "WebCLGetInfo.h"
#include "WebCLImageDescriptor.h"
#include "WebCLInputChecker.h"
#include "WebCLKernel.h"

namespace WebCore {

WebCLProgram::~WebCLProgram()
{
    releasePlatformObject();
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

WebCLProgram::WebCLProgram(WebCLContext* context, CCProgram program, const String& kernelSource)
    : WebCLObject(program)
    , m_context(context)
    , m_kernelSource(kernelSource)
{

}

WebCLGetInfo WebCLProgram::getInfo(int infoType, ExceptionCode& ec)
{
    if (!platformObject()) {
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
    if (!platformObject()) {
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }

    if (!WebCLInputChecker::validateWebCLObject(device)) {
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }
    CCDeviceID ccDeviceID = device->platformObject();

    CCerror error = 0;
    switch (infoType) {
    case ComputeContext::PROGRAM_BUILD_OPTIONS:
    case ComputeContext::PROGRAM_BUILD_LOG: {
        Vector<char> buffer;
        error = ComputeContext::getBuildInfo(platformObject(), ccDeviceID, infoType, &buffer);
        if (error == ComputeContext::SUCCESS)
            return WebCLGetInfo(String(buffer.data()));
        break;
    }
    case ComputeContext::PROGRAM_BUILD_STATUS: {
        CCBuildStatus buildStatus;
        error = ComputeContext::getBuildInfo(platformObject(), ccDeviceID, infoType, &buildStatus);
        if (error == ComputeContext::SUCCESS)
            return WebCLGetInfo(buildStatus);
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(error != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
    return WebCLGetInfo();
}

PassRefPtr<WebCLKernel> WebCLProgram::createKernel(const String& kernelName, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_PROGRAM;
        return 0;
    }

    return WebCLKernel::create(m_context, this, kernelName, ec);
}

Vector<RefPtr<WebCLKernel> > WebCLProgram::createKernelsInProgram(ExceptionCode& ec)
{

    if (!platformObject()) {
        ec = WebCLException::INVALID_PROGRAM;
        return Vector<RefPtr<WebCLKernel> >();
    }

    return WebCLKernel::createKernelsInProgram(m_context, this, ec);
}

void WebCLProgram::finishCallback(CCProgram program, void* userData)
{
	UNUSED_PARAM(program);
	UNUSED_PARAM(userData);

    WebCLProgram* self = static_cast<WebCLProgram*>(WebCLProgram::thisPointer);
    if (self)
        self->m_finishCallback.get()->handleEvent(17);
    else
        printf(" ERROR:: static_cast to WebCLProgram failed\n");
    printf(" Just Finished finishCallback() call back");
}

WebCLProgram* WebCLProgram::thisPointer = 0;

void WebCLProgram::build(const Vector<RefPtr<WebCLDevice> >& devices, const String& buildOptions, PassRefPtr<WebCLFinishCallback> finishCallback, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_PROGRAM;
        return;
    }
    WebCLProgram::thisPointer = static_cast<WebCLProgram*>(this);

    m_finishCallback = finishCallback;

    if (buildOptions.length() > 0) {
        DEFINE_STATIC_LOCAL(AtomicString, buildOptionDashD, ("-D", AtomicString::ConstructFromLiteral));
        DEFINE_STATIC_LOCAL(HashSet<AtomicString>, webCLSupportedBuildOptions, ());
        if (webCLSupportedBuildOptions.isEmpty()) {
            webCLSupportedBuildOptions.add(AtomicString("-cl-opt-disable", AtomicString::ConstructFromLiteral));
            webCLSupportedBuildOptions.add(AtomicString("-cl-single-precision-constant", AtomicString::ConstructFromLiteral));
            webCLSupportedBuildOptions.add(AtomicString("-cl-denorms-are-zero", AtomicString::ConstructFromLiteral));
            webCLSupportedBuildOptions.add(AtomicString("-cl-mad-enable", AtomicString::ConstructFromLiteral));
            webCLSupportedBuildOptions.add(AtomicString("-cl-no-signed-zeros", AtomicString::ConstructFromLiteral));
            webCLSupportedBuildOptions.add(AtomicString("-cl-unsafe-math-optimizations", AtomicString::ConstructFromLiteral));
            webCLSupportedBuildOptions.add(AtomicString("-cl-finite-math-only", AtomicString::ConstructFromLiteral));
            webCLSupportedBuildOptions.add(AtomicString("-cl-fast-relaxed-math", AtomicString::ConstructFromLiteral));
            webCLSupportedBuildOptions.add(AtomicString("-W", AtomicString::ConstructFromLiteral));
            webCLSupportedBuildOptions.add(AtomicString("-Werror", AtomicString::ConstructFromLiteral));
        }

        Vector<String> webCLBuildOptionsVector;
        buildOptions.split(" ", false /* allowEmptyEntries */, webCLBuildOptionsVector);
        for (size_t i = 0; i < webCLBuildOptionsVector.size(); i++) {
            if (webCLSupportedBuildOptions.contains(webCLBuildOptionsVector[i]))
                continue;
            if (buildOptionDashD == webCLBuildOptionsVector[i]) {
                i++;
                if (i >= webCLBuildOptionsVector.size()) {
                    ec = WebCLException::INVALID_BUILD_OPTIONS;
                    return;
                }
                continue;
            }
            ec = WebCLException::INVALID_BUILD_OPTIONS;
            return;
        }
    }

    // As per the specification, implicitly set -cl-std=CL1.1.
    StringBuilder webclBuildOptions;
    webclBuildOptions.append(buildOptions);
    webclBuildOptions.append(" -cl-std=CL1.1");

    Vector<CCDeviceID> ccDevices;
    for (size_t i = 0; i < devices.size(); i++)
        ccDevices.append(devices[i]->platformObject());

    pfnNotify callback = m_finishCallback ? &WebCLProgram::finishCallback : 0;
    CCerror err = m_context->computeContext()->buildProgram(platformObject(), ccDevices, webclBuildOptions.toString(), callback);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLProgram::release()
{
    releasePlatformObject();
}

void WebCLProgram::releasePlatformObjectImpl()
{
    CCerror computeContextErrorCode = m_context->computeContext()->releaseProgram(platformObject());
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
}


} // namespace WebCore

#endif // ENABLE(WEBCL)
