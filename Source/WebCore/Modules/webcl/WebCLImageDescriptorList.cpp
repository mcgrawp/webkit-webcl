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

#include "WebCLImageDescriptorList.h"
#include "WebCL.h"
#include "WebCLImageDescriptor.h"

namespace WebCore {

WebCLImageDescriptorList::WebCLImageDescriptorList()
{
    m_num_image_descriptor = 0;
}

WebCLImageDescriptorList::~WebCLImageDescriptorList()
{
}

PassRefPtr<WebCLImageDescriptorList> WebCLImageDescriptorList::create(cl_image_format* image_format_list,cl_uint num_image_format)
{
	
	return adoptRef(new WebCLImageDescriptorList(image_format_list,num_image_format));
}

WebCLImageDescriptorList::WebCLImageDescriptorList(cl_image_format* image_format_list,cl_uint num_image_format)
{
	for (unsigned int i = 0 ; i < num_image_format ; i++) {
		RefPtr<WebCLImageDescriptor> m_image_descriptor = WebCLImageDescriptor::create(image_format_list[i]);
		if (m_image_descriptor != NULL) 
        {
			m_image_descriptor_list.append(m_image_descriptor);
            m_num_image_descriptor++;
            printf("\n WebCLImageDescriptorList [%ld , %ld ]\t",m_image_descriptor->channelOrder(),m_image_descriptor->channelType());
		} 
	}
    for(unsigned int i=0;i<num_image_format;i++){
         printf("\n m_image_descriptor_list [%ld , %ld ]\t",m_image_descriptor_list[i]->channelOrder(),m_image_descriptor_list[i]->channelType());
    }
    printf(" WebCLImageDescriptorList  created with [%d] elements\n",m_num_image_descriptor);
	
}

unsigned WebCLImageDescriptorList::length() const
{
	return m_num_image_descriptor;
}

WebCLImageDescriptor* WebCLImageDescriptorList::item(unsigned index)
{
	if (index >= m_num_image_descriptor) {
		return 0;
	}
    WebCLImageDescriptor *ret = (m_image_descriptor_list[index]).get();
	return ret;
}


} // namespace WebCore

#endif // ENABLE(WEBCL)
