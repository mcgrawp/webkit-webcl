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

#ifndef WebCLMemoryObjectoryObject_h
#define WebCLMemoryObjectoryObject_h

//WebCL Inlcudes
#include "WebCLException.h"
/*
#include "WebCL.h"
#include "WebCLGLObjectInfo.h"
#include "WebCLGetInfo.h"
*/

#include <OpenCL/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/Forward.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class WebCL;
class WebCLException;
class WebCLGLObjectInfo;
class WebCLGetInfo;

class WebCLMemoryObject : public RefCounted<WebCLMemoryObject> {
public:
	~WebCLMemoryObject();
    //WebCLMemoryObject(){ m_cl_mem = NULL; m_shared = false; m_context = NULL; };	
    static PassRefPtr<WebCLMemoryObject> create(WebCL*, cl_mem, bool);
	cl_mem getCLMemoryObject();
	bool isShared(); 
    WebCLGetInfo getInfo(int, ExceptionCode&);
    PassRefPtr<WebCLGLObjectInfo> getGLObjectInfo(ExceptionCode&);
protected:
    // Needs to be visible to child classes
    WebCL* m_context;
	cl_mem m_cl_mem;
	bool m_shared;
	WebCLMemoryObject(WebCL*, cl_mem, bool);
private:
    // TO be included in List class.. commenting
	//long m_num_mems;
	//Vector<RefPtr<WebCLMemoryObject> > m_mem_list;
};

} // namespace WebCore

#endif // WebCLMemoryObject_h
