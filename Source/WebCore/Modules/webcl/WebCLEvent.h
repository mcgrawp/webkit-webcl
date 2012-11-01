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

#ifndef WebCLEvent_h
#define WebCLEvent_h

#include <OpenCL/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>

#include "WebCLFinishCallback.h"
#include "WebCLGetInfo.h"

namespace WebCore {

class WebCL;
class WebCLEvent;
class WebCLEvent : public RefCounted<WebCLEvent> {
public:
    virtual ~WebCLEvent();
    static PassRefPtr<WebCLEvent> create(WebCLContext*, cl_event);
    WebCLGetInfo getInfo(int, ExceptionCode&);
    WebCLGetInfo getProfilingInfo(int, ExceptionCode&);
    void setCallback(int, PassRefPtr<WebCLFinishCallback>, int , ExceptionCode&);
    void setUserEventStatus (int, ExceptionCode&);
    cl_event getCLEvent();
    RefPtr<WebCLFinishCallback> m_finishCallback;
    WebCLContext* getContext();
    int b;

private:
    WebCLEvent(WebCLContext*, cl_event);
    //void CL_CALLBACK execComplete(cl_event ev, cl_int event_status, void* user_data) const;
    //static void CL_CALLBACK execComplete(cl_event ev, cl_int event_status, void* this_pointer);
    static void execComplete(cl_event ev, cl_int event_status, void* this_pointer);
    static void callme(void* this_poonter);
    WebCLContext* m_context;
    Vector<RefPtr<WebCLEvent> > m_event_list;
    cl_event m_cl_Event;
    long m_num_events;
    static WebCLEvent *this_pointer;
};

} // namespace WebCore

#endif // WebCLEvent_h

