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

#include "JSDOMGlobalObject.h"
#include "DOMWindow.h"
#include "JSDOMWindow.h"
#include "JSDOMBinding.h"
#include "JSImageData.h"
#include "JSOESStandardDerivatives.h"
#include "JSOESTextureFloat.h"
#include "JSWebCLFinishCallback.h"
#include "NotImplemented.h"
#include <runtime/Error.h>
#include <runtime/JSArray.h>
#include <wtf/FastMalloc.h>
#include <runtime/JSFunction.h>
#include "WebCLGetInfo.h"
#include "JSWebCLEvent.h"
#include "JSWebCLCustom.h"
#include <stdio.h>

using namespace JSC;
using namespace std;

namespace WebCore { 

static PassRefPtr<WebCLFinishCallback> createFinishCallback(ExecState* exec, JSDOMGlobalObject* globalObject, JSValue value)
{
        //if (!value.inherits(&JSFunction::info)) {
        //      setDOMException(exec, TYPE_MISMATCH_ERR);
        //      return 0;
        //}
        if (value.isUndefinedOrNull())
        {
                setDOMException(exec, TYPE_MISMATCH_ERR);
                return 0;
        }
        JSObject* object = asObject(value);
        return JSWebCLFinishCallback::create(object, globalObject);
}



JSValue JSWebCLEvent::getInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 1)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLEvent* eventObj = static_cast<WebCLEvent*>(impl());	
	if (exec->hadException())
		return jsUndefined();
	unsigned event_info  = exec->argument(0).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = eventObj->getInfo(event_info, ec);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}

JSValue JSWebCLEvent::getProfilingInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 1)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLEvent* eventObj = static_cast<WebCLEvent*>(impl());
	if (exec->hadException())
		return jsUndefined();
	unsigned event_info  = exec->argument(0).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = eventObj->getProfilingInfo(event_info, ec);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}


JSValue JSWebCLEvent::setCallback(JSC::ExecState* exec)
{
	printf(" SWebCLEvent::setCallback(JSC::ExecState* exec)\n");
        if (exec->argumentCount() != 3)
                return throwSyntaxError(exec);

        if (exec->hadException())
                return jsUndefined();
        
	unsigned executionStatus  = exec->argument(0).toInt32(exec);
        if (exec->hadException())
                return jsUndefined();

        ExceptionCode ec = 0;
	 RefPtr<WebCLFinishCallback> callback;
    	if (!exec->argument(1).isUndefinedOrNull()) {
        	JSObject* object = exec->argument(1).getObject();
	        if (!object) {
        	    setDOMException(exec, TYPE_MISMATCH_ERR);
	            return jsUndefined();
        	}

        callback = JSWebCLFinishCallback::create(object, static_cast<JSDOMGlobalObject*>(globalObject()));
    }


        RefPtr<WebCLFinishCallback> finishCallback = createFinishCallback(exec, static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject()), exec->argument(1));
        if (exec->hadException())
                return jsUndefined();

        unsigned userParam  = exec->argument(2).toInt32(exec);
        if (exec->hadException())
                return jsUndefined();

        m_impl->setCallback(executionStatus,callback.release(), userParam, ec);
        if (ec) {
                setDOMException(exec, ec);
                return jsUndefined();
        }
        return jsUndefined();
}


} // namespace WebCore

#endif // ENABLE(WEBCL)
