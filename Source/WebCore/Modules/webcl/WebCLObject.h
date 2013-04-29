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

#include <wtf/RefCounted.h>

namespace WebCore {

template <class T>
class WebCLObject : public RefCounted<WebCLObject<T> > {
public:
    virtual ~WebCLObject()
    {
    }

    T platformObject() const { return m_platformObject; }

    bool isReleased() const { return m_isReleased; }

    void releasePlatformObject()
    {
        if (!m_platformObject)
            return;

        ASSERT(m_isReleased);

        releasePlatformObjectImpl();
        m_platformObject = 0;
        m_isReleased = true;
    }

protected:
    WebCLObject(T object)
        : m_platformObject(object)
        , m_isReleased(false)
    {
    }

    // WebCLEvent uses this constructor
    WebCLObject()
        : m_platformObject(0)
        , m_isReleased(false)
    {
    }

    virtual void releasePlatformObjectImpl() { }

private:
    T m_platformObject;
    bool m_isReleased;
};

} // WebCore

#endif // ENABLE(WEBCL)

#endif // WebCLProgram_h
