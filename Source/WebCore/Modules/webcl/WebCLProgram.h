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

#include "ComputeContext.h"
#include "ExceptionCode.h"
#include "WebCLFinishCallback.h"

#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class WebCL;
class WebCLContext;
class WebCLGetInfo;
class WebCLKernel;
class WebCLDevice;

class WebCLProgram : public RefCounted<WebCLProgram> {
public:
    virtual ~WebCLProgram();
    static PassRefPtr<WebCLProgram> create(WebCLContext*, const String&, ExceptionCode&);

    WebCLGetInfo getInfo(int, ExceptionCode&);
    WebCLGetInfo getBuildInfo(WebCLDevice*, int, ExceptionCode&);

    void build(const Vector<RefPtr<WebCLDevice> >&, const String&, PassRefPtr<WebCLFinishCallback>, int, ExceptionCode&);
    void build(const Vector<RefPtr<WebCLDevice> >& devices, const String& options, PassRefPtr<WebCLFinishCallback> finishCallback, ExceptionCode& ec)
    {
        return build(devices, options, finishCallback, 0, ec);
    }
    void build(const Vector<RefPtr<WebCLDevice> > devices, const String& options, ExceptionCode& ec)
    {
        return build(devices, options, 0, 0, ec);
    }
    void build(const Vector<RefPtr<WebCLDevice> >& devices, ExceptionCode& ec)
    {
        return build(devices, emptyString(), 0, 0, ec);
    }

    PassRefPtr<WebCLKernel> createKernel(const String&, ExceptionCode&);

    Vector<RefPtr<WebCLKernel> > createKernelsInProgram(ExceptionCode&);

    CCProgram getCLProgram();

    void releaseProgram(ExceptionCode&);

private:
    WebCLProgram(WebCLContext*, cl_program, const String&);
    static void finishCallback(cl_program, void*);
    static WebCLProgram* thisPointer;
    WebCLContext* m_context;
    CCProgram m_clProgram;
    String m_kernelSource;
    RefPtr<WebCLFinishCallback> m_finishCallback;
};

} // namespace WebCore

#endif
#endif // WebCLProgram_h
