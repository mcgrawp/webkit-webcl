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

#include "WebCLEvent.h"
#include "WebCL.h"
#include "WebCLException.h"


namespace WebCore {
WebCLEvent::~WebCLEvent()
{
    cl_int err = 0;
    ASSERT(m_cl_Event);

    err = clReleaseEvent(m_cl_Event);
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_EVENT  :
                printf("Error: CL_INVALID_EVENT\n");
                break;
            case CL_OUT_OF_RESOURCES  :
                printf("Error: CL_OUT_OF_RESOURCES\n");
                break;
            case CL_OUT_OF_HOST_MEMORY  :
                printf("Error: CL_OUT_OF_HOST_MEMORY\n");
                break;
            default:
                printf("Error: Invaild Error Type\n");
                break;
        }
    } else {
        for (int i = 0; i < m_num_events; i++) {
            if ((m_event_list[i].get())->getCLEvent() == m_cl_Event) {
                m_event_list.remove(i);
                m_num_events = m_event_list.size();
                break;
            }
        }
    }
}

PassRefPtr<WebCLEvent> WebCLEvent::create(WebCLContext* context, cl_event Event)
{
    return adoptRef(new WebCLEvent(context, Event));
}

WebCLEvent::WebCLEvent(WebCLContext* context, cl_event Event)
    : m_context(context),
      m_cl_Event(Event)
{
    m_num_events = 0;
    b =5;
}


WebCLContext* WebCLEvent::getContext()
{
    return m_context;
}

WebCLGetInfo WebCLEvent::getInfo(int param_name, ExceptionCode& ec)
{
    cl_int err = 0;
    cl_uint uint_units = 0;
    cl_command_type command_type = 0;
    cl_command_queue command_queue = 0;
    //cl_context my_context = 0;
    RefPtr<WebCLCommandQueue> cqObj = NULL;
    //char my_context[1024];
    if (m_cl_Event == NULL) {
        printf("Error: Invalid CL Event\n");
        ec = WebCLException::INVALID_EVENT;
        return WebCLGetInfo();
    }


    switch(param_name) {
        case ComputeContext::EVENT_COMMAND_EXECUTION_STATUS:
            err = clGetEventInfo(m_cl_Event, param_name, sizeof(cl_uint), &uint_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(uint_units));	
            break;
        case ComputeContext::EVENT_COMMAND_TYPE:
            err=clGetEventInfo(m_cl_Event, param_name, sizeof(cl_command_type), &command_type, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(command_type));
            break;
        case ComputeContext::EVENT_COMMAND_QUEUE:
            err = clGetEventInfo(m_cl_Event, param_name, sizeof(cl_command_queue), &command_queue, NULL);
            cqObj = WebCLCommandQueue::create(m_context, command_queue);
            if(cqObj == NULL)
            {
                printf("SUCCESS: Cl Event Command Queue\n");
                return WebCLGetInfo();
            }
            if (err == CL_SUCCESS)
                return WebCLGetInfo(PassRefPtr<WebCLCommandQueue>(cqObj));
            break;
            /*
            case 4564://ComputeContext::EVENT_CONTEXT:
                err = clGetEventInfo(m_cl_Event, param_name, sizeof(my_context), my_context, NULL);;
                printf("After event_context call ");
                if (err == CL_SUCCESS)
                    return WebCLGetInfo((String)my_context);
                break;
            */
        default:
            printf("Error: Unsupported Event Info type\n");
            ec = WebCLException::FAILURE;
            return WebCLGetInfo();
    }

    switch (err) {
        case CL_INVALID_VALUE:
            ec = WebCLException::INVALID_VALUE;
            printf("Error: CL_INVALID_VALUE   \n");
            break;
        case CL_INVALID_EVENT:
            ec = WebCLException::INVALID_EVENT;
            printf("Error: CL_INVALID_EVENT   \n");
            break; 
        case CL_OUT_OF_RESOURCES:
            ec = WebCLException::OUT_OF_RESOURCES;
            printf("Error: CL_OUT_OF_RESOURCES   \n");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            ec = WebCLException::OUT_OF_HOST_MEMORY;
            printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
            break;
        default:
            printf("Error: Invaild Error Type\n");
            ec = WebCLException::FAILURE;
            break;
    }	

    printf("444444444getInfo Called = %d\n", param_name); 
    return WebCLGetInfo();

}

WebCLGetInfo WebCLEvent::getProfilingInfo(int param_name, ExceptionCode& ec)
{
    cl_int err=0;
    cl_ulong  ulong_units = 0;

    if (m_cl_Event == NULL) {
        printf("Error: Invalid CL Event\n");
        ec = WebCLException::INVALID_EVENT;
        return WebCLGetInfo();
    }
    switch(param_name)
    {
        case ComputeContext::PROFILING_COMMAND_QUEUED:
        case ComputeContext::PROFILING_COMMAND_SUBMIT:
        case ComputeContext::PROFILING_COMMAND_START:
        case ComputeContext::PROFILING_COMMAND_END:
            err=clGetEventProfilingInfo(m_cl_Event, param_name, sizeof(cl_ulong), &ulong_units, NULL);
            if (err == CL_SUCCESS)
                return WebCLGetInfo(static_cast<unsigned int>(ulong_units));
            break;
        default:
            printf("Error: Unsupported Profiling Info type\n");
            return WebCLGetInfo();
    }

    switch (err) {

        case CL_PROFILING_INFO_NOT_AVAILABLE:
            ec = WebCLException::PROFILING_INFO_NOT_AVAILABLE;
            printf("Error: CL_PROFILING_INFO_NOT_AVAILABLE\n");
            break;
        case CL_INVALID_VALUE:
            ec = WebCLException::INVALID_VALUE;
            printf("Error: CL_INVALID_VALUE\n");
            break;
        case CL_INVALID_EVENT:
            ec = WebCLException::INVALID_EVENT;
            printf("Error: CL_INVALID_EVENT \n");
            break; 
        case CL_OUT_OF_RESOURCES:
            ec = WebCLException::OUT_OF_RESOURCES;
            printf("Error: CL_OUT_OF_RESOURCES\n");
            break;
        case CL_OUT_OF_HOST_MEMORY:
            ec = WebCLException::OUT_OF_HOST_MEMORY;
            printf("Error: CL_OUT_OF_HOST_MEMORY\n");
            break;
        default:
            ec = WebCLException::FAILURE;
            printf("Error: Invaild Error Type\n");
            break;
    }				
    return WebCLGetInfo();

}

void WebCLEvent::execComplete(cl_event ev, cl_int event_status, void* tptr) {
    printf("Not used params from setCallback %d  \n",event_status);
    //ev = NULL;
    //cl_event t = ev;
    //tptr = NULL;

    if (ev == NULL) 
    {
        printf(" ev is null ");
        //return;
    }
    if( tptr == NULL  )
    {
        printf(" tptr is NULL  ");
    }
    //WebCLEvent* self = static_cast<WebCLEvent*>(WebCLEvent::this_pointer);
    //(self->m_finishCallback.get())->handleEvent(17);

    WebCLEvent::callme((void*)(WebCLEvent::this_pointer));


    //(this->m_finishCallback.get())->handleEvent(17);
    printf("After handleEvent\n ");

    //return;

}

void WebCLEvent::callme(void* notify1)
{
    printf(" Inside Call Me  \n  ");
    WebCLEvent* self = static_cast<WebCLEvent*>(notify1);
    self->m_finishCallback.get()->handleEvent(17);
    //((WebCLFinishCallback*)(notify1))->handleEvent(17);
    printf(" Just Out Call Me \n ");

}
WebCLEvent* WebCLEvent::this_pointer = NULL;
void WebCLEvent::setCallback(int executionStatus, PassRefPtr<WebCLFinishCallback> notify, int userData, ExceptionCode& ec) //object userData
{
    UNUSED_PARAM(executionStatus);
    UNUSED_PARAM(notify);
    UNUSED_PARAM( userData);
    UNUSED_PARAM(ec);
    /*
       printf("WebCLEvent::setCallback is called\n");
       cl_int err = 0;

       if (m_cl_Event == NULL) {
       printf("Error: Invalid CL Event\n");
       ec = WebCLException::INVALID_EVENT;
       return;
       }

       m_finishCallback = notify;
    //void CL_CALLBACK(*const func)(cl_event,cl_int,void*) = &(this->execComplete) ;
    //func = &execComplete;
    //WebCLEvent::callme((void*)this);
    //err = clSetEventCallback(m_cl_Event, executionStatus,(const*)(&(WebCLEvent::execComplete)) ,NULL);//(void*)this);
    WebCLEvent::this_pointer =dynamic_cast<WebCLEvent*>(this);	
    err = clSetEventCallback(m_cl_Event, executionStatus, &(WebCLEvent::execComplete) ,NULL);//(void*)this);
    //WebCLEvent::callme((void*)this);
    printf(" after clSetEventCallback \n   ");
    //callme((void*)this);
    //executionStatus = CL_COMPLETE;
    //err = CL_SUCCESS;

    if (err != CL_SUCCESS) {
    switch (err) {
    case CL_INVALID_EVENT:
    printf("Error: CL_INVALID_EVENT \n");
    ec = WebCLException::INVALID_EVENT;
    break;
    case CL_INVALID_VALUE:
    printf("Error: CL_INVALID_VALUE \n");
    ec = WebCLException::INVALID_VALUE;
    break;
    case CL_OUT_OF_HOST_MEMORY:
    printf("Error: CL_OUT_OF_HOST_MEMORY \n");
    ec = WebCLException::OUT_OF_HOST_MEMORY;
    break;
    case CL_OUT_OF_RESOURCES:
    printf("Error: CL_OUT_OF_RESOURCES \n");
    ec = WebCLException::OUT_OF_RESOURCES;
    break;
    default:
    printf("Error: Invaild Error Type\n");
    printf("Unused Argument %d\n",userData);
    ec = WebCLException::FAILURE;
    break;
    }
    }
    */    
    return;
}


void WebCLEvent::setUserEventStatus (int exec_status, ExceptionCode& ec)
{
    cl_int err = -1;
    if (m_cl_Event == NULL) {
        printf("Error: Invalid CL Event\n");
        ec = WebCLException::INVALID_EVENT;
        return;
    }
    // TODO (siba samal) To be uncommented for  OpenCL 1.1	
    // http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clSetUserEventStatus.html
    if(exec_status == ComputeContext::COMPLETE) {
        //err = clSetUserEventStatus(m_cl_Event , ComputeContext::COMPLETE)
    }
    else if (exec_status < 0) {
        //err = clSetUserEventStatus(m_cl_Event , exec_status)
    }
    else
    {
        printf("Error: Invaild Error Type\n");
        return;
    }
    if (err != CL_SUCCESS) {
        switch (err) {
            case CL_INVALID_EVENT :
                printf("Error: CL_INVALID_EVENT \n");
                ec = WebCLException::INVALID_EVENT;
                break;
            case CL_INVALID_VALUE :
                printf("Error: CL_INVALID_VALUE  \n");
                ec = WebCLException::INVALID_VALUE ;
                break;
            case CL_OUT_OF_RESOURCES:
                printf("Error: CL_OUT_OF_RESOURCES  \n");
                ec = WebCLException::OUT_OF_RESOURCES ;
                break;
            case CL_INVALID_OPERATION:
                printf("Error: CL_INVALID_OPERATION  \n");
                ec = WebCLException::INVALID_OPERATION ;
                break;
            case CL_OUT_OF_HOST_MEMORY :
                printf("Error: CCL_OUT_OF_HOST_MEMORY \n");
                ec = WebCLException::OUT_OF_HOST_MEMORY;
                break;
            default:
                printf("Error: Invaild Error Type\n");
                ec = WebCLException::FAILURE;
                break;
        }

    } 
    return;
}

cl_event WebCLEvent::getCLEvent()
{
    return m_cl_Event;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)

