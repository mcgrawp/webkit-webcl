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
#include "WebCLContextProperties.h"

#if ENABLE(WEBCL)

namespace WebCore {

PassRefPtr<WebCLContextProperties> WebCLContextProperties::create()
{
    return adoptRef(new WebCLContextProperties());
}

WebCLContextProperties::~WebCLContextProperties()
{
        
}
WebCLContextProperties::WebCLContextProperties()
{
	this->objplatform = NULL;
	this->objdevices =  NULL;
	this->objdeviceType = 0;
	this->objshareGroup = 0;
	// TODO Is it needed?
	//	this->hint = 0;
}


void WebCLContextProperties::setPlatform(WebCLPlatform* platform)
{
	this->objplatform = platform;
}

void WebCLContextProperties::setDevices(WebCLDeviceList* devices)
{
	this->objdevices = devices;
}

void WebCLContextProperties::setDeviceType(int deviceType)
{
	this->objdeviceType =deviceType;
}

void WebCLContextProperties::setShareGroup(int shareGroup)
{
	this->objshareGroup =shareGroup;
}

void WebCLContextProperties::setHint(String hint)
{
	this->objhint = hint;
}


WebCLPlatform* WebCLContextProperties::platform() const
{
	return this->objplatform;
} 
	 
	  
WebCLDeviceList* WebCLContextProperties::devices() const
{
	return this->objdevices;
}   
	 

int WebCLContextProperties::deviceType() const
{
	return this->objdeviceType;
}             
	  

int WebCLContextProperties::shareGroup() const
{
	return this->objshareGroup;
}            
	  

String WebCLContextProperties::hint() const
{
	return this->objhint;
}      
	  






}
#endif // ENABLE(WEBCL)




