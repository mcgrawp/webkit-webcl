/*
 * Copyright (C) 2013 Samsung Electronics Corporation. All rights reserved.
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

#if ENABLE(WEBCL) && ENABLE(WEBGL)

#include "WebCLGLCommandQueue.h"

#include "WebCLContext.h"
#include "WebCLEvent.h"
#include "WebCLMemoryObject.h"

namespace WebCore {


PassRefPtr<WebCLGLCommandQueue> WebCLGLCommandQueue::create(WebCLContext* context, int queueProperties, const RefPtr<WebCLDevice>& webCLDevice, ExceptionCode& ec)
{
    RefPtr<WebCLGLCommandQueue> queue;
    createBase(context, queueProperties, webCLDevice, ec, queue);
    return queue.release();
}

void WebCLGLCommandQueue::enqueueAcquireGLObjects(const Vector<RefPtr<WebCLMemoryObject> >& memoryObjects, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    Vector<PlatformComputeObject> ccMemoryObjectIDs;
    for (size_t i = 0; i < memoryObjects.size(); ++i) {
        if (!memoryObjects[i]->isShared()) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
        ccMemoryObjectIDs.append(memoryObjects[i]->platformObject());
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    // FIXME: Crash!?
    CCEvent* ccEvent = 0;
    if (event)
        *ccEvent = event->platformObject();

    CCerror err = m_context->computeContext()->enqueueAcquireGLObjects(platformObject(), ccMemoryObjectIDs,
        ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLGLCommandQueue::enqueueReleaseGLObjects(const Vector<RefPtr<WebCLMemoryObject> >& memoryObjects, const Vector<RefPtr<WebCLEvent> >& events, WebCLEvent* event, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_COMMAND_QUEUE;
        return;
    }

    Vector<PlatformComputeObject> ccMemoryObjectIDs;
    for (size_t i = 0; i < memoryObjects.size(); ++i) {
        if (!memoryObjects[i]->isShared()) {
            ec = WebCLException::INVALID_MEM_OBJECT;
            return;
        }
        ccMemoryObjectIDs.append(memoryObjects[i]->platformObject());
    }

    Vector<CCEvent> ccEvents;
    for (size_t i = 0; i < events.size(); ++i)
        ccEvents.append(events[i]->platformObject());

    // FIXME: Crash!?
    CCEvent* ccEvent = 0;
    if (event)
        *ccEvent = event->platformObject();

    CCerror err = m_context->computeContext()->enqueueReleaseGLObjects(platformObject(), ccMemoryObjectIDs,
        ccEvents.size(), ccEvents.data(), ccEvent);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
