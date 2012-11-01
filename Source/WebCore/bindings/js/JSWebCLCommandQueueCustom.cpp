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
#include "WebCLProgram.h"
#include "WebCLDevice.h"
#include "WebCLGetInfo.h"
#include "JSWebCLDevice.h"
#include "JSWebCLCommandQueue.h"
#include "JSWebCLCustom.h"
#include <stdio.h>

using namespace JSC;
using namespace std;

namespace WebCore { 

JSValue JSWebCLCommandQueue::getInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 1)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCLCommandQueue*  queue = static_cast<WebCLCommandQueue*>(impl());
	if (exec->hadException())
		return jsUndefined();
	unsigned queue_info  = exec->argument(0).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = queue->getInfo(queue_info, ec);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return toJS(exec, globalObject(), info);
}
JSValue JSWebCLCommandQueue::finish(ExecState* exec)
{
	/*
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	//WebCLCommandQueue* queue  = toWebCLCommandQueue(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();

	ExceptionCode ec = 0;
	RefPtr<WebCLFinishCallback> finishCallback = createFinishCallback(exec, static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject()), exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	ASSERT(finishCallback);

	unsigned userParam  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();

	//m_impl->finish(queue ,finishCallback.release(), userParam);
	m_impl->finish(finishCallback.release(), userParam, ec);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return jsUndefined();
	*/
	ExceptionCode ec = 0;
	WebCLCommandQueue*  queue = static_cast<WebCLCommandQueue*>(impl());
	queue->finish(ec);
	if (ec) {
		setDOMException(exec, ec);
		return jsUndefined();
	}
	return jsUndefined();
}
} // namespace WebCore

#endif // ENABLE(WEBCL)
