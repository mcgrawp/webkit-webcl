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

#include "config.h"

#if ENABLE(WEBCL)

#include "WebCLPlatformList.h"
#include "WebCL.h"

namespace WebCore {

WebCLPlatformList::~WebCLPlatformList()
{
}

PassRefPtr<WebCLPlatformList> WebCLPlatformList::create(CCerror &error)
{
    CCint numberOfPlatforms = ComputeContext::platformIDs(0, 0, error);
    if (error != ComputeContext::SUCCESS)
        return 0;

    Vector<CCPlatformID> platformIDs(numberOfPlatforms);
    ComputeContext::platformIDs(numberOfPlatforms, platformIDs.data(), error);
    if (error != ComputeContext::SUCCESS)
        return 0;

    return adoptRef(new WebCLPlatformList(platformIDs));
}

WebCLPlatformList::WebCLPlatformList(const Vector<CCPlatformID>& platformIDs)
{
    for (size_t i = 0 ; i < platformIDs.size(); ++i) {
        RefPtr<WebCLPlatform> webCLPlatform = WebCLPlatform::create(platformIDs[i]);
        if (webCLPlatform)
            m_platformIDs.append(webCLPlatform);
    }
}

unsigned WebCLPlatformList::length() const
{
    return m_platformIDs.size();
}

WebCLPlatform* WebCLPlatformList::item(unsigned index) const
{
    if (index >= m_platformIDs.size())
        return 0;

    return m_platformIDs[index].get();
}


} // namespace WebCore

#endif // ENABLE(WEBCL)
