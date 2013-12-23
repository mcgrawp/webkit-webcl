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

class WebCLContext;
class WebCLDevice;
class WebCLGetInfo;
class WebCLKernel;
class WebCLMemoryObject;
class WebCLProgram;
class WebCLSampler;

class WebCLKernelArgInfo : public RefCounted<WebCLKernelArgInfo> {
public:
    WebCLKernelArgInfo(const String& addressQualifier, const String& accessQualifier, const String& type, const String& name)
        : m_addressQualifier(addressQualifier)
        , m_accessQualifier(accessQualifier)
        , m_type(type)
        , m_name(name)
    {
    }

    String name() const { return m_name; }
    String typeName() const { return m_type; }
    String addressQualifier() const { return m_addressQualifier; }
    String accessQualifier() const { return m_accessQualifier; }

private:
    String m_addressQualifier;
    String m_accessQualifier;
    String m_type;
    String m_name;
};

class WebCLKernelArgInfoProvider {
public:
    WebCLKernelArgInfoProvider(WebCLKernel*);

    const Vector<RefPtr<WebCLKernelArgInfo> >& argumentsInfo();

private:
    void ensureInfo();

    void parseAndAppendDeclaration(const String& argumentDeclaration);
    String extractAddressQualifier(Vector<String>& declaration);
    String extractAccessQualifier(Vector<String>& declaration);
    String extractType(Vector<String>& declaration);
    String extractName(Vector<String>& declaration);

    WebCLKernel* m_kernel;
    Vector<RefPtr<WebCLKernelArgInfo> > m_argumentInfoVector;
};

class WebCLKernel : public WebCLObjectImpl<CCKernel> {
public:
    virtual ~WebCLKernel();
    static PassRefPtr<WebCLKernel> create(WebCLContext*, WebCLProgram*, const String&, ExceptionCode&);
    static Vector<RefPtr<WebCLKernel> > createKernelsInProgram(WebCLContext*, WebCLProgram*, ExceptionCode&);

    WebCLGetInfo getInfo(CCenum, ExceptionCode&);
    WebCLGetInfo getWorkGroupInfo(WebCLDevice*, CCenum, ExceptionCode&);

    void setArg(CCuint index, WebCLMemoryObject*, ExceptionCode&);
    void setArg(CCuint index, WebCLSampler*, ExceptionCode&);
    void setArg(CCuint index, ArrayBufferView*, ExceptionCode&);

    WebCLProgram* program() const;
    String kernelName() const;

    unsigned numberOfArguments();

private:
    WebCLKernel(WebCLContext*, WebCLProgram*, CCKernel, const String&);

    void releasePlatformObjectImpl();

    RefPtr<WebCLContext> m_context;
    RefPtr<WebCLProgram> m_program;
    String m_kernelName;

    WebCLKernelArgInfoProvider m_argumentInfoProvider;
};

} // namespace WebCore

#endif
#endif // WebCLKernel_h
