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

#include "WebCLImage.h"
#include "WebCLException.h"
#include "webclcontext.h"
#include "WebCL.h"
#include "WebCLImageDescriptor.h"

namespace WebCore {

WebCLImageDescriptor::~WebCLImageDescriptor()
{
}

PassRefPtr<WebCLImageDescriptor> WebCLImageDescriptor::create()
{
    return adoptRef(new WebCLImageDescriptor()) ;
}
PassRefPtr<WebCLImageDescriptor> WebCLImageDescriptor::create(cl_image_format image_format)
{
    return adoptRef(new WebCLImageDescriptor(image_format)) ;
}


WebCLImageDescriptor::WebCLImageDescriptor()
{
      this->objchannelOrder=WebCL::DEFAULT_OBJECT_CHANNELORDER;
      this->objchannelType=WebCL::DEFAULT_OBJECT_CHANNELTYPE;
      this->objwidth  = WebCL::DEFAULT_OBJECT_WIDTH ;
      this->objheight = WebCL::DEFAULT_OBJECT_WIDTH;
      this->objrowPitch = WebCL::DEFAULT_OBJECT_WIDTH;
}
WebCLImageDescriptor::WebCLImageDescriptor(cl_image_format image_format)
{
    this->objchannelOrder = image_format.image_channel_order;
    this->objchannelType= image_format.image_channel_data_type;
    this->objwidth  = WebCL::DEFAULT_OBJECT_WIDTH ;
    this->objheight = WebCL::DEFAULT_OBJECT_WIDTH;
    this->objrowPitch = WebCL::DEFAULT_OBJECT_WIDTH;
}


void WebCLImageDescriptor::setChannelOrder(long channelOrder){
  this->objchannelOrder = channelOrder;
}

long WebCLImageDescriptor::channelOrder() const
{
	return objchannelOrder;
}

void WebCLImageDescriptor::setChannelType(long channelType){
  this->objchannelType = channelType;
}

long WebCLImageDescriptor::channelType() const
{
	return objchannelType;
}

void WebCLImageDescriptor::setWidth(long width){
  this->objwidth = width;
}

long WebCLImageDescriptor::width() const
{
	return objwidth;
}

void WebCLImageDescriptor::setHeight(long height){
  this->objheight  = height;
}

long WebCLImageDescriptor::height() const
{
	return objheight;
}

void WebCLImageDescriptor::setRowPitch(long rowPitch){
  this->objrowPitch = rowPitch;
}

long WebCLImageDescriptor::rowPitch() const
{
	return objrowPitch;
}

                

} // namespace WebCore

#endif // ENABLE(WEBCL)
