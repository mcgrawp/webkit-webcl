/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef JSCInlines_h
#define JSCInlines_h

// This file's only purpose is to collect commonly used *Inlines.h files, so that you don't
// have to include all of them in every .cpp file. Instead you just include this. It's good
// style to make sure that every .cpp file includes JSCInlines.h.
//
// JSC should never include this file, or any *Inline.h file, from interface headers, since
// this could lead to a circular dependency.
//
// WebCore, or any other downstream client of JSC, is allowed to include this file in headers.
// In fact, it can make a lot of sense: outside of JSC, this file becomes a kind of umbrella
// header that pulls in most (all?) of the interesting things in JSC.

#include "CallFrameInlines.h"
#include "ExceptionHelpers.h"
#include "GCIncomingRefCountedInlines.h"
#include "Interpreter.h"
#include "JSArrayBufferViewInlines.h"
#include "JSCJSValueInlines.h"
#include "JSFunctionInlines.h"
#include "JSProxy.h"
#include "JSString.h"
#include "Operations.h"
#include "SlotVisitorInlines.h"
#include "StructureInlines.h"

#endif // JSCInlines_h
