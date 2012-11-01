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

#include "config.h"

#if ENABLE(WEBCL)

#include "WebCLImage.h"
#include "WebCLException.h"
#include "WebCLContext.h"
#include "WebCL.h"

namespace WebCore {

WebCLImage::~WebCLImage()
{
}

PassRefPtr<WebCLImage> WebCLImage::create(WebCL* compute_context, cl_mem image, bool is_shared = false)
{
	return adoptRef(new WebCLImage(compute_context, image, is_shared));
}

WebCLImage::WebCLImage(WebCL* compute_context, cl_mem image, bool is_shared)
           :WebCLMemoryObject(compute_context,image,is_shared)
{
}

cl_mem WebCLImage::getCLImage()
{
	return m_cl_mem;
}

int WebCLImage::getGLtextureInfo(int paramNameobj, ExceptionCode& ec)
{
      cl_int err = 0;
      
      if (m_cl_mem == NULL) {
		printf("Error: Invalid CL Memory Object \n");
		ec = WebCLException::INVALID_MEM_OBJECT;
		return NULL;
	  }
	  cl_int int_units = 0;
	  
	  switch(paramNameobj)
	  {
            case WebCL::TEXTURE_TARGET:
			err = clGetGLTextureInfo(m_cl_mem, CL_GL_TEXTURE_TARGET, sizeof(cl_int), &int_units, NULL);
			if (err == CL_SUCCESS)
			return ((int)int_units);
			break;
		
            case WebCL::MIPMAP_LEVEL:
			err = clGetGLTextureInfo(m_cl_mem, CL_GL_MIPMAP_LEVEL, sizeof(cl_int), &int_units, NULL);
			if (err == CL_SUCCESS)
			return ((int)int_units);
			break;
			
	     default:
			printf("Error: Unsupported paramName Info type = %d ",paramNameobj);
			return (NULL);
                          
      }
      if(err != CL_SUCCESS)
	  {
        	switch (err) {
        		case CL_INVALID_MEM_OBJECT:
        			ec = WebCLException::INVALID_MEM_OBJECT;
        			printf("Error: CL_INVALID_MEM_OBJECT  \n");
        			break;
        		case CL_INVALID_GL_OBJECT:
        			ec = WebCLException::INVALID_GL_OBJECT;
        			printf("Error: CL_INVALID_GL_OBJECT \n");
        			break;
        		case CL_INVALID_VALUE:
        			ec = WebCLException::INVALID_VALUE;
        			printf("Error: CL_INVALID_VALUE \n");
        			break;
       			case CL_OUT_OF_RESOURCES:
        			ec = WebCLException::OUT_OF_RESOURCES;
        			printf("Error: CL_OUT_OF_RESOURCES \n");
        			break;
        		case CL_OUT_OF_HOST_MEMORY:
        			ec = WebCLException::OUT_OF_HOST_MEMORY;
        			printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
        			break;
        		default:
        			ec = WebCLException::FAILURE;
        			printf("Invaild Error Type\n");
        			break;
        	}
	  }
      return (NULL);
}

PassRefPtr<WebCLImageDescriptor> WebCLImage::getInfo(ExceptionCode& ec)
{
      cl_int err = 0;

      if (m_cl_mem == NULL) {
		printf("Error: Invalid CL Context\n");
		ec = WebCLException::INVALID_MEM_OBJECT;
		return NULL;
	  }
      cl_int int_units = 0;
      long channelOrder;
      long channelType;
      long width;
      long height;
      long rowPitch;
      int iflag = 0;

      PassRefPtr<WebCLImageDescriptor> objectWebCLImageDescriptor = WebCLImageDescriptor::create();

      err = clGetImageInfo (m_cl_mem,CL_IMAGE_FORMAT,sizeof(cl_int), &int_units, NULL);
      if(err == CL_SUCCESS )
      {
             channelOrder = (long)int_units;
             iflag = 1;
      }

      err = clGetImageInfo (m_cl_mem,CL_IMAGE_ELEMENT_SIZE,sizeof(cl_int), &int_units, NULL);
      if(err == CL_SUCCESS && iflag ==1)
      {
             channelType = (long)int_units;
      }
      else
	  {
		  iflag = 0;
	  }

      err = clGetImageInfo (m_cl_mem,CL_IMAGE_WIDTH,sizeof(cl_int), &int_units, NULL);
      if(err == CL_SUCCESS && iflag ==1 )
      {
             width = (long)int_units;
      }
      else
      {
    	  iflag = 0;
      }

      err = clGetImageInfo (m_cl_mem,CL_IMAGE_HEIGHT,sizeof(cl_int), &int_units, NULL);
      if(err == CL_SUCCESS && iflag ==1)
      {
           height = (long)int_units;
      }
      else
	  {
		  iflag = 0;
	  }

      err = clGetImageInfo (m_cl_mem,CL_IMAGE_ROW_PITCH,sizeof(cl_int), &int_units, NULL);
      if(err == CL_SUCCESS && iflag ==1)
      {
          rowPitch = (long)int_units;
      }
      else
	  {
		  iflag = 0;
	  }


      if(err == CL_SUCCESS && iflag ==1)
      {
    	  objectWebCLImageDescriptor->setChannelOrder(channelOrder);
    	  objectWebCLImageDescriptor->setChannelType(channelType);
    	  objectWebCLImageDescriptor->setWidth(width);
    	  objectWebCLImageDescriptor->setHeight(height);
    	  objectWebCLImageDescriptor->setRowPitch(rowPitch);
    	  return (objectWebCLImageDescriptor);
      }
      else
      {
    	  switch (err) {
				case CL_INVALID_MEM_OBJECT:
					ec = WebCLException::INVALID_MEM_OBJECT;
					printf("Error: CL_INVALID_MEM_OBJECT  \n");
					break;
				case CL_INVALID_GL_OBJECT:
					ec = WebCLException::INVALID_GL_OBJECT;
					printf("Error: CL_INVALID_GL_OBJECT \n");
					break;
				case CL_INVALID_VALUE:
					ec = WebCLException::INVALID_VALUE;
					printf("Error: CL_INVALID_VALUE \n");
					break;
					case CL_OUT_OF_RESOURCES:
					ec = WebCLException::OUT_OF_RESOURCES;
					printf("Error: CL_OUT_OF_RESOURCES \n");
					break;
				case CL_OUT_OF_HOST_MEMORY:
					ec = WebCLException::OUT_OF_HOST_MEMORY;
					printf("Error: CL_OUT_OF_HOST_MEMORY  \n");
					break;
				default:
					ec = WebCLException::FAILURE;
					printf("Invaild Error Type\n");
					break;

    	  }
      }
	return(objectWebCLImageDescriptor);
}
} // namespace WebCore

#endif // ENABLE(WEBCL)
