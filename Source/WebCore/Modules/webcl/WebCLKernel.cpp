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
    CCKernel computeContextKernel = context->computeContext()->createKernel(program->platformObject(), kernelName, error);
    if (!computeContextKernel) {
        ASSERT(error != ComputeContext::SUCCESS);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return 0;
    }

    return adoptRef(new WebCLKernel(context, program, computeContextKernel, kernelName));
}

Vector<RefPtr<WebCLKernel> > WebCLKernel::createKernelsInProgram(WebCLContext* context, WebCLProgram* program, ExceptionCode& ec)
{
    CCerror error = ComputeContext::SUCCESS;
    Vector<CCKernel> computeContextKernels = context->computeContext()->createKernelsInProgram(program->platformObject(), error);
    Vector<RefPtr<WebCLKernel> > kernels;
    if (error != ComputeContext::SUCCESS) {
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
        return kernels;
    }

    Vector<char> functionName;
    for (size_t i = 0 ; i < computeContextKernels.size(); i++) {
        error = ComputeContext::getKernelInfo(computeContextKernels[i], ComputeContext::KERNEL_FUNCTION_NAME, &functionName);
        if (error != ComputeContext::SUCCESS) {
            ec = WebCLException::computeContextErrorToWebCLExceptionCode(error);
            kernels.clear();
            return kernels;
        }

        kernels.append(adoptRef(new WebCLKernel(context, program, computeContextKernels[i], String(functionName.data()))));
    }

    return kernels;
}

WebCLKernel::WebCLKernel(WebCLContext* context, WebCLProgram* program, CCKernel kernel, const String& kernelName)
    : WebCLObjectImpl(kernel)
    , m_context(context)
    , m_program(program)
    , m_kernelName(kernelName)
    , m_argumentList(this)
{
    context->trackReleaseableWebCLObject(createWeakPtr());
}

WebCLGetInfo WebCLKernel::getInfo(CCenum kernelInfo, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    CCerror err = ComputeContext::SUCCESS;
    switch (kernelInfo) {
    case ComputeContext::KERNEL_FUNCTION_NAME:
        return WebCLGetInfo(m_kernelName);
    case ComputeContext::KERNEL_NUM_ARGS: {
        CCuint numberOfArgs = 0;
        err = ComputeContext::getKernelInfo(platformObject(), kernelInfo, &numberOfArgs);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(numberOfArgs));
        break;
    }

    // FIXME: This backpointer to m_program can be dangling in
    // case it gets deleted (JS gc'ed), etc.
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
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return WebCLGetInfo();
    }

    if (!WebCLInputChecker::validateWebCLObject(device)) {
        ec = WebCLException::INVALID_DEVICE;
        return WebCLGetInfo();
    }

    CCDeviceID ccDevice = device->platformObject();

    CCerror err = 0;
    switch (paramName) {
    case ComputeContext::KERNEL_WORK_GROUP_SIZE:
    case ComputeContext::KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: {
        size_t kernelInfo = 0;
        err = ComputeContext::getWorkGroupInfo(platformObject(), ccDevice, paramName, &kernelInfo);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned>(kernelInfo));
        break;
    }
    case ComputeContext::KERNEL_COMPILE_WORK_GROUP_SIZE: {
        Vector<size_t> workGroupSize;
        err = ComputeContext::getWorkGroupInfo(platformObject(), ccDevice, paramName, &workGroupSize);
        if (err == ComputeContext::SUCCESS) {
            RefPtr<Int32Array> values = Int32Array::create(3);
            for (int i = 0; i < 3; i++)
                values->set(i, workGroupSize[i]);
            return WebCLGetInfo(values.release());
        }
        break;
    }
    case ComputeContext::KERNEL_PRIVATE_MEM_SIZE:
    case ComputeContext::KERNEL_LOCAL_MEM_SIZE: {
        CCulong localMemSize = 0;
        err = ComputeContext::getWorkGroupInfo(platformObject(), ccDevice, paramName, &localMemSize);
        if (err == ComputeContext::SUCCESS)
            return WebCLGetInfo(static_cast<unsigned long>(localMemSize));
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

void WebCLKernel::setDevice(PassRefPtr<WebCLDevice> deviceID)
{
    m_deviceID = deviceID;
}

const WebCLKernel::Argument& WebCLKernel::argument(unsigned index)
{
    return m_argumentList.it()[index];
}

void WebCLKernel::releasePlatformObjectImpl()
{
    CCerror computeContextErrorCode = m_context->computeContext()->releaseKernel(platformObject());
    ASSERT_UNUSED(computeContextErrorCode, computeContextErrorCode == ComputeContext::SUCCESS);
}

void WebCLKernel::setArg(CCuint index, WebCLMemoryObject* memoryObject, ExceptionCode& ec)
{
    if (!platformObject()) {
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

    PlatformComputeObject ccMemoryObject = memoryObject->platformObject();
    CCerror err = ComputeContext::setKernelArg(platformObject(), index, sizeof(PlatformComputeObject), &ccMemoryObject);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(CCuint index, WebCLSampler* sampler, ExceptionCode& ec)
{
    if (!platformObject()) {
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

    CCSampler ccSampler = sampler->platformObject();
    CCerror err = ComputeContext::setKernelArg(platformObject(), index, sizeof(CCSampler), &ccSampler);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

void WebCLKernel::setArg(CCuint index, ArrayBufferView* bufferView, ExceptionCode& ec)
{
    if (!platformObject()) {
        ec = WebCLException::INVALID_KERNEL;
        return;
    }

    if (!bufferView) {
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    if (!WebCLInputChecker::isValidKernelArgIndex(this, index)) {
        ec = WebCLException::INVALID_ARG_INDEX;
        return;
    }

    bool hasLocalQualifier = argument(index).hasQualifier(Argument::Local);
    if (hasLocalQualifier) {
        if (bufferView->getType() != ArrayBufferView::TypeUint32) {
            ec = WebCLException::INVALID_VALUE;
            return;
        }

        Uint32Array* typedArray = static_cast<Uint32Array*>(bufferView);
        if (typedArray->length() != 1) {
            ec = WebCLException::INVALID_VALUE;
            return;
        }

        unsigned* value = static_cast<Uint32Array*>(bufferView)->data();
        CCerror err = ComputeContext::setKernelArg(platformObject(), index, static_cast<size_t>(value[0]), 0 /* __local required null'ed data */);
        ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
        return;
    }

    void* bufferData = 0;

    // FIXME: Add support for LONG, ULONG, HALF and DOUBLE types.
    // These need Int/Uint64Array, as well as Float16Array.
    switch(bufferView->getType()) {
    case (ArrayBufferView::TypeFloat32): // FLOAT
        bufferData = static_cast<Float32Array*>(bufferView)->data();
        break;
    case (ArrayBufferView::TypeUint32): // UINT
        bufferData = static_cast<Uint32Array*>(bufferView)->data();
        break;
    case (ArrayBufferView::TypeInt32):  // INT
        bufferData = static_cast<Int32Array*>(bufferView)->data();
        break;
    case (ArrayBufferView::TypeUint16): // USHORT
        bufferData = static_cast<Uint16Array*>(bufferView)->data();
        break;
    case (ArrayBufferView::TypeInt16): // SHORT
        bufferData = static_cast<Int16Array*>(bufferView)->data();
        break;
    case (ArrayBufferView::TypeUint8): // UCHAR
        bufferData = static_cast<Uint8Array*>(bufferView)->data();
        break;
    case (ArrayBufferView::TypeInt8): // CHAR
        bufferData = static_cast<Int8Array*>(bufferView)->data();
        break;
    default:
        ASSERT_NOT_REACHED();
        ec = WebCLException::INVALID_VALUE;
        return;
    }

    size_t bufferDataSize = bufferView->byteLength();
    CCerror err = ComputeContext::setKernelArg(platformObject(), index, bufferDataSize, bufferData);
    ec = WebCLException::computeContextErrorToWebCLExceptionCode(err);
}

unsigned WebCLKernel::numberOfArguments()
{
    return m_argumentList.it().size();
}

static bool isASCIILineBreak(UChar c)
{
    return c == '\r' || c == '\n';
}

WebCLKernel::ArgumentList::ArgumentList(WebCLKernel* kernel)
    : m_kernel(kernel)
{
}

const Vector<WebCLKernel::Argument>& WebCLKernel::ArgumentList::it()
{
    ensureArgumentData();
    return m_argumentData;
}

// FIXME: This method has to return an error code.
void WebCLKernel::ArgumentList::ensureArgumentData()
{
    if (m_argumentData.size())
        return;

    const String& source = m_kernel->m_program->sourceWithCommentsStripped();

    // 1) find "__kernel" string.
    // 2) find the first open braces past __kernel.
    // 3) reverseFind the given kernel name string.
    // 4) if not found go back to (1)
    // 5) if found, parse its argument list.
    size_t startIndex = 0;
    size_t kernelNameIndex = 0;
    while (1) {
        size_t kernelDeclarationIndex = source.find("__kernel", startIndex);
        if (kernelDeclarationIndex == WTF::notFound)
            return;

        size_t openBrace = source.find("{", kernelDeclarationIndex + 8);
        kernelNameIndex = source.reverseFind(m_kernel->m_kernelName, openBrace);

        ASSERT(kernelNameIndex > kernelDeclarationIndex);
        if (kernelNameIndex != WTF::notFound)
            break;

        startIndex = kernelDeclarationIndex + 8;
    }

    ASSERT(kernelNameIndex);
    size_t openBraket = source.find("(", kernelNameIndex);
    size_t closeBraket = source.find(")", openBraket);
    String argumentListStr = source.substring(openBraket + 1, closeBraket - openBraket - 1);

    Vector<String> argumentStrVector;
    argumentListStr.split(",", argumentStrVector);
    for (size_t i = 0; i < argumentStrVector.size(); ++i) {
        argumentStrVector[i] = argumentStrVector[i].removeCharacters(isASCIILineBreak);
        m_argumentData.append(Argument(argumentStrVector[i]));
    }
}

// FIXME: A better wat to fix this would be splitting out tokens by space,
// and handle each token.
WebCLKernel::Argument::Argument(const String& argumentDeclaration)
    : m_hasLocalQualifier(argumentDeclaration.contains("__local"))
    , m_hasGlobalQualifier(argumentDeclaration.contains("__global"))
    , m_hasConstQualifier(argumentDeclaration.contains("const"))
{
}

bool WebCLKernel::Argument::hasQualifier(enum Qualifier qualifier) const
{
    switch (qualifier) {
    case Local:
        return m_hasLocalQualifier;
    case Global:
        return m_hasGlobalQualifier;
    case Const:
        return m_hasConstQualifier;
    }

    ASSERT_NOT_REACHED();
    return false;
}
} // namespace WebCore

#endif // ENABLE(WEBCL)
