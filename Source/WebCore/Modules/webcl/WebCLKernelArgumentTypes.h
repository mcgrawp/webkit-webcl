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

#ifndef WebCLKernelArgumentTypes_h
#define WebCLKernelArgumentTypes_h

#if ENABLE(WEBCL)

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

using namespace std;

namespace WebCore {

class WebCLKernelTypeObject;
class WebCLKernelTypeValue;

class WebCLKernelArgumentTypes : public RefCounted<WebCLKernelArgumentTypes> {
public:
    static PassRefPtr<WebCLKernelArgumentTypes> create();
    virtual ~WebCLKernelArgumentTypes();
    enum {
        // Scalar types; may be bitwise-OR'ed with a vector type
        CHAR   = 0,
        UCHAR  = 1,
        SHORT  = 2,
        USHORT = 3,
        INT    = 4,
        UINT   = 5,
        LONG   = 6,
        ULONG  = 7,
        FLOAT  = 8,
        HALF   = 9, // not supported in all implementations
        DOUBLE = 10, // not supported in all implementations

        // Vector types; must be bitwise-OR'ed with a scalar type
        VEC2  = 0x0200,
        VEC3  = 0x0300,
        VEC4  = 0x0400,
        VEC8  = 0x0800,
        VEC16 = 0x1000,

        // Special types must not be bitwise-OR'ed with any other type.
        LOCAL_MEMORY_SIZE = 255,

        ARG_MEMORY_OBJECT   = 256,
        KERNEL_NULL_TYPE = 0xFFFF,
    };

private:
    WebCLKernelArgumentTypes();
};
} // namespace WebCore

#endif // ENABLE(WEBCL)

#endif // WebCL_h
