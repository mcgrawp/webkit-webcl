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

#include "ExceptionCode.h"
#include "WebCLGetInfo.h"
#include "WebCLKernelTypes.h"
#include <OpenCL/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class WebCL;
class WebCLGetInfo;
class WebCLDevice;
class WebCLMemoryObject;

class WebCLKernel : public RefCounted<WebCLKernel> {
public:
    virtual ~WebCLKernel();
    static PassRefPtr<WebCLKernel> create(WebCLContext*, cl_kernel);
    WebCLGetInfo getInfo(int, ExceptionCode&);
    WebCLGetInfo getWorkGroupInfo(WebCLDevice*, int, ExceptionCode&);

    void setArg(unsigned, PassRefPtr<WebCLKernelTypeValue>, unsigned, ExceptionCode&);
    void setArg(unsigned, WebCLMemoryObject*, ExceptionCode&);
    void setArg(unsigned, unsigned, unsigned, ExceptionCode&);
    void setArg(unsigned, unsigned, ExceptionCode&);

    void setDevice(PassRefPtr<WebCLDevice>);
    cl_kernel getCLKernel();

private:
    WebCLKernel(WebCLContext*, cl_kernel);
    // private fucntions simulating opencl API for WebCLKernelTypeValue object.
    template<class T> unsigned clSetKernelArgPrimitiveType(cl_kernel, RefPtr<WebCLKernelTypeValue>, T, unsigned, int);
    unsigned clSetKernelArgVectorType(cl_kernel, RefPtr<WebCLKernelTypeValue>, RefPtr<WebCLKernelTypeVector>, unsigned,
        int, unsigned);

    WebCLContext* m_context;
    cl_kernel m_clKernel;
    RefPtr<WebCLDevice> m_deviceID;
    Vector<RefPtr<WebCLKernel> > m_kernelList;
    long m_numKernels;
};

} // namespace WebCore

#endif // WebCLKernel_h
