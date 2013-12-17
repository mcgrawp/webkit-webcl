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

#ifndef WebCLGetInfo_h
#define WebCLGetInfo_h

#if ENABLE(WEBCL)

#include <wtf/Float32Array.h>
#include <wtf/Int32Array.h>
#include <wtf/Int8Array.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefPtr.h>
#include <wtf/Uint8Array.h>
#include <wtf/text/WTFString.h>

namespace WebCore {

class WebCLCommandQueue;
class WebCLContext;
class WebCLContextProperties;
class WebCLDevice;
class WebCLImageDescriptor;
class WebCLMemoryObject;
class WebCLPlatform;
class WebCLProgram;

// A tagged union representing the result of get queries like
// getParameter (encompassing getBooleanv, getIntegerv, getFloatv) and
// similar variants. For reference counted types, increments and
// decrements the reference count of the target object.

class WebCLGetInfo {
public:
    enum Type {
        kTypeBool,
        kTypeBoolArray,
        kTypeFloat,
        kTypeInt,
        kTypeNull,
        kTypeString,
        kTypeUnsignedInt,
        kTypeUnsignedLong,
        kTypeVoidPointer,
        kTypeWebCLFloatArray,
        kTypeWebCLImageDescriptor,
        kTypeWebCLIntArray,
        kTypeWebCLInt8Array,
        kTypeWebCLObjectArray,
        kTypeWebCLProgram,
        kTypeWebCLContext,
        kTypeWebCLCommandQueue,
        kTypeWebCLDevice,
        kTypeWebCLDevices,
        kTypeWebCLMemoryObject,
        kTypeWebCLPlatform,
        kTypeWebCLContextProperties
    };

    WebCLGetInfo();
    WebCLGetInfo(const bool* value, int size);
    explicit WebCLGetInfo(bool value);
    explicit WebCLGetInfo(float value);
    explicit WebCLGetInfo(int value);
    explicit WebCLGetInfo(const String& value);
    explicit WebCLGetInfo(unsigned int value);
    explicit WebCLGetInfo(unsigned long value);
    explicit WebCLGetInfo(void* value);
    explicit WebCLGetInfo(const Vector<RefPtr<WebCLDevice> >& value);
    explicit WebCLGetInfo(PassRefPtr<Float32Array> value);
    explicit WebCLGetInfo(PassRefPtr<Int32Array> value);
    explicit WebCLGetInfo(PassRefPtr<Int8Array> value);
    explicit WebCLGetInfo(WebCLImageDescriptor* value);
    explicit WebCLGetInfo(WebCLProgram* value);
    explicit WebCLGetInfo(WebCLContext* value);
    explicit WebCLGetInfo(WebCLCommandQueue* value);
    explicit WebCLGetInfo(WebCLDevice* value);
    explicit WebCLGetInfo(WebCLMemoryObject* value);
    explicit WebCLGetInfo(WebCLPlatform* value);
    explicit WebCLGetInfo(WebCLContextProperties* value);

    virtual ~WebCLGetInfo();
    Type getType() const;
    bool getBool() const;
    const Vector<bool>& getBoolArray() const;
    float getFloat() const;
    int getInt() const;
    const String& getString() const;
    unsigned int getUnsignedInt() const;
    unsigned long getUnsignedLong() const;
    void* getVoidPointer() const;
    PassRefPtr<Float32Array> getWebCLFloatArray() const;
    PassRefPtr<WebCLImageDescriptor> getWebCLImageDescriptor() const;
    PassRefPtr<Int32Array> getWebCLIntArray() const;
    PassRefPtr<Int8Array> getWebCLInt8Array() const;
    PassRefPtr<WebCLProgram> getWebCLProgram() const;
    PassRefPtr<WebCLContext> getWebCLContext() const;
    PassRefPtr<WebCLCommandQueue> getWebCLCommandQueue() const;
    PassRefPtr<WebCLDevice> getWebCLDevice() const;
    const Vector<RefPtr<WebCLDevice> >& getWebCLDevices() const;
    PassRefPtr<WebCLMemoryObject> getWebCLMemoryObject() const;
    PassRefPtr<WebCLPlatform> getWebCLPlatform() const;
    PassRefPtr<WebCLContextProperties> getWebCLContextProperties() const;

private:
    Type m_type;
    bool m_bool;
    Vector<bool> m_boolArray;
    float m_float;
    int m_int;
    String m_string;
    unsigned int m_unsignedInt;
    unsigned long m_unsignedLong;
    void* m_voidPointer;
    RefPtr<Float32Array> m_webclFloatArray;
    RefPtr<WebCLImageDescriptor> m_webclImageDescriptor;
    RefPtr<Int32Array> m_webclIntArray;
    RefPtr<Int8Array> m_webclInt8Array;
    RefPtr<WebCLProgram> m_webclProgram;
    RefPtr<WebCLContext> m_webclContext;
    RefPtr<WebCLCommandQueue> m_webCLCommandQueue;
    RefPtr<WebCLDevice> m_webCLDevice;
    Vector<RefPtr<WebCLDevice> > m_webCLDevices;
    RefPtr<WebCLMemoryObject> m_webCLMemoryObject;
    RefPtr<WebCLPlatform> m_webCLPlatform;
    RefPtr<WebCLContextProperties> m_webCLContextProperties;
};

} // namespace WebCore

#endif // ENABLE(WEBCL)
#endif // WebCLGetInfo_h
