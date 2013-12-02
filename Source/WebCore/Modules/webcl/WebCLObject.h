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

#ifndef WebCLObject_h
#define WebCLObject_h

#if ENABLE(WEBCL)

#include "ComputeContext.h"
#include "ExceptionCode.h"
#include "WebCLException.h"
#include <wtf/RefCounted.h>
#include <wtf/WeakPtr.h>

namespace WebCore {

class WebCLAgnosticObject : public RefCounted<WebCLAgnosticObject> {
public:
    virtual ~WebCLAgnosticObject()
    {
    }

    bool isReleased() const { return m_isReleased; }
    virtual void release() { }

    WeakPtr<WebCLAgnosticObject> createWeakPtr() { return m_weakFactory.createWeakPtr(); }

protected:
    WebCLAgnosticObject()
        : m_weakFactory(this)
        , m_isReleased(false)
    {
    }

    WeakPtrFactory<WebCLAgnosticObject> m_weakFactory;
    bool m_isReleased;
};

template <class T>
class WebCLObject : public WebCLAgnosticObject {
public:
    virtual ~WebCLObject()
    {
    }

    T platformObject() const { return m_platformObject; }

    virtual void release()
    {
        releasePlatformObject();
    }

    virtual void releasePlatformObject()
    {
        if (!m_platformObject)
            return;

        ASSERT(!m_isReleased);

        releasePlatformObjectImpl();
        m_platformObject = 0;
        m_isReleased = true;
    }

protected:
    WebCLObject(T object)
        : m_platformObject(object)
    {
    }

    virtual void releasePlatformObjectImpl() { }

private:
    T m_platformObject;
};

} // WebCore

#endif // ENABLE(WEBCL)

#endif // WebCLObject_h
