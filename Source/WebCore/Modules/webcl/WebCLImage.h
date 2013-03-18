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

#ifndef WebCLImage_h
#define WebCLImage_h

#include "WebCLException.h"
#include "WebCLImageDescriptor.h"
#include "WebCLMemoryObject.h"

#include <OpenCL/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class WebCL;

class WebCLImage : public WebCLMemoryObject {
public:
    ~WebCLImage();
    static PassRefPtr<WebCLImage> create(WebCLContext*, int, int, int, const CCImageFormat&, void*, ExceptionCode&);
    //FIXME: Remove this method when WebCL WebGL interop become an extension
    static PassRefPtr<WebCLImage> create() { return adoptRef(new WebCLImage()); }

    PlatformComputeObject getCLImage();
    PassRefPtr<WebCLImageDescriptor> getInfo(ExceptionCode&);
    int getGLtextureInfo(int, ExceptionCode&);

private:
    WebCLImage(WebCLContext*, PlatformComputeObject image, int width, int height, const CCImageFormat& format);
    WebCLImage()
        : WebCLMemoryObject(0, 0, false)
        { }

    int m_width;
    int m_height;
    CCImageFormat m_format;
};

} // namespace WebCore

#endif // WebCLImage_h
