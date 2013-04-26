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

#ifndef ExtensionsCL_h
#define ExtensionsCL_h

#include "ComputeContext.h"
#include "ComputeTypes.h"
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/text/StringHash.h>

namespace WebCore {

class ExtensionsCL;
typedef ExtensionsCL ComputeExtensions;

class ExtensionsCL {
public:
    static ExtensionsCL& get();

    virtual ~ExtensionsCL();

    // Takes full name of extension; for example "cl_khr_gl_sharing".
    bool supports(const String&);
    bool supports(const String&, CCPlatformID);
    bool supports(const String&, CCDeviceID);

private:
    ExtensionsCL();

    // Global extensions.
    bool supportsExtension(const WTF::String& name);
    void cacheGlobalExtensions();
    void initializeGlobalExtensions();
    bool m_initializedGlobalExtensions;
    HashSet<String> m_globalExtensions;

    // Platform extensions.
    bool supportsExtension(const WTF::String& name, CCPlatformID);
    bool cacheExtensionsForPlatform(CCPlatformID platformID)
    {
        return cacheExtensionsHelper(ComputeContext::getPlatformInfo<Vector<char> >, platformID, ComputeContext::PLATFORM_EXTENSIONS, m_platformExtensions); 
    }
    HashMap<CCPlatformID, HashSet<String> > m_platformExtensions;

    // Device extensions.
    bool supportsExtension(const WTF::String& name, CCDeviceID);
    bool cacheExtensionsForDevice(CCDeviceID deviceID)
    {
        return cacheExtensionsHelper(ComputeContext::getDeviceInfo<Vector<char> >, deviceID, ComputeContext::DEVICE_EXTENSIONS, m_deviceExtensions); 
    }
    HashMap<CCDeviceID, HashSet<String> > m_deviceExtensions;


    template <typename Func, typename Type>
    struct Functor {
        Func function;
        CCerror operator()(Type computeType, int property, Vector<char>* buffer)
        {
            return function(computeType, property, buffer);
        }
    };

    template <typename Func, typename Type, typename Cache>
    struct CacheExtensions
    {
        static bool perform(Func getExtensionsFunc, Type computeType, int property, Cache& cache)
        {
            Functor<Func, Type> func = {getExtensionsFunc};

            Vector<char> buffer;
            CCerror error = func(computeType, property , &buffer);
            if (error != ComputeContext::SUCCESS)
                return false;

            HashSet<String> extensionSet;
            if (buffer.isEmpty()) {
                cache.set(computeType, extensionSet);
                return true;
            }

            Vector<String> extensionArray;
            String(buffer.data()).split(" ", extensionArray);

            for (size_t i = 0; i < extensionArray.size(); ++i)
                extensionSet.add(extensionArray[i]);
            cache.set(computeType, extensionSet);
            return true;
        }
    };

    template <typename Func, typename Type, typename Cache>
    bool cacheExtensionsHelper(Func func, Type computeType, int property, Cache& cache)
    {
        return CacheExtensions<Func, Type, Cache>::perform(func, computeType, property, cache);
    }

};
} // namespace WebCore

#endif // ExtensionsCL_h
