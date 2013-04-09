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

#ifndef WebCLKernel_h
#define WebCLKernel_h

#if ENABLE(WEBCL)

#include "ExceptionCode.h"
#include "WebCLGetInfo.h"
#include "WebCLKernelTypes.h"
#include "WebCLObject.h"

#include <OpenCL/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class WebCL;
class WebCLContext;
class WebCLGetInfo;
class WebCLDevice;
class WebCLMemoryObject;

class WebCLKernel : public WebCLObject<CCKernel> {
public:
    virtual ~WebCLKernel();
    static PassRefPtr<WebCLKernel> create(WebCLContext*, WebCLProgram*, const String&, ExceptionCode&);
    static Vector<RefPtr<WebCLKernel> > createKernelsInProgram(WebCLContext*, WebCLProgram*, ExceptionCode&);
    WebCLGetInfo getInfo(int, ExceptionCode&);
    WebCLGetInfo getWorkGroupInfo(WebCLDevice*, int, ExceptionCode&);

    void setArg(unsigned, PassRefPtr<WebCLKernelTypeValue>, unsigned, ExceptionCode&);
    void setArg(unsigned, WebCLMemoryObject*, ExceptionCode&);
    void setArg(unsigned, unsigned, unsigned, ExceptionCode&);
    void setArg(unsigned, unsigned, ExceptionCode&);

    void setDevice(PassRefPtr<WebCLDevice>);
private:
    WebCLKernel(WebCLContext*, WebCLProgram*, CCKernel, const String&);
    // private fucntions simulating opencl API for WebCLKernelTypeValue object.
    template<class T>
    CCerror setKernelArgPrimitiveType(CCKernel, PassRefPtr<WebCLKernelTypeValue>, T, unsigned, int);
    CCerror setKernelArgVectorType(CCKernel, PassRefPtr<WebCLKernelTypeValue>, PassRefPtr<WebCLKernelTypeVector>, unsigned, int, unsigned);

    void releasePlatformObjectImpl();

    WebCLContext* m_context;
    WebCLProgram* m_program;
    String m_kernelName;
    RefPtr<WebCLDevice> m_deviceID;
};

} // namespace WebCore

#endif
#endif // WebCLKernel_h
