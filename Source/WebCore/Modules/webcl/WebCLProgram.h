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

#include "ExceptionCode.h"
#include <OpenCL/opencl.h>
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class WebCL;
class WebCLContext;
class WebCLGetInfo;
class WebCLKernel;
class WebCLKernelList;
class WebCLFinishCallback;
class WebCLDevice;
class WebCLDeviceList;

class WebCLProgram : public RefCounted<WebCLProgram> {
public:
    virtual ~WebCLProgram();
    static PassRefPtr<WebCLProgram> create(WebCLContext*, cl_program);

    WebCLGetInfo getInfo(int, ExceptionCode&);
    WebCLGetInfo getBuildInfo(WebCLDevice*, int, ExceptionCode&);

    void build(WebCLDeviceList*, const String&, PassRefPtr<WebCLFinishCallback>, int, ExceptionCode&);
    void build(WebCLDeviceList* devices, const String& options, PassRefPtr<WebCLFinishCallback> finishCallBack,
        ExceptionCode& ec)
    {
        return build(devices, options, finishCallBack, 0, ec);
    }
    void build(WebCLDeviceList* devices, const String& options, ExceptionCode& ec)
    {
        return build(devices, options, 0, 0, ec);
    }
    void build(WebCLDeviceList* devices, ExceptionCode& ec)
    {
        return build(devices, "", 0, 0, ec);
    }

    PassRefPtr<WebCLKernel> createKernel(const String&, ExceptionCode&);

    void buildProgram(int, int, int, ExceptionCode&);
    void buildProgram(WebCLDevice*, int, int, int, ExceptionCode&);
    void buildProgram(WebCLDeviceList*, int, int, int, ExceptionCode&);
    PassRefPtr<WebCLKernelList> createKernelsInProgram(ExceptionCode&);

    void releaseProgram(ExceptionCode&);
    RefPtr<WebCLFinishCallback> m_finishCallback;

private:
    WebCLProgram(WebCLContext*, cl_program);
    static void finishCallBack(cl_program, void*);
    static WebCLProgram* thisPointer;
    WebCLContext* m_context;
    cl_program m_clProgram;
};

} // namespace WebCore

#endif // WebCLProgram_h
