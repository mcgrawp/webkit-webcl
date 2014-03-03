/*
 * Copyright (C) 2011, 2012, 2013 Samsung Electronics Corporation. All rights reserved.
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

#include "WebCLKernel.h"

#include "WebCLCommandQueue.h"
#include "WebCLContext.h"
#include "WebCLGetInfo.h"
#include "WebCLImageDescriptor.h"
#include "WebCLProgram.h"
#include "WebCLException.h"
#include "WebCLMemoryObject.h"
#include "WebCLSampler.h"

#include <wtf/ArrayBufferView.h>
#include <wtf/Float32Array.h>
#include <wtf/Int16Array.h>
#include <wtf/Int32Array.h>
#include <wtf/Int8Array.h>
#include <wtf/Uint16Array.h>
#include <wtf/Uint32Array.h>
#include <wtf/Uint8Array.h>

namespace WebCore {

WebCLKernel::~WebCLKernel()
{
    releasePlatformObject();
}

PassRefPtr<WebCLKernel> WebCLKernel::create(WebCLContext* context, WebCLProgram* program, const String& kernelName, ExceptionCode& ec)
{
    CCerror error = ComputeContext::SUCCESS;
    ComputeKernel* computeKernel = program->computeProgram()->createKernel(kernelName, error);
    if (error != ComputeContext::SUCCESS) {
        delete computeKernel;
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }
    return adoptRef(new WebCLKernel(context, program, computeKernel, kernelName));
}

Vector<RefPtr<WebCLKernel> > WebCLKernel::createKernelsInProgram(WebCLContext* context, WebCLProgram* program, ExceptionCode& ec)
{
    CCerror error = ComputeContext::SUCCESS;
    Vector<ComputeKernel*> computeKernels = program->computeProgram()->createKernelsInProgram(error);
    Vector<RefPtr<WebCLKernel> > kernels;
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return kernels;
    }

    Vector<char> functionName;
    for (size_t i = 0 ; i < computeKernels.size(); i++) {
        error = computeKernels[i]->getKernelInfo(ComputeContext::KERNEL_FUNCTION_NAME, &functionName);
        if (error != ComputeContext::SUCCESS) {
            ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
            kernels.clear();
            return kernels;
        }

        kernels.append(adoptRef(new WebCLKernel(context, program, computeKernels[i], String(functionName.data()))));
    }

    return kernels;
}

WebCLKernel::WebCLKernel(WebCLContext* context, WebCLProgram* program, ComputeKernel* kernel, const String& kernelName)
    : WebCLObjectImpl(kernel)
    , m_context(context)
    , m_program(program)
    , m_kernelName(kernelName)
    , m_argumentInfoProvider(this)
{
    context->trackReleaseableWebCLObject(createWeakPtr());
}

WebCLGetInfo WebCLKernel::getInfo(CCenum kernelInfo, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    CCerror err = ComputeContext::SUCCESS;
    switch (kernelInfo) {
    case ComputeContext::KERNEL_FUNCTION_NAME:
        return WebCLGetInfo(m_kernelName);
    case ComputeContext::KERNEL_NUM_ARGS: {
        CCuint numberOfArgs = 0;
        err = platformObject()->getKernelInfo(kernelInfo, &numberOfArgs);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<CCuint>(numberOfArgs));
        break;
    }
    case ComputeContext::KERNEL_PROGRAM:
        return WebCLGetInfo(m_program.get());
    case ComputeContext::KERNEL_CONTEXT:
        return WebCLGetInfo(m_context.get());
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

WebCLGetInfo WebCLKernel::getWorkGroupInfo(WebCLDevice* device, CCenum paramName, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    if (!device) {
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }

    CCDeviceID ccDevice = device->platformObject();

    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::KERNEL_WORK_GROUP_SIZE:
    case ComputeContext::KERNEL_PRIVATE_MEM_SIZE:
    case ComputeContext::KERNEL_LOCAL_MEM_SIZE:
    case ComputeContext::KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: {
        size_t kernelInfo = 0;
        err = platformObject()->getWorkGroupInfo(ccDevice, paramName, &kernelInfo);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<CCuint>(kernelInfo));
        break;
    }
    case ComputeContext::KERNEL_COMPILE_WORK_GROUP_SIZE: {
        Vector<size_t> workGroupSize;
        err = platformObject()->getWorkGroupInfo(ccDevice, paramName, &workGroupSize);
        if (err == ComputeContext::SUCCESS) {
            Vector<CCuint, 3> values;
            for (size_t i = 0; i < workGroupSize.size(); i++)
                values.uncheckedAppend(workGroupSize[i]);
            return WebCLGetInfo(values);
        }
        break;
    }
    default:
        ec = WebCLException::INVALID_VALUE;
        return WebCLGetInfo();
    }

    ASSERT(err != ComputeContext::SUCCESS);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
    return WebCLGetInfo();
}

void WebCLKernel::setArg(CCuint index, WebCLMemoryObject* memoryObject, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(memoryObject)) {
        ec = WebCLException::INVALID_MEM_OBJECT;
        return;
    }

    if (!WebCLInputChecker::isValidKernelArgIndex(this, index)) {
        ec = WebCLException::INVALID_ARG_INDEX;
        return;
    }

    CCerror err = platformObject()->setKernelArg(index, memoryObject->platformObject());
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(CCuint index, WebCLSampler* sampler, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    if (!WebCLInputChecker::validateWebCLObject(sampler)) {
        ec = WebCLException::INVALID_SAMPLER;
        return;
    }

    if (!WebCLInputChecker::isValidKernelArgIndex(this, index)) {
        ec = WebCLException::INVALID_ARG_INDEX;
        return;
    }

    ComputeSampler* computeSampler = sampler->platformObject();
    CCerror err = platformObject()->setKernelArg(index, computeSampler);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(CCuint index, ArrayBufferView* bufferView, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    if (!bufferView) {
        ec = WebCLException::INVALID_ARG_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidKernelArgIndex(this, index)) {
        ec = WebCLException::INVALID_ARG_INDEX;
        return;
    }

    String accessQualifier = m_argumentInfoProvider.argumentsInfo()[index]->addressQualifier();
    bool hasLocalQualifier = accessQualifier == "local";
    if (hasLocalQualifier) {
        if (bufferView->getType() != ArrayBufferView::TypeUint32) {
            ec = WebCLException::INVALID_ARG_VALUE;
            return;
        }

        Uint32Array* typedArray = static_cast<Uint32Array*>(bufferView);
        if (typedArray->length() != 1) {
            ec = WebCLException::INVALID_ARG_VALUE;
            return;
        }

        unsigned* value = static_cast<Uint32Array*>(bufferView)->data();
        CCerror err = platformObject()->setKernelArg(index, static_cast<size_t>(value[0]), 0 /* __local required null'ed data */);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
        return;
    }

    void* bufferData = 0;
    size_t arrayLength = 0;
    bool isLong  = m_argumentInfoProvider.argumentsInfo()[index]->typeName().contains("long");
    // FIXME: Add support for LONG, ULONG, HALF and DOUBLE types.
    // These need Int/Uint64Array, as well as Float16Array.
    switch(bufferView->getType()) {
    case (ArrayBufferView::TypeFloat32): // FLOAT
        bufferData = static_cast<Float32Array*>(bufferView)->data();
        arrayLength = bufferView->byteLength() / 4;
        break;
    case (ArrayBufferView::TypeUint32): // UINT
        bufferData = static_cast<Uint32Array*>(bufferView)->data();
        arrayLength = bufferView->byteLength() / 4;
        // For Long data type, input 
        if (isLong)
            arrayLength = arrayLength / 2;
        break;
    case (ArrayBufferView::TypeInt32):  // INT
        bufferData = static_cast<Int32Array*>(bufferView)->data();
        arrayLength = bufferView->byteLength() / 4;
        if (isLong)
            arrayLength = arrayLength / 2;
        break;
    case (ArrayBufferView::TypeUint16): // USHORT
        bufferData = static_cast<Uint16Array*>(bufferView)->data();
        arrayLength = bufferView->byteLength() / 2;
        break;
    case (ArrayBufferView::TypeInt16): // SHORT
        bufferData = static_cast<Int16Array*>(bufferView)->data();
        arrayLength = bufferView->byteLength() / 2;
        break;
    case (ArrayBufferView::TypeUint8): // UCHAR
        bufferData = static_cast<Uint8Array*>(bufferView)->data();
        arrayLength = bufferView->byteLength() / 1;
        break;
    case (ArrayBufferView::TypeInt8): // CHAR
        bufferData = static_cast<Int8Array*>(bufferView)->data();
        arrayLength = bufferView->byteLength() / 1;
        break;
    default:
        ASSERT_NOT_REACHED();
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!isValidVectorLength(arrayLength)) {
        ec = WebCLException::INVALID_ARG_VALUE;
        return;
    }
    size_t bufferDataSize = bufferView->byteLength();
    CCerror err = platformObject()->setKernelArg(index, bufferDataSize, bufferData);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

WebCLKernelArgInfo* WebCLKernel::getArgInfo(CCuint index, ExceptionCode& ec)
{
    if (isPlatformObjectNeutralized()) {
        ec = WebCLException::INVALID_KERNEL;
        return 0;
    }

    if (!WebCLInputChecker::isValidKernelArgIndex(this, index)) {
        ec = WebCLException::INVALID_ARG_INDEX;
        return 0;
    }

    return m_argumentInfoProvider.argumentsInfo().at(index).get();
}

WebCLProgram* WebCLKernel::program() const
{
    return m_program.get();
}

String WebCLKernel::kernelName() const
{
    return m_kernelName;
}

unsigned WebCLKernel::numberOfArguments()
{
    return m_argumentInfoProvider.argumentsInfo().size();
}

bool WebCLKernel::isValidVectorLength(size_t arrayLength)
{
    switch (arrayLength) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 8:
    case 16:
    case 32:
        return true;
    }
    return false;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
