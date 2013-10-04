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

#ifndef WebCLProgram_h
#define WebCLProgram_h

#if ENABLE(WEBCL)

#include "WebCLObject.h"

namespace WebCore {

class WebCLContext;
class WebCLDevice;
class WebCLFinishCallback;
class WebCLGetInfo;
class WebCLKernel;

class WebCLProgram : public WebCLObject<CCProgram> {
public:
    virtual ~WebCLProgram();
    static PassRefPtr<WebCLProgram> create(WebCLContext*, const String& programSource, ExceptionCode&);

    WebCLGetInfo getInfo(int flag, ExceptionCode&);
    WebCLGetInfo getBuildInfo(WebCLDevice*, int flag, ExceptionCode&);

    void build(const Vector<RefPtr<WebCLDevice> >&, const String& buildOptions, PassRefPtr<WebCLFinishCallback>, ExceptionCode&);

    PassRefPtr<WebCLKernel> createKernel(const String& kernelName, ExceptionCode&);

    Vector<RefPtr<WebCLKernel> > createKernelsInProgram(ExceptionCode&);

    void release();

private:
    WebCLProgram(WebCLContext*, CCProgram, const String&);
    static void finishCallback(CCProgram, void*);
    void releasePlatformObjectImpl();

    static WebCLProgram* thisPointer;
    WebCLContext* m_context;
    String m_kernelSource;
    RefPtr<WebCLFinishCallback> m_finishCallback;
};

} // namespace WebCore

#endif
#endif // WebCLProgram_h
