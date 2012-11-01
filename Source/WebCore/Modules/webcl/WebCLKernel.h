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

#ifndef WebCLKernel_h
#define WebCLKernel_h

#include "ExceptionCode.h"
#include "WebCLDevice.h"
#include "WebCLMemoryObject.h"
#include "WebCLGetInfo.h"
#include "WebCLDeviceList.h"
#include "WebCLKernelList.h"
#include "WebCLKernelTypes.h"

#include <OpenCL/opencl.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

namespace WebCore {

class WebCL;
class WebCLGetInfo;

class WebCLKernel : public RefCounted<WebCLKernel> {
public:
	virtual ~WebCLKernel();
	static PassRefPtr<WebCLKernel> create(WebCLContext*, cl_kernel);
	WebCLGetInfo getInfo (int, ExceptionCode&);
	WebCLGetInfo getWorkGroupInfo(WebCLDevice*, int, ExceptionCode&);

	void setKernelArg(unsigned int, PassRefPtr<WebCLKernelTypeValue>, int, ExceptionCode&);
	void setKernelArgGlobal(unsigned int, WebCLMemoryObject*, ExceptionCode&);
	void setKernelArgConstant(unsigned int, WebCLMemoryObject*, ExceptionCode&);
	void setKernelArgLocal(unsigned int,unsigned int, ExceptionCode&);


	void setArg(unsigned int, PassRefPtr<WebCLKernelTypeValue>, unsigned int, ExceptionCode& ec);
	void setArg(unsigned int, WebCLMemoryObject*, ExceptionCode& ec);
	void setArg(unsigned int, unsigned int, ExceptionCode& ec);

	//unsigned long getKernelWorkGroupInfo(WebCLDeviceList*, int);
	void setDevice(RefPtr<WebCLDevice>);
	cl_kernel getCLKernel();

private:
	WebCLKernel(WebCLContext*, cl_kernel);

	template<class T> unsigned int clSetKernelArgPrimitiveType(cl_kernel cl_kernel_id, PassRefPtr<WebCLKernelTypeValue> kernelObject, T nodeId, unsigned int argIndex, int size);
	unsigned int clSetKernelArgVectorType(cl_kernel cl_kernel_id,PassRefPtr<WebCLKernelTypeValue> kernelObject, RefPtr<WebCLKernelTypeVector> array , unsigned int argIndex, int size,  unsigned int length);

	WebCLContext* m_context;
	cl_kernel m_cl_kernel;
	RefPtr<WebCLDevice> m_device_id;
	Vector<RefPtr<WebCLKernel> > m_kernel_list;
	long m_num_kernels;
};

} // namespace WebCore

#endif // WebCLKernel_h
