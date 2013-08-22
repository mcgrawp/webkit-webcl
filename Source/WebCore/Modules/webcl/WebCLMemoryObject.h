/*
 * Copyright (C) 2011, 2013 Samsung Electronics Corporation. All rights reserved.
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

#ifndef WebCLMemoryObject_h
#define WebCLMemoryObject_h

#if ENABLE(WEBCL)

#include "ComputeContext.h"
#include "WebCLException.h"
#include "WebCLObject.h"

namespace WebCore {

class WebCLContext;
class WebCLException;
class WebCLGLObjectInfo;
class WebCLGetInfo;

class WebCLMemoryObject : public WebCLObject<PlatformComputeObject> {
public:
    virtual ~WebCLMemoryObject();
    static PassRefPtr<WebCLMemoryObject> create(WebCLContext*, PlatformComputeObject);

    virtual bool isShared() const { return false; }
    WebCLGetInfo getInfo(int, ExceptionCode&);

protected:
    WebCLMemoryObject(WebCLContext*, PlatformComputeObject, WebCLMemoryObject* = 0);
    virtual void releasePlatformObjectImpl();

    WebCLContext* m_context;
    WebCLMemoryObject* m_parentMemObject;
};

} // namespace WebCore

#endif
#endif // WebCLMemoryObject_h
