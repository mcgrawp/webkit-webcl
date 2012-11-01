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
#include <runtime/JSFunction.h>
#include "JSWebCL.h"
#include "JSDOMBinding.h"
#include "WebCL.h"
#include "JSImageData.h"
#include "JSOESStandardDerivatives.h"
#include "JSOESTextureFloat.h"


#include "JSWebCLContext.h"

#include "JSWebKitCSSMatrix.h"
#include "NotImplemented.h"
#include "OESStandardDerivatives.h"
#include "OESTextureFloat.h"
#include <wtf/Float32Array.h>
#include "WebCLGetInfo.h"
#include <wtf/Int32Array.h>
#include <runtime/Error.h>
#include <runtime/JSArray.h>
#include <wtf/FastMalloc.h>
#include "JSWebCLPlatform.h"
#include "JSWebCLDevice.h"
#include "JSWebCLKernel.h"
#include "JSWebCLProgram.h"
#include "JSWebCLCommandQueue.h"
#include "JSWebCLContext.h"
#include "JSWebCLEvent.h"
#include "JSWebCLSampler.h"
#include "JSWebCLImage.h"
#include "WebCLPlatform.h"
#include "WebCLKernel.h"  //not needeed
#include "WebCLProgram.h"
#include "WebCLDevice.h"
#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLEvent.h"
#include "WebCLSampler.h"
#include "WebCLImage.h"
#include "WebCLBuffer.h"
#include "WebCLKernelTypes.h" //not needeed
#include "ScriptValue.h"  //not needeed
#include "JSWebCLCustom.h"
#include "WebCLContextProperties.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <stdio.h>

using namespace JSC;
using namespace std;

namespace WebCore { 

//static JSC::JSValue toJS(JSC::ExecState* exec, JSDOMGlobalObject* globalObject, int user_data)
//{
//	return getDOMObjectWrapper<JSWebCL>(exec, globalObject, object);
//}



JSValue JSWebCL::getImageInfo(JSC::ExecState* exec)
{
	if (exec->argumentCount() != 2)
		return throwSyntaxError(exec);

	ExceptionCode ec = 0;
	WebCL* context = static_cast<WebCL*>(impl());
	WebCLImage* wimage  = toWebCLImage(exec->argument(0));
	//WebCLImage* wimage  = toWebCLMemoryObject(exec->argument(0));
	if (exec->hadException())
		return jsUndefined();
	unsigned image_info  = exec->argument(1).toInt32(exec);
	if (exec->hadException())
		return jsUndefined();
	WebCLGetInfo info = context->getImageInfo(wimage,image_info,ec);
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
    
    //printf(" inside  JSWebCL::createContext ");
    
	if (exec->argumentCount() == 0)
	{
       // m_impl->createContext(NULL,ec);
        
        objWebCLContext  = m_impl->createContext(NULL,ec);
        if (ec) {
            setDOMException(exec, ec);
            return jsUndefined();
        }
        else
        {
            return toJS(exec, globalObject(), objWebCLContext.get());
        }

        return jsUndefined();
    }
    else
    {
        if(exec->argumentCount() == 1)
        {
            PassRefPtr<WebCore::WebCLContextProperties> objWebCLContextProperties = WebCLContextProperties::create();
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
                    //ustringToString(exec->argument(0).toString(exec)->value(exec))
                    objWebCLContextProperties->setHint(ustringToString(jsAttrs->get(exec, hint).toString(exec)->value(exec)));
                
                
                
                objWebCLContext  = m_impl->createContext(objWebCLContextProperties.get() ,ec);
                if (ec) {
                        setDOMException(exec, ec);
                        return jsUndefined();
                }
                else
                {
                    return toJS(exec, globalObject(), objWebCLContext.get());
                }
                return jsUndefined();
                
            }
                return jsUndefined();
            
        }
    }
    
    return jsUndefined();
	

}

    
EncodedJSValue JSC_HOST_CALL JSWebCLConstructor::constructJSWebCL(ExecState* exec)
{
	JSWebCLConstructor* jsConstructor = static_cast<JSWebCLConstructor*>(exec->callee());
	ScriptExecutionContext* context = jsConstructor->scriptExecutionContext();
	RefPtr<WebCL> webCLComputeContext = WebCL::create(context);

	//DOMWindow* window = asJSDOMWindow(exec->lexicalGlobalObject())->impl();
	//RefPtr<WebCL> webCLComputeContext = WebCL::create(window->document());
	//return JSValue::encode(asObject(toJS(exec, jsConstructor->globalObject())));
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
