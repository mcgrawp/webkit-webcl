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

#include "WebCLGL.h"

#include "WebCLGLContext.h"

namespace WebCore {

#if 0
PassRefPtr<WebCLGL> WebCLGL::create()
{
    return adoptRef(new WebCLGL);
}
#else
WebCLGL* WebCLGL::create()
{
    return new WebCLGL;
}
#endif

WebCLGL::WebCLGL()
{
    // For now, we call relaxAdoptionRequirement to not assert.
    // See issue #223.
    relaxAdoptionRequirement();
}

WebCLGL::~WebCLGL()
{
    // FIXME: We manually call derefBase here, since
    // WebCLGL is a RefCounted-derived class, but we use
    // OwnPtr to store its instance.
    // See issue #233.
    derefBase();
}

WebCLGL::ExtensionName WebCLGL::getName() const
{
    return KhrGLSharingName;
}

PassRefPtr<WebCLGLContext> WebCLGL::createContext(PassRefPtr<WebCLContextProperties> properties, ExceptionCode& ec)
{
    RefPtr<WebCLGLContext> context;
    createContextBase(properties, ec, context);
    return context.release();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
