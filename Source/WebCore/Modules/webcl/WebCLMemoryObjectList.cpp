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

#include "WebCLMemoryObjectList.h"
#include "WebCL.h"
#include "WebCLMemoryObject.h"

namespace WebCore {

WebCLMemoryObjectList::WebCLMemoryObjectList()
{
    m_num_memory_obj = 0;
}

WebCLMemoryObjectList::~WebCLMemoryObjectList()
{
}

PassRefPtr<WebCLMemoryObjectList> WebCLMemoryObjectList::create(WebCL* ctx ,cl_mem *mem_obj_list,cl_uint num_mem_obj)
{
	
	return adoptRef(new WebCLMemoryObjectList(ctx,mem_obj_list,num_mem_obj));
}

WebCLMemoryObjectList::WebCLMemoryObjectList(WebCL* ctx ,cl_mem *mem_obj_list,cl_uint num_mem_obj)
:m_context(ctx)
{
	for (unsigned int i = 0 ; i < num_mem_obj ; i++) {
		RefPtr<WebCLMemoryObject> m_mem_object= WebCLMemoryObject::create(m_context,mem_obj_list[i],false);
		if (m_mem_object!= NULL) 
        {
			m_memory_obj_list.append(m_mem_object);
            m_num_memory_obj++;
		} 
	}
   printf(" WebCLMemoryObjectList  created with [%d] elements\n",m_num_memory_obj);
}

unsigned WebCLMemoryObjectList::length() const
{
	return m_num_memory_obj;
}

WebCLMemoryObject* WebCLMemoryObjectList::item(unsigned index)
{
	if (index >= m_num_memory_obj) {
		return 0;
	}
    WebCLMemoryObject *ret = (m_memory_obj_list[index]).get();
	return ret;
}


} // namespace WebCore

#endif // ENABLE(WEBCL)
