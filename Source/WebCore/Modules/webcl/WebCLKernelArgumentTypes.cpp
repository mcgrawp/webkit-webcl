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

#include "WebCL.h"
#include "WebCLKernelArgumentTypes.h"
#include "ScriptExecutionContext.h"
#include "Document.h"
#include "DOMWindow.h"
#include "Image.h"
#include "SharedBuffer.h"
#include "CanvasRenderingContext2D.h"
#include "ImageBuffer.h"
#include "CachedImage.h"
//#include <wtf/ArrayBuffer.h>
//#include "CanvasPixelArray.h"
#include "HTMLCanvasElement.h"
#include "WebCLException.h"
#include <stdio.h>
//#include <wtf/ByteArray.h>
#include <CanvasRenderingContext.h>

class CanvasRenderingContext;

using namespace JSC;

namespace WebCore {   

/*	WebCLKernelArgumentTypes::WebCLKernelArgumentTypes(ScriptExecutionContext* context) : ActiveDOMObject(context, this)																				
	{
		m_num_mems = 0;
		m_num_programs = 0;
		m_num_events = 0;
		m_num_samplers = 0;
		m_num_contexts = 0;
		m_num_commandqueues = 0;
	}
*/
	PassRefPtr<WebCLKernelArgumentTypes> WebCLKernelArgumentTypes::create() //(ScriptExecutionContext* context)
	{
		//return adoptRef(new WebCLKernelArgumentTypes(context));
		return adoptRef(new WebCLKernelArgumentTypes());
	}

/*    PassRefPtr<WebCLKernelArgumentTypes> WebCLKernelArgumentTypes::create()
    {
        return(adoptRef(new WebCLKernelArgumentTypes());
    }
*/
    /*PassRefPtr<WebCLKernelArgumentTypes> WebCLKernelArgumentTypes::create()
    {
            return adoptRef(new WebCLKernelArgumentTypes());
    }*/

	WebCLKernelArgumentTypes::WebCLKernelArgumentTypes() //(ScriptExecutionContext* context): ActiveDOMObject(context, this)
	{}

    WebCLKernelArgumentTypes::~WebCLKernelArgumentTypes()
    {}
    


} // namespace WebCore

#endif // ENABLE(WEBCL)
