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
#include "JSFloat32Array.h"
#include "JSInt32Array.h"
#include "JSWebCLContext.h"
#include "JSUint8Array.h"
#include "JSWebKitCSSMatrix.h"
#include "NotImplemented.h"
#include "OESStandardDerivatives.h"
#include "OESTextureFloat.h"
#include <runtime/Error.h>
#include <runtime/JSArray.h>
#include <wtf/FastMalloc.h>
#include <runtime/JSFunction.h>
#include "WebCLKernel.h"
#include "WebCLDevice.h"
#include "WebCLKernelTypes.h"
#include <wtf/Float32Array.h>
#include "WebCLGetInfo.h"
#include <wtf/Int32Array.h>
#include "ScriptValue.h"
#include "JSWebCLDevice.h"
#include "JSWebCLKernel.h"
#include "JSWebCLCustom.h"
#include "JSWebCLMemoryObject.h"
#include <stdio.h>

using namespace JSC;
using namespace std;

namespace WebCore { 



JSValue JSWebCLKernel::getInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 1)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLKernel* kernel = static_cast<WebCLKernel*>(impl());	
	if (exec->hadException())
		return jsUndefined();
	unsigned kernel_info  = exec->argument(0).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = kernel->getInfo(kernel_info, ec);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}

JSValue JSWebCLKernel::getWorkGroupInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLKernel* kernel = static_cast<WebCLKernel*>(impl());
	WebCLDevice* device  = toWebCLDevice(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned work_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = kernel->getWorkGroupInfo(device,work_info, ec);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}

JSValue JSWebCLKernel::setArg(ExecState* exec)
{
	
	unsigned count = exec->argumentCount();
	

	if (count < 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLKernel* kernel = static_cast<WebCLKernel*>(impl());	
	if (exec->hadException())
		return jsUndefined();

	unsigned argIndex  = exec->argument(0).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();

	if(count  == 3)
	{
		ScriptValue object(exec->globalData(), exec->argument(1));
		if (exec->hadException())
			return jsUndefined();

		unsigned argType  = exec->argument(2).toInt32(exec);
		if (exec->hadException())
			return jsUndefined();
   		kernel->setArg(argIndex, object.toWebCLKernelTypeValue(exec),argType, ec);
		if (ec) {
			setDOMException(exec, ec);
		return jsUndefined();
		}


		return jsUndefined();
	}
	else{
		if(count == 2){

			unsigned argIndex  = exec->argument(0).toInt32(exec);
			if (exec->hadException())
			return jsUndefined();
			
			if(exec->argument(1).isInt32()){

				unsigned argSize = exec->argument(1).toInt32(exec);
				kernel->setArg(argIndex, argSize, ec);


			}
			else
			{
				WebCLMemoryObject* value = toWebCLMemoryObject(exec->argument(1));
				ScriptValue object(exec->globalData(), exec->argument(1));
				if (exec->hadException())
					return jsUndefined();
				//kernel->setArg(argIndex, object.toWebCLMemoryObject(exec), ec);
				kernel->setArg(argIndex, value, ec);
				return jsUndefined();

			}			

			if (ec) {
				setDOMException(exec, ec);
				return jsUndefined();
			}
			return jsUndefined();


		}
		else{
			if (ec) {
				setDOMException(exec, ec);
				return jsUndefined();
			}
			return jsUndefined();

		
		}

	}


	

}


} // namespace WebCore

#endif // ENABLE(WEBCL)
