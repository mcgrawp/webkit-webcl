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
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS, OR BUSINESS INTERRUPTION), HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
 * NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WebCLGLCommandQueue_h
#define WebCLGLCommandQueue_h

#if ENABLE(WEBCL) && ENABLE(WEBGL)

#include "WebCLCommandQueue.h"

namespace WebCore {

class WebCLContext;
class WebCLMemoryObject;

class WebCLGLCommandQueue : public WebCLCommandQueue {
public:
    ~WebCLGLCommandQueue() { }

    static PassRefPtr<WebCLGLCommandQueue> create(WebCLContext*, int queueProperties, const RefPtr<WebCLDevice>&, ExceptionCode&);

    void enqueueAcquireGLObjects(const Vector<RefPtr<WebCLMemoryObject> >&, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);
    void enqueueReleaseGLObjects(const Vector<RefPtr<WebCLMemoryObject> >&, const Vector<RefPtr<WebCLEvent> >&, WebCLEvent*, ExceptionCode&);

private:
    WebCLGLCommandQueue(WebCLContext* context, const RefPtr<WebCLDevice>& devices, CCCommandQueue ccCommandQueue)
        : WebCLCommandQueue(context, devices, ccCommandQueue)
    {
    }
    friend class WebCLCommandQueue;
};

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCLGLCommandQueue_h
