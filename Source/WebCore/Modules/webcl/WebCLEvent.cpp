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

#include "WebCLContext.h"
#include "WebCLException.h"

namespace WebCore {
WebCLEvent::~WebCLEvent()
{
    ASSERT(m_clEvent);
    CCerror computeContextErrorCode = m_context->computeContext()->releaseEvent(m_clEvent);

    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
    m_clEvent = 0;
}

PassRefPtr<WebCLEvent> WebCLEvent::create(WebCLContext* context, CCEvent Event)
{
    return adoptRef(new WebCLEvent(context, Event));
}

WebCLEvent::WebCLEvent(WebCLContext* context, CCEvent event)
    : m_context(context)
    , m_clEvent(event)
{
}


WebCLContext* WebCLEvent::getContext()
{
    return m_context;
}

WebCLGetInfo WebCLEvent::getInfo(int paramName, ExceptionCode& ec)
{
    if (!m_clEvent) {
        ec = WebCLException::INVALID_EVENT;
        return WebCLGetInfo();
    }
    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::EVENT_COMMAND_EXECUTION_STATUS: {
        CCuint ccExecStatus = 0;
        err = ComputeContext::getEventInfo(m_clEvent, paramName, &ccExecStatus);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(ccExecStatus));
        break;
    }
    case ComputeContext::EVENT_COMMAND_TYPE: {
        CCCommandType ccCommandType = 0;
        err= ComputeContext::getEventInfo(m_clEvent, paramName, &ccCommandType);
        if (err == CL_SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(ccCommandType));
        break;
    }
    case ComputeContext::EVENT_COMMAND_QUEUE: {
        /* FIXME: Cannot create a CommandQueue here */
        /*CCCommandQueue ccCommandQueue = 0;
        err = ComputeContext::getEventInfo(m_clEvent, paramName, sizeof(CCCommandQueue), &ccCommandQueue);
        RefPtr<WebCLCommandQueue> commandQueue = WebCLCommandQueue::create(m_context, ccCommandQueue);
        if (!commandQueue)
            return WebCLGetInfo();

        if (err == CL_SUCCESS)
            return WebCLGetInfo(commandQueue.release());
        */

        break;
    }
    case ComputeContext::EVENT_CONTEXT: {
        /* FIXME: Cannot create Context here.
        err = ComputeContext::getEventInfo(m_clEvent, paramName, sizeof(myContext), myContext);;
        if (err == CL_SUCCESS)
            return WebCLGetInfo((String)myContext);
        }
        */
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

WebCLGetInfo WebCLEvent::getProfilingInfo(int paramName, ExceptionCode& ec)
{
    if (!m_clEvent) {
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
        err = ComputeContext::getEventProfilingInfo(m_clEvent, paramName, &eventProfilingInfo);
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

void WebCLEvent::setUserEventStatus(int execStatus, ExceptionCode& ec)
{
    if (!m_clEvent) {
        ec = WebCLException::INVALID_EVENT;
        return;
    }
    CCerror err = 0;
    if (execStatus == ComputeContext::COMPLETE || execStatus < 0)
        err = clSetUserEventStatus(m_clEvent, execStatus);
    else {
        err = WebCLException::INVALID_VALUE;
        return;
    }
    ASSERT(err != CL_SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return;
}

WebCLEvent* WebCLEvent::thisPointer = 0;

void WebCLEvent::setCallback(int executionStatus, PassRefPtr<WebCLFinishCallback> notify, int userData, ExceptionCode& ec)
{
    UNUSED_PARAM(executionStatus);
    UNUSED_PARAM(notify);
    UNUSED_PARAM(userData);
    UNUSED_PARAM(ec);
    // FIXME :: Callback implementation is not working. Need to rework.
    // Issue # 102
    return;
}

CCEvent WebCLEvent::getCLEvent()
{
    return m_clEvent;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)

