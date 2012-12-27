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

#include "JSWebCLProgram.h"

#include "JSWebCLContext.h"
#include "JSWebCLCustom.h"
#include "JSWebCLDevice.h"
#include "JSWebCLFinishCallback.h"
#include "NotImplemented.h"
#include "WebCLDevice.h"
#include "WebCLGetInfo.h"
#include "WebCLProgram.h"

using namespace JSC;
using namespace std;

namespace WebCore {


JSValue JSWebCLProgram::getInfo(JSC::ExecState* exec)
{
    if (exec->argumentCount() != 1)
        return throwSyntaxError(exec);

    ExceptionCode ec = 0;
    WebCLProgram* program = static_cast<WebCLProgram*>(impl());
    unsigned programInfo  = exec->argument(0).toInt32(exec);
    if (exec->hadException())
        return jsUndefined();

    WebCLGetInfo info = program->getInfo(programInfo, ec);
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }
    return toJS(exec, globalObject(), info);
}

JSValue JSWebCLProgram::getBuildInfo(JSC::ExecState* exec)
{
    if (exec->argumentCount() != 2)
        return throwSyntaxError(exec);

    ExceptionCode ec = 0;
    WebCLProgram* program = static_cast<WebCLProgram*>(impl());
    WebCLDevice* device  = toWebCLDevice(exec->argument(0));
    if (exec->hadException())
        return jsUndefined();
    unsigned buildInfo  = exec->argument(1).toInt32(exec);
    if (exec->hadException())
        return jsUndefined();
    WebCLGetInfo info = program->getBuildInfo(device, buildInfo, ec);
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }
    return toJS(exec, globalObject(), info);
}

static PassRefPtr<WebCLFinishCallback> createFinishCallback(ExecState* exec, JSDOMGlobalObject* globalObject,
    JSValue value)
{
    if (value.isUndefinedOrNull()) {
        setDOMException(exec, TYPE_MISMATCH_ERR);
        return 0;
    }
    JSObject* object = asObject(value);
    return JSWebCLFinishCallback::create(object, globalObject);
}

JSValue JSWebCLProgram::build(JSC::ExecState* exec)
{
    if (exec->argumentCount() < 1)
        return throwSyntaxError(exec);

    WebCLDeviceList* devices = toWebCLDeviceList(exec->argument(0));
    if (exec->hadException())
        return jsUndefined();

    String options = "";
    if (exec->argumentCount() > 1 && !exec->argument(1).isNull())
        options = exec->argument(1).toString(exec)->value(exec);
    if (exec->hadException())
        return jsUndefined();

    ExceptionCode ec = 0;
    RefPtr<WebCLFinishCallback> callback;
    int userData = exec->argument(3).toInt32(exec);
    if (!exec->argument(2).getObject())
        m_impl->build(devices, options, 0 /*WebCLFinishCallback*/, userData, ec);
    else {
        if (!exec->argument(2).isUndefinedOrNull()) {
            JSObject* object = exec->argument(2).getObject();
            if (!object) {
                setDOMException(exec, TYPE_MISMATCH_ERR);
                return jsUndefined();
            }
            callback = JSWebCLFinishCallback::create(object, static_cast<JSDOMGlobalObject*>(globalObject()));
        }
        RefPtr<WebCLFinishCallback> finishCallback = createFinishCallback(exec,
            static_cast<JSDOMGlobalObject*>(exec->lexicalGlobalObject()), exec->argument(2));
        if (exec->hadException())
            return jsUndefined();
        m_impl->build(devices, options, callback.release(), userData, ec);
    }
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }
    return jsUndefined();
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
