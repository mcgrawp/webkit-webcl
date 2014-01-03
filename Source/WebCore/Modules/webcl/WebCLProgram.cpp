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

#include "ComputeProgram.h"
#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLGetInfo.h"
#include "WebCLImageDescriptor.h"
#include "WebCLInputChecker.h"
#include "WebCLKernel.h"
#include <wtf/text/StringBuilder.h>

namespace WebCore {

WebCLProgram::~WebCLProgram()
{
    releasePlatformObject();
}

PassRefPtr<WebCLProgram> WebCLProgram::create(WebCLContext* context, const String& programSource, ExceptionCode& ec)
{
    CCerror error = 0;

    ComputeProgram* computeProgram = context->computeContext()->createProgram(programSource, ec);
    if (error != ComputeContext::SUCCESS) {
        delete computeProgram;
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    return adoptRef(new WebCLProgram(context, computeProgram, programSource));
}

WebCLProgram::WebCLProgram(WebCLContext*context, ComputeProgram* program, const String& programSource)
    : WebCLObjectImpl(program)
    , m_context(context)
    , m_weakPtrFactory(this)
    , m_programSource(programSource)
{
    context->trackReleaseableWebCLObject(createWeakPtr());
}

WebCLGetInfo WebCLProgram::getInfo(CCenum infoType, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }

    switch (infoType) {
    case ComputeContext::PROGRAM_NUM_DEVICES:
        return m_context->getInfo(ComputeContext::CONTEXT_NUM_DEVICES, ec);
    case ComputeContext::PROGRAM_SOURCE:
        return WebCLGetInfo(m_programSource);
    case ComputeContext::PROGRAM_CONTEXT:
        return WebCLGetInfo(m_context.get());
    case ComputeContext::PROGRAM_DEVICES:
        return m_context->getInfo(ComputeContext::CONTEXT_DEVICES, ec);
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT_NOT_REACHED();
}

WebCLGetInfo WebCLProgram::getBuildInfo(WebCLDevice* device, CCenum infoType, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }

    if (!WebCLInputChecker::validateWebCLObject(device)) {
        // FIXME: Wrong exception?
        ec = WebCLException::INVALID_PROGRAM;
        return WebCLGetInfo();
    }
    CCDeviceID ccDeviceID = device->platformObject();

    CCerror error = 0;
    switch (infoType) {
    case ComputeContext::PROGRAM_BUILD_OPTIONS:
    case ComputeContext::PROGRAM_BUILD_LOG: {
        Vector<char> buffer;
        error = platformObject()->getBuildInfo(ccDeviceID, infoType, &buffer);
        if (error == ComputeContext::SUCCESS)
            return WebCLGetInfo(String(buffer.data()));
        break;
    }
    case ComputeContext::PROGRAM_BUILD_STATUS: {
        CCBuildStatus buildStatus;
        error = platformObject()->getBuildInfo(ccDeviceID, infoType, &buildStatus);
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
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_PROGRAM;
        return 0;
    }

    return WebCLKernel::create(m_context.get(), this, kernelName, ec);
}

Vector<RefPtr<WebCLKernel> > WebCLProgram::createKernelsInProgram(ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_PROGRAM;
        return Vector<RefPtr<WebCLKernel> >();
    }

    return WebCLKernel::createKernelsInProgram(m_context.get(), this, ec);
}

Vector<WeakPtr<WebCLProgram> >* WebCLProgram::s_thisPointers =  nullptr;

/*  Static function to be sent as callback to OpenCL clBuildProgram.
    Must be static and must map a (CCProgram,userData) to corresponding WebCLProgram.
    Here we use the userData as a index to a static Vector. On each call to build with callback,
    append a copy of this pointer to this Vector and send index as userData. On return this static
    member will be used to retrive the WebCLProgram.
    */

void WebCLProgram::callbackProxy(CCProgram program, void* userData)
{
    UNUSED_PARAM(program);
    size_t index = *((int*)userData);
    ASSERT(index < s_thisPointers->size());

    WeakPtr<WebCLProgram> webCLProgram = WebCLProgram::s_thisPointers->at(index);
    if (webCLProgram.get())
        webCLProgram.get()->callEvent();
}

void WebCLProgram::build(const Vector<RefPtr<WebCLDevice> >& devices, const String& buildOptions, PassRefPtr<WebCLCallback> callback, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_PROGRAM;
        return;
    }

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
            // Every build option must start with a hyphen.
            if (!webCLBuildOptionsVector[i].startsWith("-")) {
                ec = WebCLException::INVALID_BUILD_OPTIONS;
                return;
            }
            if (webCLSupportedBuildOptions.contains(webCLBuildOptionsVector[i]))
                continue;
            /* If the token begins with "-D" it can be one of "-D NAME" or "-D name=definition".
               Currently OpenCL specification does not state any restriction adding spaces in between.
               So on encounter of a token starting with "-D" we skip validation till we reach next build option.
               Pushing the validation of "-D" options to underlying OpenCL. */
            if (webCLBuildOptionsVector[i].startsWith(buildOptionDashD)) {
                size_t j;
                for (j = ++i; j < webCLBuildOptionsVector.size() && !webCLBuildOptionsVector[j].startsWith("-"); ++j) { }
                i = --j;
                continue;
            }
            ec = WebCLException::INVALID_BUILD_OPTIONS;
            return;
        }
    }

    Vector<CCDeviceID> ccDevices;
    for (size_t i = 0; i < devices.size(); i++)
        ccDevices.append(devices[i]->platformObject());
    pfnNotify callbackProxyPtr = 0;
    if (callback) {
        m_callback = callback;
        if (!s_thisPointers)
            s_thisPointers = new Vector<WeakPtr<WebCLProgram> >();
        s_thisPointers->append(m_weakPtrFactory.createWeakPtr());
        callbackProxyPtr = &callbackProxy;
    }

    CCerror err = platformObject()->buildProgram(ccDevices, buildOptions, callbackProxyPtr, s_thisPointers ? s_thisPointers->size() - 1 : 0);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

// FIXME: Guard this change under !VALIDATOR_INTEGRATION.
static void removeComments(const String& inSource, String& outSource)
{
    enum Mode { DEFAULT, BLOCK_COMMENT, LINE_COMMENT };
    Mode currentMode = DEFAULT;

    ASSERT(!inSource.isNull());
    ASSERT(!inSource.isEmpty());

    outSource = inSource;

    for (unsigned i = 0; i < outSource.length(); ++i) {
        if (currentMode == BLOCK_COMMENT) {
            if (outSource[i] == '*' && outSource[i + 1] == '/') {
                outSource.replace(i++, 2, "  ");
                currentMode = DEFAULT;
                continue;
            }
            outSource.replace(i, 1, " ");
            continue;
        }

        if (currentMode == LINE_COMMENT) {
            if (outSource[i] == '\n' || outSource[i] == '\r') {
                currentMode = DEFAULT;
                continue;
            }
            outSource.replace(i, 1, " ");
            continue;
        }

        if(outSource[i] == '/') {
            if(outSource[i + 1] == '*') {
                outSource.replace(i++, 2, "  ");
                currentMode = BLOCK_COMMENT;
                continue;
            }

            if(outSource[i + 1] == '/'){
                outSource.replace(i++, 2, "  ");
                currentMode = LINE_COMMENT;
                continue;
            }
        }
    }
}

const String& WebCLProgram::sourceWithCommentsStripped()
{
    if (m_programSourceWithCommentsStripped.isNull())
        removeComments(m_programSource, m_programSourceWithCommentsStripped);

    return m_programSourceWithCommentsStripped;
}

void WebCLProgram::releasePlatformObjectImpl()
{
    delete platformObject();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
