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
#include "WebCLGLObjectInfo.h"

#if ENABLE(WEBCL)

namespace WebCore {

PassRefPtr<WebCLGLObjectInfo> WebCLGLObjectInfo::create()
{
    return adoptRef(new WebCLGLObjectInfo());
}


WebCLGLObjectInfo::~WebCLGLObjectInfo()
{
        
}
WebCLGLObjectInfo::WebCLGLObjectInfo()
{

    this->objGLObjectType = 0;
    this->objGLObject = 0;
}

WebCLGLObjectInfo::WebCLGLObjectInfo(unsigned int/*long*/ *gl_object_type,
                   unsigned int  *gl_object_name)
{ 
    this->objGLObjectType = *gl_object_type;
    this->objGLObject = *gl_object_name;

}

PassRefPtr<WebCLGLObjectInfo> WebCLGLObjectInfo::create(unsigned int/*long*/ *gl_object_type, unsigned int  *gl_object_name)
{
        return adoptRef(new WebCLGLObjectInfo(gl_object_type, gl_object_name)); 
}

void WebCLGLObjectInfo::setGlObjectType(unsigned int/*long*/ glObjectType)
{
	this->objGLObjectType = glObjectType;
}

void WebCLGLObjectInfo::setGlObject(unsigned int glObject)
{
	this->objGLObject = glObject;
}

unsigned int /*long*/ WebCLGLObjectInfo::glObjectType() const
{
	return this->objGLObjectType;
}            
	  

unsigned int WebCLGLObjectInfo::glObject() const
{
	return this->objGLObject;
}      

}
#endif // ENABLE(WEBCL)




