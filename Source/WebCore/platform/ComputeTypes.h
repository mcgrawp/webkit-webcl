#ifndef ComputeTypes_h
#define ComputeTypes_h

#include <OpenCL/opencl.h>

typedef unsigned int CCenum;
typedef cl_int CCint;
typedef cl_uint CCuint;
typedef cl_mem PlatformComputeObject;
typedef cl_platform_id CCPlatformID;
typedef cl_device_type CCDeviceType;
typedef cl_device_id CCDeviceID;
typedef cl_context_properties CCContextProperties;
typedef cl_image_format CCImageFormat;
typedef cl_sampler CCSampler;
typedef cl_command_queue CCCommandQueue;
typedef cl_program CCProgram;
typedef cl_kernel_work_group_info CCKernelWorkGroupInfo;
typedef cl_kernel CCKernel;

#endif
