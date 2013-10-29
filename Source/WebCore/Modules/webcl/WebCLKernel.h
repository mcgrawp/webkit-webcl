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
#include "WebCLInputChecker.h"
#include "WebCLMemoryObject.h"
#include "WebCLObject.h"

namespace WTF {
class ArrayBufferView;
}

namespace WebCore {

class WebCLGetInfo;
class WebCLContext;
class WebCLDevice;
class WebCLProgram;
class WebCLMemoryObject;
class WebCLSampler;

class WebCLKernel : public WebCLObject<CCKernel> {
public:
    class Argument {
    public:
        Argument(const String& argumentDeclaration);

        enum Qualifier {
            Local,
            Global,
            Const
        };
        bool hasQualifier(enum Qualifier qualifier) const;
    private:
        bool m_hasLocalQualifier;
        bool m_hasGlobalQualifier;
        bool m_hasConstQualifier;
    };

    class ArgumentList {
    public:
        ArgumentList(WebCLKernel*);

        const Vector<Argument>& it();
    private:
        void ensureArgumentData();

        WebCLKernel* m_kernel;
        Vector<Argument> m_argumentData;
    };

    virtual ~WebCLKernel();
    static PassRefPtr<WebCLKernel> create(WebCLContext*, WebCLProgram*, const String&, ExceptionCode&);
    static Vector<RefPtr<WebCLKernel> > createKernelsInProgram(WebCLContext*, WebCLProgram*, ExceptionCode&);
    WebCLGetInfo getInfo(CCenum, ExceptionCode&);
    WebCLGetInfo getWorkGroupInfo(WebCLDevice*, CCenum, ExceptionCode&);

    void setArg(CCuint index, WebCLMemoryObject*, ExceptionCode&);
    void setArg(CCuint index, WebCLSampler*, ExceptionCode&);
    void setArg(CCuint index, ArrayBufferView*, ExceptionCode&);

    void setDevice(PassRefPtr<WebCLDevice>);

    unsigned numberOfArguments();

private:
    WebCLKernel(WebCLContext*, WebCLProgram*, CCKernel, const String&);

    const Argument& argument(unsigned index);

    void releasePlatformObjectImpl();

    WebCLContext* m_context;
    WebCLProgram* m_program;
    String m_kernelName;
    // FIXME: Kernels keep their own copies of the program source
    // instead of querying it from the m_program backpointer. This
    // happens because the object ownership is yet not well defined,
    // and it is possible that a program gets garbage collected (and deleted).
    String m_programSource;
    RefPtr<WebCLDevice> m_deviceID;
    ArgumentList m_argumentList;

    friend class ArgumentList;
};

} // namespace WebCore

#endif
#endif // WebCLKernel_h
