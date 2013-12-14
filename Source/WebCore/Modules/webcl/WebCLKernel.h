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
    typedef enum {
        Global,
        Local,
        Constant,
        Private
    } AddressQualifier;

    typedef enum {
        ReadOnly,
        WriteOnly,
        ReadWrite,
        None
    } AccessQualifier;

    typedef enum {
        CL_MEM,
        IMAGE2D_T,
        SAMPLER_T,
        HALF,
        CHAR,   CHAR16,   CHAR2,   CHAR3,   CHAR4,   CHAR8,
        DOUBLE, DOUBLE16, DOUBLE2, DOUBLE3, DOUBLE4, DOUBLE8,
        FLOAT,  FLOAT16,  FLOAT2,  FLOAT3,  FLOAT4,  FLOAT8,
        INT,    INT16,    INT2,    INT3,    INT4,    INT8,
        LONG,   LONG16,   LONG2,   LONG3,   LONG4,   LONG8,
        SHORT,  SHORT16,  SHORT2,  SHORT3,  SHORT4,  SHORT8,
        UCHAR,  UCHAR16,  UCHAR2,  UCHAR3,  UCHAR4,  UCHAR8,
        UINT,   UINT16,   UINT2,   UINT3,   UINT4,   UINT8,
        ULONG,  ULONG16,  ULONG2,  ULONG3,  ULONG4,  ULONG8,
        USHORT, USHORT16, USHORT2, USHORT3, USHORT4, USHORT8
    } ScalarType;

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
    String extractNameOrType(Vector<String>& declaration);

    WebCLKernel* m_kernel;
    Vector<RefPtr<WebCLKernelArgInfo> > m_argumentInfoVector;
};

class WebCLKernel : public WebCLObjectImpl<CCKernel> {
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

    WebCLProgram* program() const;
    String kernelName() const;

    unsigned numberOfArguments();

private:
    WebCLKernel(WebCLContext*, WebCLProgram*, CCKernel, const String&);

    const Argument& argument(unsigned index);

    void releasePlatformObjectImpl();

    RefPtr<WebCLContext> m_context;
    RefPtr<WebCLProgram> m_program;
    String m_kernelName;

    ArgumentList m_argumentList;

    WebCLKernelArgInfoProvider m_argumentInfoProvider;

    friend class ArgumentList;
};

} // namespace WebCore

#endif
#endif // WebCLKernel_h
