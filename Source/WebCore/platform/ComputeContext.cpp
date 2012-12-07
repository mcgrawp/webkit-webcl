#include "ComputeContext.h"

#include <wtf/text/CString.h>
#include <wtf/text/WTFString.h>

// FIXME: it is really necessary?
#define TO_CL_BOOL(var) var ? CL_TRUE : CL_FALSE

namespace WebCore {

static int clToComputeContextError(CCint clError)
{
    int computeContextError;
    switch (clError) {
    case CL_SUCCESS:
        computeContextError = ComputeContext::SUCCESS;
        break;
    case CL_INVALID_PLATFORM:
        computeContextError = ComputeContext::INVALID_PLATFORM;
        break;
    case CL_INVALID_DEVICE:
        computeContextError = ComputeContext::INVALID_DEVICE;
        break;
    case CL_INVALID_OPERATION:
        computeContextError = ComputeContext::INVALID_OPERATION;
        break;
    case CL_DEVICE_NOT_AVAILABLE:
        computeContextError = ComputeContext::DEVICE_NOT_AVAILABLE;
        break;
    case CL_OUT_OF_RESOURCES:
        computeContextError = ComputeContext::OUT_OF_RESOURCES;
        break;
    case CL_OUT_OF_HOST_MEMORY:
        computeContextError = ComputeContext::OUT_OF_HOST_MEMORY;
        break;
    case CL_INVALID_CONTEXT:
        computeContextError = ComputeContext::INVALID_CONTEXT;
        break;
    case CL_INVALID_VALUE:
        computeContextError = ComputeContext::INVALID_VALUE;
        break;
    case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
        computeContextError = ComputeContext::INVALID_IMAGE_FORMAT_DESCRIPTOR;
        break;
    case CL_INVALID_IMAGE_SIZE:
        computeContextError = ComputeContext::INVALID_IMAGE_SIZE;
        break;
    case CL_INVALID_HOST_PTR:
        computeContextError = ComputeContext::INVALID_HOST_PTR;
        break;
    case CL_IMAGE_FORMAT_NOT_SUPPORTED:
        computeContextError = ComputeContext::IMAGE_FORMAT_NOT_SUPPORTED;
        break;
    case CL_MEM_OBJECT_ALLOCATION_FAILURE:
        computeContextError = ComputeContext::MEM_OBJECT_ALLOCATION_FAILURE;
        break;
    case CL_INVALID_PROGRAM:
        computeContextError = ComputeContext::INVALID_PROGRAM;
        break;
    case CL_INVALID_PROGRAM_EXECUTABLE:
        computeContextError = ComputeContext::INVALID_PROGRAM_EXECUTABLE;
        break;
    case CL_INVALID_KERNEL_NAME:
        computeContextError = ComputeContext::INVALID_KERNEL_NAME;
        break;
    case CL_INVALID_KERNEL_DEFINITION:
        computeContextError = ComputeContext::INVALID_KERNEL_DEFINITION;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return computeContextError;
}

static cl_image_format computeImageFormatToCL(const ComputeContext::ImageFormat& imageFormat)
{
    cl_image_format clImageFormat;
    switch (imageFormat.channelOrder) {
    case ComputeContext::R:
        clImageFormat.image_channel_order = CL_R;
        break;
    case ComputeContext::A:
        clImageFormat.image_channel_order = CL_A;
        break;
    case ComputeContext::RG:
        clImageFormat.image_channel_order = CL_RG;
        break;
    case ComputeContext::RA:
        clImageFormat.image_channel_order = CL_RA;
        break;
    case ComputeContext::RGB:
        clImageFormat.image_channel_order = CL_RGB;
        break;
    case ComputeContext::RGBA:
        clImageFormat.image_channel_order = CL_RGBA;
        break;
    case ComputeContext::BGRA:
        clImageFormat.image_channel_order = CL_BGRA;
        break;
    case ComputeContext::ARGB:
        clImageFormat.image_channel_order = CL_ARGB;
        break;
    case ComputeContext::INTENSITY:
        clImageFormat.image_channel_order = CL_INTENSITY;
        break;
    case ComputeContext::LUMINANCE:
        clImageFormat.image_channel_order = CL_LUMINANCE;
        break;
    case ComputeContext::Rx:
        clImageFormat.image_channel_order = CL_Rx;
        break;
    case ComputeContext::RGx:
        clImageFormat.image_channel_order = CL_RGx;
        break;
    case ComputeContext::RGBx:
        clImageFormat.image_channel_order = CL_RGBx;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    switch (imageFormat.channelDataType) {
    case ComputeContext::SNORM_INT8:
        clImageFormat.image_channel_data_type = CL_SNORM_INT8;
        break;
    case ComputeContext::SNORM_INT16:
        clImageFormat.image_channel_data_type = CL_SNORM_INT16;
        break;
    case ComputeContext::UNORM_INT8:
        clImageFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
    case ComputeContext::UNORM_INT16:
        clImageFormat.image_channel_data_type = CL_UNORM_INT16;
        break;
    case ComputeContext::UNORM_SHORT_565:
        clImageFormat.image_channel_data_type = CL_UNORM_SHORT_565;
        break;
    case ComputeContext::UNORM_SHORT_555:
        clImageFormat.image_channel_data_type = CL_UNORM_SHORT_555;
        break;
    case ComputeContext::UNORM_INT_101010:
        clImageFormat.image_channel_data_type = CL_UNORM_INT_101010;
        break;
    case ComputeContext::SIGNED_INT8:
        clImageFormat.image_channel_data_type = CL_SIGNED_INT8;
        break;
    case ComputeContext::SIGNED_INT16:
        clImageFormat.image_channel_data_type = CL_SIGNED_INT16;
        break;
    case ComputeContext::SIGNED_INT32:
        clImageFormat.image_channel_data_type = CL_SIGNED_INT32;
        break;
    case ComputeContext::UNSIGNED_INT8:
        clImageFormat.image_channel_data_type = CL_UNSIGNED_INT8;
        break;
    case ComputeContext::UNSIGNED_INT16:
        clImageFormat.image_channel_data_type = CL_UNSIGNED_INT16;
        break;
    case ComputeContext::UNSIGNED_INT32:
        clImageFormat.image_channel_data_type = CL_UNSIGNED_INT32;
        break;
    case ComputeContext::HALF_FLOAT:
        clImageFormat.image_channel_data_type = CL_HALF_FLOAT;
        break;
    case ComputeContext::FLOAT:
        clImageFormat.image_channel_data_type = CL_FLOAT;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clImageFormat;
}

static cl_mem_flags computeMemoryTypeToCL(int memoryType)
{
    cl_int clMemoryType;
    switch (memoryType) {
    case ComputeContext::MEM_READ_ONLY:
        clMemoryType = CL_MEM_READ_ONLY;
        break;
    case ComputeContext::MEM_WRITE_ONLY:
        clMemoryType = CL_MEM_WRITE_ONLY;
        break;
    case ComputeContext::MEM_READ_WRITE:
        clMemoryType = CL_MEM_READ_WRITE;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clMemoryType;
}

ComputeContext::ComputeContext(CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID* devices, int* error)
{
    CCint clError;
    m_clContext = clCreateContext(contextProperties, numberDevices, devices, NULL, NULL, &clError);

    if (clError == CL_SUCCESS)
        return;

    int computeContextError = clToComputeContextError(clError);
    if (error)
        *error = computeContextError;
}

ComputeContext::ComputeContext(CCContextProperties* contextProperties, unsigned int deviceType, int* error)
{
    CCint clError;

    switch (deviceType) {
    case DEVICE_TYPE_GPU:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL, NULL, &clError);
        break;
    case DEVICE_TYPE_CPU:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_CPU, NULL, NULL, &clError);
        break;
    case DEVICE_TYPE_ACCELERATOR:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ACCELERATOR, NULL, NULL, &clError);
        break;
    case DEVICE_TYPE_DEFAULT:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &clError);
        break;
    case DEVICE_TYPE_ALL:
        m_clContext = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_ALL, NULL, NULL, &clError);
        break;
    default:
        clError = CL_INVALID_DEVICE_TYPE;
        break;
    }

    int computeContextError = clToComputeContextError(clError);
    if (error)
        *error = computeContextError;
}

ComputeContext::~ComputeContext()
{
    clReleaseContext(m_clContext);
}

PassRefPtr<ComputeContext> ComputeContext::create(CCContextProperties* contextProperties, CCuint numberDevices, CCDeviceID* devices, int* error)
{
    return adoptRef(new ComputeContext(contextProperties, numberDevices, devices, error));
}

PassRefPtr<ComputeContext> ComputeContext::create(CCContextProperties* contextProperties, unsigned int deviceType, int* error)
{
    return adoptRef(new ComputeContext(contextProperties, deviceType, error));
}

CCint ComputeContext::platformIDs(CCuint numberEntries, CCPlatformID* platforms)
{
    cl_uint numberOfPlatforms;
    const cl_int result = clGetPlatformIDs(numberEntries, platforms, &numberOfPlatforms);

    return result == CL_SUCCESS ? numberOfPlatforms : -1;
}

CCint ComputeContext::deviceIDs(CCPlatformID platform, CCDeviceType deviceType, CCuint numberEntries, CCDeviceID* devices)
{
    cl_uint numberOfDevices;
    const cl_int result = clGetDeviceIDs(platform, deviceType, numberEntries, devices, &numberOfDevices);

    return result == CL_SUCCESS ? numberOfDevices : -1;
}

CCCommandQueue ComputeContext::createCommandQueue(CCDeviceID deviceId, int properties, int& error)
{
    cl_command_queue_properties clProperties;
    switch (properties) {
    case QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE:
        clProperties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE;
        break;
    case QUEUE_PROFILING_ENABLE:
        clProperties = CL_QUEUE_PROFILING_ENABLE;
        break;
    default:
        //FIXME 
        clProperties = 0;
        break;
    }

    cl_int clError;
    cl_command_queue clCommandQueue = clCreateCommandQueue(m_clContext, deviceId, clProperties, &clError);

    error = clToComputeContextError(clError);
    return clCommandQueue;
}

CCProgram ComputeContext::createProgram(const String& kernelSource, int& error)
{
    cl_program clProgram;
    cl_int clError;
    const char* kernelSourcePtr = kernelSource.utf8().data();

    clProgram = clCreateProgramWithSource(m_clContext, 1, &kernelSourcePtr, NULL, &clError);
    error = clToComputeContextError(clError);

    return clProgram;
}

PlatformComputeObject ComputeContext::createBuffer(int type, size_t size, void* data, int& error)
{
    cl_mem clMemoryBuffer;
    cl_int clError;
    cl_int memoryType = computeMemoryTypeToCL(type);

    clMemoryBuffer = clCreateBuffer(m_clContext, memoryType, size, data, &clError);
    error = clToComputeContextError(clError);

    return clMemoryBuffer;
}

PlatformComputeObject ComputeContext::createImage2D(int type, int width, int height, const ImageFormat& imageFormat, void* data, int& error)
{
    cl_mem clMemoryImage = NULL;
    cl_int memoryImageError = CL_SUCCESS;
    cl_image_format clImageFormat = computeImageFormatToCL(imageFormat);

    // FIXME: verify if CL_MEM_USE_HOST_PTR is necessary
    cl_int memoryType = computeMemoryTypeToCL(type);
    clMemoryImage = clCreateImage2D(m_clContext, memoryType, &clImageFormat, width, height, 0,
                                    data, &memoryImageError);

    error = clToComputeContextError(memoryImageError);
    return clMemoryImage;
}

PlatformComputeObject ComputeContext::createFromGLBuffer(int type, int bufferId, int& error)
{
    cl_mem clMemoryImage = NULL;
    cl_int clError;
    cl_int memoryType = computeMemoryTypeToCL(type);

    clMemoryImage = clCreateFromGLBuffer(m_clContext, memoryType, bufferId, &clError);
    error = clToComputeContextError(clError);

    return clMemoryImage;
}

PlatformComputeObject ComputeContext::createFromGLRenderbuffer(int type, GC3Dint renderbufferId, int& error)
{
    cl_mem clMemory;
    cl_int clError;
    cl_int memoryType = computeMemoryTypeToCL(type);

    clMemory = clCreateFromGLRenderbuffer(m_clContext, memoryType, renderbufferId, &clError);
    error = clToComputeContextError(clError);

    return clMemory;
}

static cl_addressing_mode computeAddressingModeToCL(int addrMode)
{
    cl_addressing_mode clAddrMode;
    switch (addrMode) {
    case ComputeContext::ADDRESS_NONE:
        clAddrMode = CL_ADDRESS_NONE;
        break;
    case ComputeContext::ADDRESS_CLAMP_TO_EDGE:
        clAddrMode = CL_ADDRESS_CLAMP_TO_EDGE;
        break;
    case ComputeContext::ADDRESS_CLAMP:
        clAddrMode = CL_ADDRESS_CLAMP;
        break;
    case ComputeContext::ADDRESS_REPEAT:
        clAddrMode = CL_ADDRESS_REPEAT;
        break;
    case ComputeContext::ADDRESS_MIRRORED_REPEAT:
        clAddrMode = CL_ADDRESS_MIRRORED_REPEAT;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clAddrMode;
}

static cl_filter_mode computeFilterModeToCL(int filterMode)
{
    cl_filter_mode clFilterMode;
    switch (filterMode) {
    case ComputeContext::FILTER_LINEAR:
        clFilterMode = CL_FILTER_LINEAR;
        break;
    case ComputeContext::FILTER_NEAREST :
        clFilterMode = CL_FILTER_NEAREST ;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clFilterMode;
}

CCSampler ComputeContext::createSampler(bool normalizedCoords, int addressingMode, int filterMode, int& error)
{
    cl_addressing_mode clAddressingMode = computeAddressingModeToCL(addressingMode);
    cl_filter_mode clFilterMode = computeFilterModeToCL(filterMode);
    cl_int clError;

    cl_sampler sampler = clCreateSampler(m_clContext, TO_CL_BOOL(normalizedCoords), clAddressingMode, clFilterMode, &clError);
    error = clToComputeContextError(clError);

    return sampler;
}

PlatformComputeObject ComputeContext::createFromGLTexture2D(int type, GC3Denum textureTarget, GC3Dint mipLevel, GC3Duint texture, int& error)
{
    cl_int memoryType = computeMemoryTypeToCL(type);
    cl_int clError;
    PlatformComputeObject memory;

    memory = clCreateFromGLTexture2D(m_clContext, memoryType, textureTarget, mipLevel, texture, &clError);
    error = clToComputeContextError(clError);

    return memory;
}

static cl_mem_object_type computeObjectTypeToCL(int type)
{
    cl_mem_object_type clObjectType;
    switch(type) {
    case ComputeContext::GL_OBJECT_BUFFER:
        clObjectType = CL_GL_OBJECT_BUFFER;
        break;
    case ComputeContext::GL_OBJECT_TEXTURE2D:
        clObjectType = CL_GL_OBJECT_TEXTURE2D;
        break;
    case ComputeContext::GL_OBJECT_RENDERBUFFER:
        clObjectType = CL_GL_OBJECT_RENDERBUFFER;
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    return clObjectType;
}

// FIXME: improve the API
CCint ComputeContext::supportedImageFormats(int type, int imageType, CCuint numberOfEntries, CCuint *numberImageFormat, CCImageFormat* imageFormat)
{
    cl_mem_flags memoryType = computeMemoryTypeToCL(type);
    cl_mem_object_type clImageType = computeObjectTypeToCL(imageType);

    cl_int error = clGetSupportedImageFormats(m_clContext, memoryType, clImageType, numberOfEntries, imageFormat, numberImageFormat);
    return clToComputeContextError(error);
}

CCint ComputeContext::enqueueNDRangeKernel(CCCommandQueue commandQueue, CCKernel kernelID, int workItemDimensions,
	size_t* globalWorkOffset, size_t* globalWorkSize, size_t* localWorkSize, int eventWaitListLength, CCEvent* eventWaitList, CCEvent* event)
{
    cl_int error = clEnqueueNDRangeKernel(commandQueue, kernelID, workItemDimensions,
        globalWorkOffset, globalWorkSize, localWorkSize, eventWaitListLength, eventWaitList, event);

    return clToComputeContextError(error);
}

CCint ComputeContext::releaseCommandQueue(CCCommandQueue commandQueue)
{
    cl_int error = clReleaseCommandQueue(commandQueue);
    return clToComputeContextError(error);
}

CCKernel ComputeContext::createKernel(CCProgram program, const String& kernelName, int& error)
{
    cl_int clError;
    cl_kernel kernel = clCreateKernel(program, kernelName.utf8().data(), &clError);

    error = clToComputeContextError(clError);

    return kernel;
}

}
