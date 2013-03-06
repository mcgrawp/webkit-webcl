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

#include "WebCLSampler.h"

#include "WebCL.h"
#include "WebCLException.h"
#include <wtf/Assertions.h>

namespace WebCore {

WebCLSampler::~WebCLSampler()
{
    ASSERT(m_ccSampler);
    CCerror computeContextErrorCode = m_context->computeContext()->releaseSampler(m_ccSampler);
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
    m_ccSampler = 0;
}

PassRefPtr<WebCLSampler> WebCLSampler::create(WebCLContext* context, CCSampler sampler)
{
    return adoptRef(new WebCLSampler(context, sampler));
}

WebCLSampler::WebCLSampler(WebCLContext* context, CCSampler sampler)
    : m_context(context)
    , m_ccSampler(sampler)
{
    // FIXME: For now, use it in order to silent a clang compiler warning.
    // Remove this when SAMPLE_CONTEXT is supported.
    UNUSED_PARAM(m_context);
}

WebCLGetInfo WebCLSampler::getInfo(int infoType, ExceptionCode& ec)
{
    if (!m_ccSampler) {
        ec = WebCLException::INVALID_SAMPLER;
        return WebCLGetInfo();
    }

    CCerror error = 0;
    switch (infoType) {
    case ComputeContext::SAMPLER_NORMALIZED_COORDS: {
        CCbool booleanValue = false;
        error = ComputeContext::getSamplerInfo(m_ccSampler, infoType, sizeof(CCbool), &booleanValue);
        if (error == CL_SUCCESS)
            return WebCLGetInfo(static_cast<bool>(booleanValue));
        break;
    }
    /* FIXME: Implementation needed.
    case ComputeContext::SAMPLER_CONTEXT: {
        CCContext ccContext;
        error = ComputeContext::getSamplerInfo(m_ccSampler, infoType, sizeof(CCContext), &ccContext);
        if (error == CL_SUCCESS) {
            RefPtr<WebCLContext> contextObj = WebCLContext::create(m_context, ccContext);
            return WebCLGetInfo(contextObj.release());
        }
        break;
    }
    */
    case ComputeContext::SAMPLER_ADDRESSING_MODE:
    case ComputeContext::SAMPLER_FILTER_MODE: {
        CCuint samplerInfo = 0;
        error = ComputeContext::getSamplerInfo(m_ccSampler, infoType, sizeof(CCuint), &samplerInfo);
        if (error == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(samplerInfo));
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

CCSampler WebCLSampler::getCLSampler()
{
    return m_ccSampler;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
