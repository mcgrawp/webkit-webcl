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

#ifndef WebCLContextProperties_h
#define WebCLContextProperties_h

#include <OpenCL/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>
#include "WebCLException.h"
#include "WebCLDevice.h"
#include "WebCLPlatform.h"
#include <wtf/text/WTFString.h>

using namespace std ;

namespace WebCore{

class WebCL;

class WebCLContextProperties : public RefCounted<WebCLContextProperties>
{
public:
    ~WebCLContextProperties();
    static PassRefPtr<WebCLContextProperties> create(WebCLPlatform*, const Vector<RefPtr<WebCLDevice> >&, int, int, const String&);

    WebCLPlatform* platform() const;
    void setPlatform(WebCLPlatform*);

    Vector<RefPtr<WebCLDevice> > devices() const;

    int deviceType() const;
    void setDeviceType(int type);

    int shareGroup() const;
    void setShareGroup(int);

    String hint() const;
    void setHint(const String&);

private:
    WebCLContextProperties(WebCLPlatform*, const Vector<RefPtr<WebCLDevice> >&, int, int, const String&);

    WebCLPlatform* objplatform;
    Vector<RefPtr<WebCLDevice> > objdevices;
    int objdeviceType;
    int objshareGroup;
    String objhint;
};

}// namespace WebCore

#endif // WebCLContextProperties_h

