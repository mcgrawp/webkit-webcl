/*
 * Copyright (C) 2013 Samsung Electronics Corporation. All rights reserved.
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

#if ENABLE(WORKERS) && ENABLE(WEBCL)

#include "WorkerContextWebCLEnvironment.h"

#include "ScriptExecutionContext.h"
#include "WebCL.h"
#include <wtf/PassRefPtr.h>

namespace WebCore {

WorkerContextWebCLEnvironment::~WorkerContextWebCLEnvironment()
{
}

WorkerContextWebCLEnvironment::WorkerContextWebCLEnvironment(ScriptExecutionContext* context)
    : m_context(context)
{
}

WorkerContextWebCLEnvironment* WorkerContextWebCLEnvironment::from(ScriptExecutionContext* context)
{
    DEFINE_STATIC_LOCAL(AtomicString, name, ("WorkerContextWebCLEnvironment"));
    WorkerContextWebCLEnvironment* supplement =
        static_cast<WorkerContextWebCLEnvironment*>(Supplement<ScriptExecutionContext>::from(context, name));
    if (!supplement) {
        supplement = new WorkerContextWebCLEnvironment(context);
        provideTo(context, name, adoptPtr(supplement));
    }
    return supplement;
}

WebCL* WorkerContextWebCLEnvironment::webcl(ScriptExecutionContext* context)
{
    return WorkerContextWebCLEnvironment::from(context)->webcl();
}

WebCL* WorkerContextWebCLEnvironment::webcl() const
{
    if (!m_webcl)
        m_webcl = WebCL::create();
    return m_webcl.get();
}
}
#endif // ENABLE(WEBCL)

