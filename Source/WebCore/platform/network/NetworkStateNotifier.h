/*
 * Copyright (C) 2008 Apple Inc. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef NetworkStateNotifier_h
#define NetworkStateNotifier_h

#include <functional>
#include <wtf/FastMalloc.h>
#include <wtf/Noncopyable.h>
#include <wtf/Vector.h>

#if PLATFORM(MAC) && !PLATFORM(IOS)

#include <wtf/RetainPtr.h>
#include "Timer.h"

typedef const struct __CFArray * CFArrayRef;
typedef const struct __SCDynamicStore * SCDynamicStoreRef;

#elif PLATFORM(WIN)

#include <windows.h>

#elif PLATFORM(EFL)

typedef struct _Ecore_Fd_Handler Ecore_Fd_Handler;
typedef unsigned char Eina_Bool;

#endif

namespace WebCore {

class NetworkStateNotifier {
    WTF_MAKE_NONCOPYABLE(NetworkStateNotifier); WTF_MAKE_FAST_ALLOCATED;
public:
    NetworkStateNotifier();
#if PLATFORM(EFL)
    ~NetworkStateNotifier();
#endif
    void addNetworkStateChangeListener(std::function<void (bool isOnLine)>);

    bool onLine() const { return m_isOnLine; }
    
#if PLATFORM(IOS)
    void setIsOnLine(bool);
#endif

private:
    bool m_isOnLine;
    Vector<std::function<void (bool)>> m_listeners;

    void notifyNetworkStateChange();
    void updateState();

#if PLATFORM(MAC) && !PLATFORM(IOS)
    void networkStateChangeTimerFired(Timer<NetworkStateNotifier>&);

    static void dynamicStoreCallback(SCDynamicStoreRef, CFArrayRef changedKeys, void *info); 

    RetainPtr<SCDynamicStoreRef> m_store;
    Timer<NetworkStateNotifier> m_networkStateChangeTimer;

#elif PLATFORM(WIN)
    static void CALLBACK addrChangeCallback(void*, BOOLEAN timedOut);
    static void callAddressChanged(void*);
    void addressChanged();

    void registerForAddressChange();
    HANDLE m_waitHandle;
    OVERLAPPED m_overlapped;

#elif PLATFORM(EFL)
    void networkInterfaceChanged();
    static Eina_Bool readSocketCallback(void* userData, Ecore_Fd_Handler*);

    int m_netlinkSocket;
    Ecore_Fd_Handler* m_fdHandler;
#endif
};

#if !PLATFORM(COCOA) && !PLATFORM(WIN) && !PLATFORM(EFL)

inline NetworkStateNotifier::NetworkStateNotifier()
    : m_isOnLine(true)
{
}

inline void NetworkStateNotifier::updateState() { }

#endif

NetworkStateNotifier& networkStateNotifier();

};

#endif // NetworkStateNotifier_h
