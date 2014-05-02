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

#ifndef WebCLKernelArgInfo_h
#define WebCLKernelArgInfo_h

#if ENABLE(WEBCL)

#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class WebCLKernelArgInfo : public RefCounted<WebCLKernelArgInfo> {
public:
    enum WebCLKernelTypes {
        CHAR,
        UCHAR,
        SHORT,
        USHORT,
        INT,
        UINT,
        LONG,
        ULONG,
        FLOAT,
        BUFFER,
        SAMPLER,
        IMAGE,
        UNKNOWN
    };

    static PassRefPtr<WebCLKernelArgInfo> create(const String& addressQualifier, const String& accessQualifier, const String& type, const String& name, const bool isPointerType = false) { return adoptRef(new WebCLKernelArgInfo(addressQualifier, accessQualifier, type, name, isPointerType)); }

    String name() const { return m_name; }
    String typeName() const { return m_type; }
    String addressQualifier() const { return m_addressQualifier; }
    String accessQualifier() const { return m_accessQualifier; }
    WebCLKernelTypes type() const { return m_types; }
    bool hasLocalAddressQualifier() const { return m_hasLocalAddressQualifier; }

private:
    WebCLKernelArgInfo(const String& addressQualifier, const String& accessQualifier, const String& type, const String& name, const bool isPointerType)
        : m_addressQualifier(addressQualifier)
        , m_accessQualifier(accessQualifier)
        , m_type(type)
        , m_name(name)
    {
        m_types = extractTypeEnum(m_type, isPointerType);
        m_hasLocalAddressQualifier = (m_addressQualifier == "local");
    }
    String m_addressQualifier;
    String m_accessQualifier;
    String m_type;
    String m_name;
    WebCLKernelTypes m_types;
    bool m_hasLocalAddressQualifier;

    static inline WebCLKernelTypes extractTypeEnum(const String& typeName, bool isPointerType)
    {
        if (isPointerType)
            return WebCLKernelTypes::BUFFER;

        DEFINE_STATIC_LOCAL(AtomicString, image2d_t, ("image2d_t", AtomicString::ConstructFromLiteral));
        if (typeName == image2d_t)
            return WebCLKernelTypes::IMAGE;

        DEFINE_STATIC_LOCAL(AtomicString, sampler_t, ("sampler_t", AtomicString::ConstructFromLiteral));
        if (typeName == sampler_t)
            return WebCLKernelTypes::SAMPLER;

        DEFINE_STATIC_LOCAL(AtomicString, ucharLiteral, ("uchar", AtomicString::ConstructFromLiteral));
        if (typeName.contains(ucharLiteral))
            return WebCLKernelTypes::UCHAR;
        DEFINE_STATIC_LOCAL(AtomicString, charLiteral, ("char", AtomicString::ConstructFromLiteral));
        if (typeName.contains(charLiteral))
            return WebCLKernelTypes::CHAR;

        DEFINE_STATIC_LOCAL(AtomicString, ushortLiteral, ("ushort", AtomicString::ConstructFromLiteral));
        if (typeName.contains(ushortLiteral))
            return WebCLKernelTypes::USHORT;
        DEFINE_STATIC_LOCAL(AtomicString, shortLiteral, ("short", AtomicString::ConstructFromLiteral));
        if (typeName.contains(shortLiteral))
            return WebCLKernelTypes::SHORT;

        DEFINE_STATIC_LOCAL(AtomicString, uintLiteral, ("uint", AtomicString::ConstructFromLiteral));
        if (typeName.contains(uintLiteral))
            return WebCLKernelTypes::UINT;
        DEFINE_STATIC_LOCAL(AtomicString, intLiteral, ("int", AtomicString::ConstructFromLiteral));
        if (typeName.contains(intLiteral))
            return WebCLKernelTypes::INT;

        DEFINE_STATIC_LOCAL(AtomicString, ulongLiteral, ("ulong", AtomicString::ConstructFromLiteral));
        if (typeName.contains(ulongLiteral))
            return WebCLKernelTypes::ULONG;
        DEFINE_STATIC_LOCAL(AtomicString, longLiteral, ("long", AtomicString::ConstructFromLiteral));
        if (typeName.contains(longLiteral))
            return WebCLKernelTypes::LONG;

        DEFINE_STATIC_LOCAL(AtomicString, floatLiteral, ("float", AtomicString::ConstructFromLiteral));
        if (typeName.contains(floatLiteral))
            return WebCLKernelTypes::FLOAT;

        return WebCLKernelTypes::UNKNOWN;
    }

};
} // namespace WebCore

#endif
#endif // WebCLKernelArgInfo_h
