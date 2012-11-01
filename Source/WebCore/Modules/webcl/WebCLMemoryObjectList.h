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

#ifndef WebCLMemoryObjectList_h
#define WebCLMemoryObjectList_h

#include <OpenCL/opencl.h>
//#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>
//#include "WebCLMemoryObject.h"

namespace WebCore {

class WebCL;
class WebCLMemoryObject;
class WebCLMemoryObjectList : public RefCounted<WebCLMemoryObjectList> {

public:
	WebCLMemoryObjectList();
	~WebCLMemoryObjectList();
    WebCLMemoryObjectList( WebCLMemoryObjectList& obj);
	
    static PassRefPtr<WebCLMemoryObjectList> create( WebCL*,cl_mem* ,cl_uint );
	unsigned length() const;
	WebCLMemoryObject* item(unsigned index);
private:
	WebCLMemoryObjectList( WebCL*, cl_mem * ,cl_uint );
	Vector<RefPtr<WebCLMemoryObject> > m_memory_obj_list;
	cl_uint m_num_memory_obj;
    WebCL* m_context;
};

} // namespace WebCore

#endif // WebCLMemoryObjectList_h
