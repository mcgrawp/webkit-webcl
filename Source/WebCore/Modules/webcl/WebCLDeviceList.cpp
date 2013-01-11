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

#include "WebCL.h"
#include "WebCLContext.h"
#include "WebCLDeviceList.h"
#include "WebCLDevice.h"

namespace WebCore {

WebCLDeviceList::~WebCLDeviceList()
{
}

PassRefPtr<WebCLDeviceList> WebCLDeviceList::create(CCPlatformID platformId, CCenum deviceType, CCerror& error)
{
    CCint devicesSize = ComputeContext::deviceIDs(platformId, deviceType, 0, 0, error);
    if (error != ComputeContext::SUCCESS)
        return 0;

    Vector<CCDeviceID> devices(devicesSize);
    ComputeContext::deviceIDs(platformId, deviceType, devicesSize, devices.data(), error);
    if (error != ComputeContext::SUCCESS)
        return 0;

    return adoptRef(new WebCLDeviceList(devices));
}

PassRefPtr<WebCLDeviceList> WebCLDeviceList::create(const Vector<CCDeviceID>& devices)
{
    return adoptRef(new WebCLDeviceList(devices));
}

WebCLDeviceList::WebCLDeviceList(const Vector<CCDeviceID>& devices)
    : m_clDevices(devices)
{
    for (size_t i = 0; i < devices.size(); ++i) {
        RefPtr<WebCLDevice> webCLDevice = WebCLDevice::create(devices[i]);
        if (!webCLDevice)
            m_devices.append(webCLDevice);
    }
}

CCDeviceID* WebCLDeviceList::getCLDevices()
{
    return m_clDevices.data();
}

unsigned WebCLDeviceList::length() const
{
    return m_devices.size();
}

WebCLDevice* WebCLDeviceList::item(unsigned index) const
{
    if (index >= m_devices.size())
        return 0;

    return m_devices[index].get();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
