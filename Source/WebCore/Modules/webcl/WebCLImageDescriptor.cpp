/*
 * Copyright (C) 2011, 2013 Samsung Electronics Corporation. All rights reserved.
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

#include "WebCLImageDescriptor.h"

namespace WebCore {

WebCLImageDescriptor::~WebCLImageDescriptor()
{
}

PassRefPtr<WebCLImageDescriptor> WebCLImageDescriptor::create()
{
    return adoptRef(new WebCLImageDescriptor);
}

PassRefPtr<WebCLImageDescriptor> WebCLImageDescriptor::create(CCImageFormat imageFormat)
{
    return adoptRef(new WebCLImageDescriptor(imageFormat));
}

WebCLImageDescriptor::WebCLImageDescriptor()
{
    m_channelOrder = DEFAULT_OBJECT_CHANNELORDER;
    m_channelType = DEFAULT_OBJECT_CHANNELTYPE;
    m_width = DEFAULT_OBJECT_WIDTH;
    m_height = DEFAULT_OBJECT_WIDTH;
    m_rowPitch = DEFAULT_OBJECT_WIDTH;
}

WebCLImageDescriptor::WebCLImageDescriptor(CCImageFormat imageFormat)
{
    m_channelOrder = imageFormat.image_channel_order;
    m_channelType= imageFormat.image_channel_data_type;
    m_width = DEFAULT_OBJECT_WIDTH;
    m_height = DEFAULT_OBJECT_WIDTH;
    // FIXME: PITCH_ROW = OBJECT_WIDTH?
    m_rowPitch = DEFAULT_OBJECT_WIDTH;
}

void WebCLImageDescriptor::setChannelOrder(long channelOrder)
{
    m_channelOrder = channelOrder;
}

long WebCLImageDescriptor::channelOrder() const
{
    return m_channelOrder;
}

void WebCLImageDescriptor::setChannelType(long channelType)
{
    m_channelType = channelType;
}

long WebCLImageDescriptor::channelType() const
{
    return m_channelType;
}

void WebCLImageDescriptor::setWidth(long width)
{
    m_width = width;
}

long WebCLImageDescriptor::width() const
{
    return m_width;
}

void WebCLImageDescriptor::setHeight(long height)
{
    m_height = height;
}

long WebCLImageDescriptor::height() const
{
    return m_height;
}

void WebCLImageDescriptor::setRowPitch(long rowPitch)
{
    m_rowPitch = rowPitch;
}

long WebCLImageDescriptor::rowPitch() const
{
    return m_rowPitch;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
