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

#include "ExceptionCode.h"
#include "WebCL.h"
#include "WebCLException.h"

namespace WebCore {

WebCLSampler::~WebCLSampler()
{
    CCerror err = 0;
    ASSERT(m_ccSampler);
    err = clReleaseSampler(m_ccSampler);
    if (err != CL_SUCCESS) {
        switch (err) {
        case CL_INVALID_SAMPLER:
            printf("Error: CL_INVALID_SAMPLER\n");
            break;
        case CL_OUT_OF_RESOURCES:
            printf("Error: CL_OUT_OF_RESOURCES\n");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            printf("Error: CL_OUT_OF_HOST_MEMORY\n");
            break;

        default:
            printf("Error: Invaild Error Type\n");
            break;
        }
    } else
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
}

WebCLGetInfo WebCLSampler::getInfo(int paramName, ExceptionCode& ec)
{
    CCerror err = 0;
    CCuint uintUnits = 0;
    CCbool boolUnits = false;
    if (!m_ccSampler) {
        ec = WebCLException::INVALID_SAMPLER;
        printf("Error: Invalid Sampler.\n");
        return WebCLGetInfo();
    }
    switch (paramName) {
    case WebCL::SAMPLER_NORMALIZED_COORDS:
        err = clGetSamplerInfo(m_ccSampler, CL_SAMPLER_NORMALIZED_COORDS, sizeof(CCbool), &boolUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<bool>(boolUnits));
        break;
    /*case WebCL::SAMPLER_CONTEXT:
        err = clGetSamplerInfo(m_ccSampler, CL_SAMPLER_CONTEXT, sizeof(cl_context), &cl_context_id, 0);
        contextObj = WebCLContext::create(m_context, cl_context_id);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(PassRefPtr<WebCLContext>(contextObj));
        break;*/
    case WebCL::SAMPLER_ADDRESSING_MODE:
        err = clGetSamplerInfo(m_ccSampler, CL_SAMPLER_ADDRESSING_MODE, sizeof(CCuint), &uintUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uintUnits));
        break;
    case WebCL::SAMPLER_FILTER_MODE:
        err = clGetSamplerInfo(m_ccSampler, CL_SAMPLER_FILTER_MODE, sizeof(CCuint), &uintUnits, 0);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(uintUnits));
        break;
    default:
        printf("Error: Unsupported Sampler Info type.\n");
        ec = WebCLException::INVALID_SAMPLER;
        return WebCLGetInfo();
    }
    switch (err) {
    case CL_INVALID_VALUE:
        ec = WebCLException::INVALID_VALUE;
        printf("Error: CL_INVALID_VALUE \n");
        break;
    case CL_INVALID_SAMPLER:
        ec = WebCLException::INVALID_SAMPLER;
        printf("Error: CL_INVALID_SAMPLER\n");
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
        ec = WebCLException::INVALID_SAMPLER;
        printf("Error: Invaild Error Type\n");
        break;
    }
    return WebCLGetInfo();
}

CCSampler WebCLSampler::getCLSampler()
{
    return m_ccSampler;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
