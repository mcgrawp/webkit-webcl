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

#ifndef WebCL_h
#define WebCL_h

#if ENABLE(WEBCL)

#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLDevice.h"
#include "WebCLEvent.h"
#include "WebCLMemoryObject.h"
#include "WebCLPlatform.h"
#include "WebCLProgram.h"
#include "WebCLSampler.h"
#include <OpenCL/opencl.h>

using namespace std;

namespace WebCore {

class ScriptExecutionContext;
class WebCLImage;
class WebCLException;
class WebCLContextProperties;

class WebCL : public RefCounted<WebCL> {
public:
    virtual ~WebCL();

    // FIXME: Remove the enum below in favor of constants.
    enum {
        // Buffer size
        CHAR_BUFFER_SIZE                         = 1024,

        // Image Discriptor defaults
        DEFAULT_OBJECT_CHANNELORDER              = 0x10B5,
        DEFAULT_OBJECT_CHANNELTYPE               = 0x10D2,
        DEFAULT_OBJECT_WIDTH                     = 0, // CL_DEVICE_IMAGE2D_MAX_WIDTH
        DEFAULT_OBJECT_HEIGHT                    = 0, // CL_DEVICE_IMAGE2D_MAX_HEIGHT
        DEFAULT_OBJECT_ROWPITCH                  = 0,
    };

    Vector<RefPtr<WebCLPlatform> > getPlatforms(ExceptionCode&) const;
    void waitForEvents(const Vector<RefPtr<WebCLEvent> >&, ExceptionCode&);
    PassRefPtr<WebCLContext> createContext(PassRefPtr<WebCLContextProperties>, ExceptionCode&);
    Vector<String> getSupportedExtensions(ExceptionCode&);

    static PassRefPtr<WebCL> create();

private:
    WebCL();
    RefPtr<WebCLContextProperties>& defaultProperties(ExceptionCode&);
    Vector<RefPtr<WebCLContext> > m_context;
    RefPtr<WebCLContextProperties> m_defaultProperties;
};

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCL_h
