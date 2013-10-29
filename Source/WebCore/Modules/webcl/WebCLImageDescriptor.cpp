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

PassRefPtr<WebCLImageDescriptor> WebCLImageDescriptor::create(const CCImageFormat& imageFormat)
{
    return adoptRef(new WebCLImageDescriptor(imageFormat));
}

WebCLImageDescriptor::WebCLImageDescriptor()
{
    m_channelOrder = DEFAULT_CHANNEL_ORDER;
    m_channelType = DEFAULT_CHANNEL_TYPE;
    m_width = DEFAULT_WIDTH;
    m_height = DEFAULT_WIDTH;
    m_rowPitch = DEFAULT_WIDTH;
}

WebCLImageDescriptor::WebCLImageDescriptor(const CCImageFormat& imageFormat)
{
    m_channelOrder = imageFormat.image_channel_order;
    m_channelType= imageFormat.image_channel_data_type;
    m_width = DEFAULT_WIDTH;
    m_height = DEFAULT_HEIGHT;
    m_rowPitch = DEFAULT_ROW_PITCH;
}

void WebCLImageDescriptor::setChannelOrder(CCenum channelOrder)
{
    m_channelOrder = channelOrder;
}

CCenum WebCLImageDescriptor::channelOrder() const
{
    return m_channelOrder;
}

void WebCLImageDescriptor::setChannelType(CCenum channelType)
{
    m_channelType = channelType;
}

CCenum WebCLImageDescriptor::channelType() const
{
    return m_channelType;
}

void WebCLImageDescriptor::setWidth(CCuint width)
{
    m_width = width;
}

CCuint WebCLImageDescriptor::width() const
{
    return m_width;
}

void WebCLImageDescriptor::setHeight(CCuint height)
{
    m_height = height;
}

CCuint WebCLImageDescriptor::height() const
{
    return m_height;
}

void WebCLImageDescriptor::setRowPitch(CCuint rowPitch)
{
    m_rowPitch = rowPitch;
}

CCuint WebCLImageDescriptor::rowPitch() const
{
    return m_rowPitch;
}

} // namespace WebCore

#endif // ENABLE(WEBCL)
