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

#include "WebCLEvent.h"

#include "WebCLCallback.h"
#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLGetInfo.h"
#include "WebCLImageDescriptor.h"
#include "WebCLMemoryObject.h"
#include "WebCLProgram.h"

namespace WebCore {
WebCLEvent::~WebCLEvent()
{
    releasePlatformObject();
}

PassRefPtr<WebCLEvent> WebCLEvent::create()
{
    // the platform object is initialized by enqueue methods
    return adoptRef(new WebCLEvent(0));
}

WebCLEvent::WebCLEvent(CCEvent event)
    : WebCLObjectImpl(event)
    , m_commandQueue(0)
{
}

WebCLGetInfo WebCLEvent::getInfo(CCenum paramName, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_EVENT;
        return WebCLGetInfo();
    }
    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::EVENT_COMMAND_EXECUTION_STATUS: {
        CCuint ccExecStatus = 0;
        err = ComputeContext::getEventInfo(platformObject(), paramName, &ccExecStatus);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(ccExecStatus));
        break;
    }
    case ComputeContext::EVENT_COMMAND_TYPE: {
        CCCommandType ccCommandType = 0;
        err= ComputeContext::getEventInfo(platformObject(), paramName, &ccCommandType);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(ccCommandType));
        break;
    }
    case ComputeContext::EVENT_CONTEXT: {
        if (!m_commandQueue)
            return WebCLGetInfo();

        ASSERT(m_commandQueue->m_context);
        return WebCLGetInfo(m_commandQueue->m_context.get());
    }
    case ComputeContext::EVENT_COMMAND_QUEUE: {
        if (!m_commandQueue)
            return WebCLGetInfo();

        return WebCLGetInfo(m_commandQueue.get());
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

WebCLGetInfo WebCLEvent::getProfilingInfo(CCenum paramName, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_EVENT;
        return WebCLGetInfo();
    }
    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::PROFILING_COMMAND_QUEUED:
    case ComputeContext::PROFILING_COMMAND_SUBMIT:
    case ComputeContext::PROFILING_COMMAND_START:
    case ComputeContext::PROFILING_COMMAND_END: {
        CCulong eventProfilingInfo = 0;
        err = ComputeContext::getEventProfilingInfo(platformObject(), paramName, &eventProfilingInfo);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(eventProfilingInfo));
        }
        break;
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }
    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

void WebCLEvent::setAssociatedCommandQueue(WebCLCommandQueue* commandQueue)
{
    m_commandQueue = commandQueue;
}

WebCLEvent* WebCLEvent::thisPointer = 0;

void WebCLEvent::setCallback(CCenum executionStatus, PassRefPtr<WebCLCallback> notify, ExceptionCode& ec)
{
    UNUSED_PARAM(executionStatus);
    UNUSED_PARAM(notify);
    UNUSED_PARAM(ec);
    // FIXME :: Callback implementation is not working. Need to rework.
    // Issue # 102
    return;
}

bool WebCLEvent::isPlatformObjectNeutralized() const
{
    return isReleased();
}

void WebCLEvent::releasePlatformObjectImpl()
{
    CCerror computeContextErrorCode = m_commandQueue->m_context->computeContext()->releaseEvent(platformObject());
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)

