/*
 * Copyright (C) 2013 Samsung Electronics. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ExtensionsCL.h"

namespace WebCore {

ExtensionsCL& ExtensionsCL::get()
{
    DEFINE_STATIC_LOCAL(ExtensionsCL, singleton, ());
    return singleton;
}

ExtensionsCL::ExtensionsCL()
    : m_initializedGlobalExtensions(false)
{
}

ExtensionsCL::~ExtensionsCL()
{
}

bool ExtensionsCL::supports(const String& name)
{
    if (!m_initializedGlobalExtensions)
        initializeGlobalExtensions();

    return supportsExtension(name);
}

bool ExtensionsCL::supports(const String& name, CCPlatformID platformID)
{
    if (!m_platformExtensions.contains(platformID)) {
        if (!cacheExtensionsForPlatform(platformID))
            return false;
    }

    return supportsExtension(name, platformID);
}

bool ExtensionsCL::supports(const String& name, CCDeviceID deviceID)
{
    if (!m_deviceExtensions.contains(deviceID)) {
        if (!cacheExtensionsForDevice(deviceID))
            return false;
    }

    return supportsExtension(name, deviceID);
}

void ExtensionsCL::initializeGlobalExtensions()
{
    if (m_initializedGlobalExtensions)
        return;

    cacheGlobalExtensions();
    m_initializedGlobalExtensions = true;
}

bool ExtensionsCL::supportsExtension(const WTF::String& name)
{
    if (name == "cl_khr_gl_sharing") {
#if PLATFORM(MAC)
        return m_globalExtensions.contains("cl_APPLE_gl_sharing");
#else
        // FIXME: implement support for other platforms.
        return false;
#endif
    }
    return m_globalExtensions.contains(name);
}

bool ExtensionsCL::supportsExtension(const WTF::String& name, CCPlatformID platformID)
{
    if (name == "cl_khr_gl_sharing") {
#if PLATFORM(MAC)
        return m_platformExtensions.get(platformID).contains("cl_APPLE_gl_sharing");
#else
        // FIXME: implement support for other platforms.
        return false;
#endif
    }
    return m_platformExtensions.get(platformID).contains(name);
}

bool ExtensionsCL::supportsExtension(const WTF::String& name, CCDeviceID deviceID)
{
    if (name == "cl_khr_gl_sharing") {
#if PLATFORM(MAC)
        return m_deviceExtensions.get(deviceID).contains("cl_APPLE_gl_sharing");
#else
        // FIXME: implement support for other devices.
        return false;
#endif
    }
    return m_deviceExtensions.get(deviceID).contains(name);
}

void ExtensionsCL::cacheGlobalExtensions()
{
    Vector<CCPlatformID> ccPlatforms;
    CCerror error = ComputeContext::getPlatformIDs(ccPlatforms);
    if (error != ComputeContext::SUCCESS)
        return;

    if (!m_platformExtensions.contains(ccPlatforms[0]))
        cacheExtensionsForPlatform(ccPlatforms[0]);
    m_globalExtensions = m_platformExtensions.get(ccPlatforms[0]);

    if (ccPlatforms.size() == 1) {
        // Fast path: if there is only one platform,
        // "global extensions" is equal to this platform's
        // extension.
        return;
    }
    ASSERT(ccPlatforms.size() > 1);

    for (size_t i = 1; i < ccPlatforms.size(); ++i) {
        if (!m_platformExtensions.contains(ccPlatforms[i]))
            cacheExtensionsForPlatform(ccPlatforms[i]);
        HashSet<String> temporary = m_platformExtensions.get(ccPlatforms[i]);

        Vector<String> toBeRemoved;
        HashSet<String>::iterator end = m_globalExtensions.end();
        for (HashSet<String>::iterator it = m_globalExtensions.begin(); it != end; ++it) {
            if (temporary.contains(*it))
                continue;

            toBeRemoved.append(*it);
        }
        for (size_t j = 0; j < toBeRemoved.size(); ++j)
            m_globalExtensions.remove(toBeRemoved[j]);
    }
}

} // namespace WebCore
