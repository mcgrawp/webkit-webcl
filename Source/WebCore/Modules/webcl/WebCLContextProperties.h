/*
 * Copyright (C) 2011, 2012, 2013 Samsung Electronics Corporation. All rights reserved.
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

#ifndef WebCLContextProperties_h
#define WebCLContextProperties_h

#include "WebCLException.h"
#include "WebCLPlatform.h"

#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

#include <OpenCL/opencl.h>

using namespace std;

namespace WebCore {

class WebCL;
class WebCLDevice;
class WebCLPlatform;

class WebCLContextProperties : public RefCounted<WebCLContextProperties>
{
public:
    static PassRefPtr<WebCLContextProperties> create(PassRefPtr<WebCLPlatform>, const Vector<RefPtr<WebCLDevice> >&, int, int, const String&);
    ~WebCLContextProperties() { }


    Vector<RefPtr<WebCLDevice> > devices() const;

    RefPtr<WebCLPlatform>& platform();
    void setPlatform(PassRefPtr<WebCLPlatform> platform);

    int deviceType() const;
    void setDeviceType(int type);

    int shareGroup() const;
    void setShareGroup(int);

    String hint() const;
    void setHint(const String&);

    Vector<CCContextProperties>& computeContextProperties();

private:
    WebCLContextProperties(PassRefPtr<WebCLPlatform>, const Vector<RefPtr<WebCLDevice> >&, int, int, const String&);

    RefPtr<WebCLPlatform> objplatform;
    Vector<RefPtr<WebCLDevice> > objdevices;
    int objdeviceType;
    int objshareGroup;
    Vector<CCContextProperties> m_ccProperties;
    // FIXME: spec says DOMString[].
    String objhint;
};

}// namespace WebCore

#endif // WebCLContextProperties_h
