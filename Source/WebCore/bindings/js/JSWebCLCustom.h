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


#include "JSUint8Array.h"
#include "JSInt32Array.h"
#include "JSFloat32Array.h"
#include <runtime/JSFunction.h>
#include <runtime/Error.h>
#include <runtime/JSArray.h>

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
#include "JSWebCLDeviceList.h"
#include "JSWebCLContext.h"
#include "JSWebCLMemoryObject.h"
#include "JSWebCLEvent.h"
#include "JSWebCLSampler.h"
#include "JSWebCLImage.h"
#include "WebCLPlatform.h"
#include "WebCLKernel.h"  //not needeed
#include "WebCLProgram.h"
#include "WebCLDevice.h"
#include "WebCLCommandQueue.h"
#include "WebCLDeviceList.h"
#include "WebCLContext.h"
#include "WebCLMemoryObject.h"
#include "WebCLEvent.h"
#include "WebCLSampler.h"
#include "WebCLImage.h"
#include "WebCLBuffer.h"
#include "WebCLKernelTypes.h" //not needeed
#include "ScriptValue.h"  //not needeed


using namespace JSC;
using namespace std;

namespace WebCore {

static JSValue toJS(ExecState* exec, JSDOMGlobalObject* globalObject, const WebCLGetInfo& info)
{
	switch (info.getType()) {
		case WebCLGetInfo::kTypeBool:
			return jsBoolean(info.getBool());
		case WebCLGetInfo::kTypeBoolArray: {
											   MarkedArgumentBuffer list;
											   const Vector<bool>& value = info.getBoolArray();
											   for (size_t ii = 0; ii < value.size(); ++ii)
												   list.append(jsBoolean(value[ii]));
											   return constructArray(exec, list);
										   }
		case WebCLGetInfo::kTypeFloat:
										   return jsNumber(info.getFloat());
		case WebCLGetInfo::kTypeInt:
										   return jsNumber(info.getInt());
		case WebCLGetInfo::kTypeNull:
										   return jsNull();
		case WebCLGetInfo::kTypeString:
										   return jsString(exec, info.getString());
		case WebCLGetInfo::kTypeUnsignedInt:
										   return jsNumber(info.getUnsignedInt());
		case WebCLGetInfo::kTypeUnsignedLong:
										   return jsNumber(info.getUnsignedLong());
		case WebCLGetInfo::kTypeWebCLFloatArray:
										   return toJS(exec, globalObject, info.getWebCLFloatArray());
		case WebCLGetInfo::kTypeWebCLIntArray:
										   return toJS(exec, globalObject, info.getWebCLIntArray());
		case WebCLGetInfo::kTypeWebCLProgram:
										   return toJS(exec, globalObject, info.getWebCLProgram());
		case WebCLGetInfo::kTypeWebCLContext:
										   return toJS(exec, globalObject, info.getWebCLContext());
		case WebCLGetInfo::kTypeWebCLCommandQueue:
										   return toJS(exec, globalObject, info.getWebCLCommandQueue());
		case WebCLGetInfo::kTypeWebCLDevice:
										   return toJS(exec, globalObject, info.getWebCLDevice());
		case WebCLGetInfo::kTypeWebCLDeviceList:
										   return toJS(exec, globalObject, info.getWebCLDeviceList());
		default:
										   notImplemented();
										   return jsUndefined();
	}
}
} // namespace WebCore 

#endif // ENABLE(WEBCL)

