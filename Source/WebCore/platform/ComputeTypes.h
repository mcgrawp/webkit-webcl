#ifndef ComputeTypes_h
#define ComputeTypes_h

#include <OpenCL/opencl.h>

typedef unsigned int CCenum;
typedef int CCerror;

typedef cl_int CCint;
typedef cl_bool CCbool;
typedef cl_uint CCuint;
typedef cl_ulong CCulong;
typedef cl_mem PlatformComputeObject;
typedef cl_platform_id CCPlatformID;
typedef cl_device_type CCDeviceType;
typedef cl_device_id CCDeviceID;
typedef cl_context_properties CCContextProperties;
typedef cl_image_format CCImageFormat;
typedef cl_sampler CCSampler;
typedef cl_command_queue CCCommandQueue;
typedef cl_command_queue_properties CCCommandQueueProperties;
typedef cl_program CCProgram;
typedef cl_kernel_work_group_info CCKernelWorkGroupInfo;
typedef cl_kernel CCKernel;
typedef cl_event CCEvent;
typedef cl_buffer_region CCBufferRegion;
typedef cl_mem_object_type CCMemObjectype;
typedef cl_mem_flags CCMemFlags;
typedef cl_device_mem_cache_type CCDeviceMemCachetype;
typedef cl_device_exec_capabilities CCDeviceExecCapabilities;
typedef cl_device_local_mem_type CCDeviceLocalMemType;
typedef cl_device_fp_config CCDeviceFPConfig;
typedef cl_build_status CCBuildStatus;
typedef cl_context CCContext;

#endif
