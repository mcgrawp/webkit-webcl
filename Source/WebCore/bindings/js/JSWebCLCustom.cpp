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
#include "JSWebCLCustom.h"


#include "JSWebCL.h"
#include "JSWebCLContext.h"
#include "JSWebCLDevice.h"
#include "JSWebCLImage.h"
#include "JSWebCLKernel.h"
#include "JSWebCLPlatform.h"
#include "WebCL.h"
#include "WebCLContext.h"
#include "WebCLContextProperties.h"
#include "WebCLGetInfo.h"
#include "WebCLImage.h"

using namespace JSC;
using namespace std;

namespace WebCore { 

JSValue JSWebCL::getImageInfo(JSC::ExecState* exec)
{
    if (exec->argumentCount() != 2)
        return throwSyntaxError(exec);

    ExceptionCode ec = 0;
    WebCL* context = static_cast<WebCL*>(impl());
    WebCLImage* wimage  = toWebCLImage(exec->argument(0));
    if (exec->hadException())
        return jsUndefined();
    unsigned imageInfo  = exec->argument(1).toInt32(exec);
    if (exec->hadException())
        return jsUndefined();
    WebCLGetInfo info = context->getImageInfo(wimage, imageInfo, ec);
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }
    return toJS(exec, globalObject(), info);
}
JSValue JSWebCL::createContext(JSC::ExecState* exec)
{
    ExceptionCode ec = 0;
    RefPtr<WebCLContext> objWebCLContext;

    if (!exec->argumentCount()) {
        objWebCLContext  = m_impl->createContext(0, ec);
        if (ec) {
            setDOMException(exec, ec);
            return jsUndefined();
        }
        return toJS(exec, globalObject(), objWebCLContext.get());
    }

    if (exec->argumentCount() == 1) {
        RefPtr<WebCore::WebCLContextProperties> objWebCLContextProperties = WebCLContextProperties::create();
        if (exec->argumentCount() == 1 && exec->argument(0).isObject()) {
            JSObject* jsAttrs = exec->argument(0).getObject();
            Identifier platform(exec, "platform");
            if (jsAttrs->hasProperty(exec, platform))
                objWebCLContextProperties->setPlatform(toWebCLPlatform(jsAttrs->get(exec, platform)));
            Identifier devices(exec, "devices");
            if (jsAttrs->hasProperty(exec, devices))
                objWebCLContextProperties->setDevices(toWebCLDeviceList(jsAttrs->get(exec, devices)));
            Identifier deviceType(exec, "deviceType");
            if (jsAttrs->hasProperty(exec, deviceType))
                objWebCLContextProperties->setDeviceType(jsAttrs->get(exec, deviceType).toInt32(exec));
            Identifier shareGroup(exec, "shareGroup");
            if (jsAttrs->hasProperty(exec, shareGroup))
                objWebCLContextProperties->setShareGroup(jsAttrs->get(exec, shareGroup).toInt32(exec));
            Identifier hint(exec, "hint");
            if (jsAttrs->hasProperty(exec, hint))
                objWebCLContextProperties->setHint(jsAttrs->get(exec, hint).toString(exec)->value(exec));

            objWebCLContext  = m_impl->createContext(objWebCLContextProperties.get(), ec);
            if (ec) {
                setDOMException(exec, ec);
                return jsUndefined();
            }
            return toJS(exec, globalObject(), objWebCLContext.get());
        }
        return jsUndefined();
    }
    return jsUndefined();
}

    
EncodedJSValue JSC_HOST_CALL JSWebCLConstructor::constructJSWebCL(ExecState* exec)
{
    JSWebCLConstructor* jsConstructor = static_cast<JSWebCLConstructor*>(exec->callee());
    RefPtr<WebCL> webCLComputeContext = WebCL::create();
    return JSValue::encode(CREATE_DOM_WRAPPER(exec, jsConstructor->globalObject(), WebCL, webCLComputeContext.get()));
}

JSValue JSWebCL::getSupportedExtensions(ExecState* exec)
{
    WebCL* cl = static_cast<WebCL*>(impl());
    ExceptionCode ec = 0;
    Vector<String> value = cl->getSupportedExtensions(ec);
    MarkedArgumentBuffer list;
    for (size_t ii = 0; ii < value.size(); ++ii)
        list.append(jsString(exec, value[ii]));
    if (ec) {
        setDOMException(exec, ec);
        return jsUndefined();
    }
    return constructArray(exec, globalObject(), list);
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
