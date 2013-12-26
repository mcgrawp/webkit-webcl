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

#include "config.h"

#if ENABLE(WEBCL)

#include "WebCLKernelArgInfoProvider.h"

#include "WebCLKernel.h"
#include "WebCLProgram.h"

namespace WebCore {

static bool isASCIILineBreakOrStarCharacter(UChar c)
{
    return c == '\r' || c == '\n' || c == '*';
}

inline bool isEmptySpace(UChar character)
{
    return character <= ' ' && (character == ' ' || character == '\n' || character == '\t' || character == '\r' || character == '\f');
}

inline bool isPrecededByUnderscores(const String& string, size_t index)
{
     return string[index - 1] == '_' && string[index - 2] == '_';
}

WebCLKernelArgInfoProvider::WebCLKernelArgInfoProvider(WebCLKernel* kernel)
    : m_kernel(kernel)
{
    ASSERT(kernel);
}

const Vector<RefPtr<WebCLKernelArgInfo> >& WebCLKernelArgInfoProvider::argumentsInfo()
{
    ensureInfo();
    return m_argumentInfoVector;
}

void WebCLKernelArgInfoProvider::ensureInfo()
{
    if (m_argumentInfoVector.size())
        return;

    const String& source = m_kernel->program()->sourceWithCommentsStripped();

    // 0) find "kernel" string.
    // 1) Check if it is a valid kernel declaration.
    // 2) find the first open braces past "kernel".
    // 3) reverseFind the given kernel name string.
    // 4) if not found go back to (1)
    // 5) if found, parse its argument list.

    size_t kernelNameIndex = 0;
    size_t kernelDeclarationIndex = 0;
    for (size_t startIndex = 0; ; startIndex = kernelDeclarationIndex + 6) {

        kernelDeclarationIndex = source.find("kernel", startIndex);
        if (kernelDeclarationIndex == WTF::notFound)
            CRASH();

        // Check if "kernel" is not a substring of a valid token,
        // e.g. "akernel" or "__kernel_":
        // 1) After "kernel" there has to be an empty space.
        // 2) Before "kernel" there has to be either:
        // 2.1) two underscore characters or
        // 2.2) none, i.e. "kernel" is the first string in the program source or
        // 2.3) an empty space.
        if (!isEmptySpace(source[kernelDeclarationIndex + 6]))
            continue;

        // if index is not 0.
        if (kernelDeclarationIndex) {
             ASSERT(kernelDeclarationIndex >= 2);
             if (!(isPrecededByUnderscores(source, kernelDeclarationIndex) || isEmptySpace(source[kernelDeclarationIndex - 1])))
                 continue;
        }

        size_t openBrace = source.find("{", kernelDeclarationIndex + 6);
        kernelNameIndex = source.reverseFind(m_kernel->kernelName(), openBrace);

        ASSERT(kernelNameIndex > kernelDeclarationIndex);
        if (kernelNameIndex != WTF::notFound)
            break;
    }

    ASSERT(kernelNameIndex);
    size_t openBraket = source.find("(", kernelNameIndex);
    size_t closeBraket = source.find(")", openBraket);
    String argumentListStr = source.substring(openBraket + 1, closeBraket - openBraket - 1);

    Vector<String> argumentStrVector;
    argumentListStr.split(",", argumentStrVector);
    for (size_t i = 0; i < argumentStrVector.size(); ++i) {
        argumentStrVector[i] = argumentStrVector[i].removeCharacters(isASCIILineBreakOrStarCharacter);
        argumentStrVector[i] = argumentStrVector[i].stripWhiteSpace();
        parseAndAppendDeclaration(argumentStrVector[i]);
    }
}

void WebCLKernelArgInfoProvider::parseAndAppendDeclaration(const String& argumentDeclaration)
{
    Vector<String> declarationStrVector;
    argumentDeclaration.split(" ", declarationStrVector);

    String name = extractName(declarationStrVector);
    String type = extractType(declarationStrVector);
    String addressQualifier = extractAddressQualifier(declarationStrVector);

    DEFINE_STATIC_LOCAL(AtomicString, image2d_t, ("image2d_t", AtomicString::ConstructFromLiteral));
    String accessQualifier = (type == image2d_t) ? extractAccessQualifier(declarationStrVector) : "none";

    m_argumentInfoVector.append(adoptRef(new WebCLKernelArgInfo(addressQualifier, accessQualifier, type, name)));
}

// FIXME: As of now, it returns the qualifier as declared: e.g., if "__private" is declared,
// it returns "__private", while it is declared as "private", it returns "private".
// See http://www.khronos.org/bugzilla/show_bug.cgi?id=1079
String WebCLKernelArgInfoProvider::extractAddressQualifier(Vector<String>& declarationStrVector)
{
    DEFINE_STATIC_LOCAL(AtomicString, __Private, ("__private", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(AtomicString, Private, ("private", AtomicString::ConstructFromLiteral));

    DEFINE_STATIC_LOCAL(AtomicString, __Global, ("__global", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(AtomicString, Global, ("global", AtomicString::ConstructFromLiteral));

    DEFINE_STATIC_LOCAL(AtomicString, __Constant, ("__constant", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(AtomicString, Constant, ("constant", AtomicString::ConstructFromLiteral));

    DEFINE_STATIC_LOCAL(AtomicString, __Local, ("__local", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(AtomicString, Local, ("local", AtomicString::ConstructFromLiteral));

    String addressQualifier = __Private;
    size_t i = 0;
    for ( ; i < declarationStrVector.size(); ++i) {
        String candidate = declarationStrVector[i];
        if (candidate == __Private  || candidate == Private
         || candidate == __Global   || candidate == Global
         || candidate == __Constant || candidate == Constant
         || candidate == __Local    || candidate == Local) {
            addressQualifier = candidate;
            break;
        }
    }

    if (i < declarationStrVector.size())
        declarationStrVector.remove(i);

    return addressQualifier;
}

// FIXME: As of now, it returns the qualifier as declared: e.g., if "__read_only" is declared,
// it returns "__read_only", while it is declared as "read_only", "read_only" is returned.
// See http://www.khronos.org/bugzilla/show_bug.cgi?id=1079
String WebCLKernelArgInfoProvider::extractAccessQualifier(Vector<String>& declarationStrVector)
{
    DEFINE_STATIC_LOCAL(AtomicString, __read_only, ("__read_only", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(AtomicString, read_only, ("read_only", AtomicString::ConstructFromLiteral));

    DEFINE_STATIC_LOCAL(AtomicString, __write_only, ("__read_only", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(AtomicString, write_only, ("write_only", AtomicString::ConstructFromLiteral));

    DEFINE_STATIC_LOCAL(AtomicString, __read_write, ("__read_write", AtomicString::ConstructFromLiteral));
    DEFINE_STATIC_LOCAL(AtomicString, read_write, ("read_write", AtomicString::ConstructFromLiteral));

    String accessQualifier = __read_only;
    size_t i = 0;
    for ( ; i < declarationStrVector.size(); ++i) {
        String candidate = declarationStrVector[i];
        if (candidate == __read_only  || candidate == read_only
         || candidate == __write_only || candidate == write_only
         || candidate == __read_write || candidate == read_write) {
            accessQualifier = candidate;
            break;
        }
    }

    if (i < declarationStrVector.size())
        declarationStrVector.remove(i);

    return accessQualifier;
}

String WebCLKernelArgInfoProvider::extractName(Vector<String>& declarationStrVector)
{
    String last = declarationStrVector.last();
    declarationStrVector.removeLast();
    return last;
}

// FIXME: Support collapsing "unsigned <scalar>" into "u<scalar>", as per OpenCL 1.2 spec.
String WebCLKernelArgInfoProvider::extractType(Vector<String>& declarationStrVector)
{
    String last = declarationStrVector.last();
    declarationStrVector.removeLast();
    return last;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
