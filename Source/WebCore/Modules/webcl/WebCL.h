/*
 * Copyright (C) 2011, 2012, 2013 Samsung Electronics Corporation. All rights reserved.
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

#ifndef WebCL_h
#define WebCL_h

#if ENABLE(WEBCL)

#include "WebCLContext.h"
#include "WebCLEvent.h"
#include "WebCLExtension.h"
#include "WebCLPlatform.h"

namespace WebCore {

class WebCLContextProperties;

class WebCL : public RefCounted<WebCL> , public WebCLExtensionsAccessor<> {
public:
    static PassRefPtr<WebCL> create();
    virtual ~WebCL() { }

    Vector<RefPtr<WebCLPlatform> > getPlatforms(ExceptionCode&);

    void waitForEvents(const Vector<RefPtr<WebCLEvent> >&, ExceptionCode&);

    PassRefPtr<WebCLContext> createContext(PassRefPtr<WebCLContextProperties>, ExceptionCode&);

protected:
    WebCL();

    template <class T>
    void inline createContextBase(PassRefPtr<WebCLContextProperties> properties, ExceptionCode& ec, RefPtr<T>& out);

private:
    RefPtr<WebCLContextProperties>& defaultProperties(ExceptionCode&);
    RefPtr<WebCLContextProperties> m_defaultProperties;
    Vector<RefPtr<WebCLPlatform> > m_platforms;
};

template <class T>
inline void WebCL::createContextBase(PassRefPtr<WebCLContextProperties> properties, ExceptionCode& ec, RefPtr<T>& outValue)
{
    outValue = 0;

    RefPtr<WebCLContextProperties> refProperties = properties;
    if (refProperties) {
        if (!refProperties->platform()) {
            Vector<RefPtr<WebCLPlatform> > webCLPlatforms = getPlatforms(ec);
            if (ec != WebCLException::SUCCESS)
                return;

            refProperties->setPlatform(webCLPlatforms[0]);
        }

        if (!refProperties->devices().size()) {
            Vector<RefPtr<WebCLDevice> > webCLDevices = refProperties->platform()->getDevices(ComputeContext::DEVICE_TYPE_DEFAULT, ec);
            if (ec != WebCLException::SUCCESS)
                return;

            refProperties->devices().append(webCLDevices[0]);
        }
    } else {
        refProperties = defaultProperties(ec);
        if (ec != WebCLException::SUCCESS)
            return;
    }

    CCerror error = ComputeContext::SUCCESS;
    outValue = T::create(refProperties.release(), error);
    if (!outValue) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
    }
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCL_h
