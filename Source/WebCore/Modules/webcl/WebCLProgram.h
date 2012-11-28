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

#ifndef WebCLProgram_h
#define WebCLProgram_h

#include <OpenCL/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/text/WTFString.h>
#include "ExceptionCode.h"

#include "WebCLDevice.h"
#include "WebCLDeviceList.h"
#include "WebCLFinishCallback.h"

namespace WebCore {

class WebCLContext;
class WebCLGetInfo;
class WebCLKernel;
class WebCLKernelList;

class WebCLProgram : public RefCounted<WebCLProgram> {
public:
	virtual ~WebCLProgram();
	static PassRefPtr<WebCLProgram> create(WebCLContext*, cl_program);
	WebCLGetInfo getInfo(int, ExceptionCode&);
	WebCLGetInfo getBuildInfo(WebCLDevice*, int, ExceptionCode&);
	PassRefPtr<WebCLKernel> createKernel(const String&, ExceptionCode&);
	void buildProgram(int, int, int, ExceptionCode&);
	void buildProgram(WebCLDevice*,int, int, int, ExceptionCode&);
	void buildProgram(WebCLDeviceList*,int, int, int, ExceptionCode&);
	void setDevice(RefPtr<WebCLDevice>);
	PassRefPtr<WebCLKernelList> createKernelsInProgram( ExceptionCode&); 
	cl_program getCLProgram();
	void build(WebCLDeviceList*,const String&, PassRefPtr<WebCLFinishCallback>, int, ExceptionCode&);
	void build(WebCLDeviceList* devices,const String& options,PassRefPtr<WebCLFinishCallback> finishCallBack, ExceptionCode& ec)
	{
		return build(devices,options,finishCallBack,0,ec);
	}
	void build(WebCLDeviceList* devices,const String& options, ExceptionCode& ec)
	{
		return build(devices,options,NULL,0,ec);
	}
	RefPtr<WebCLFinishCallback> m_finishCallback;

private:
	WebCLProgram(WebCLContext*, cl_program);
	//static void CL_CALLBACK finishCallBack(cl_program program,void *user_data);
	static void finishCallBack(cl_program program,void *user_data);
	static WebCLProgram *this_pointer;
	WebCLContext* m_context;
	cl_program m_cl_program;
	long m_num_programs;
	long m_num_kernels;
	Vector<RefPtr<WebCLProgram> > m_program_list;
	Vector<RefPtr<WebCLKernel> > m_kernel_list;
	RefPtr<WebCLDevice> m_device_id;
};

} // namespace WebCore

#endif // WebCLProgram_h
