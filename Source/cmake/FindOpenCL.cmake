# - Try to find OpenCL
# This module tries to find an OpenCL implementation on your system. It supports
# AMD / ATI, Apple and NVIDIA implementations, but shoudl work, too.
#
# Once done this will define
#  OPENCL_FOUND        - system has OpenCL
#  OPENCL_INCLUDE_DIRS  - the OpenCL include directory
#  OPENCL_LIBRARIES    - link these to use OpenCL
#
# WIN32 should work, but is untested

find_package(PackageHandleStandardArgs)

set(OPENCL_VERSION_STRING "0.1.0")
set(OPENCL_VERSION_MAJOR 0)
set(OPENCL_VERSION_MINOR 1)
set(OPENCL_VERSION_PATCH 0)

if (APPLE)

  find_library(OPENCL_LIBRARIES OpenCL DOC "OpenCL lib for OSX")
  find_path(OPENCL_INCLUDE_DIRS OpenCL/cl.h DOC "Include for OpenCL on OSX")
  find_path(_OPENCL_CPP_INCLUDE_DIRS OpenCL/cl.hpp DOC "Include for OpenCL CPP bindings on OSX")

else (APPLE)

    if (WIN32)
        find_path(OPENCL_INCLUDE_DIRS CL/cl.h)
        find_path(_OPENCL_CPP_INCLUDE_DIRS CL/cl.hpp)
        # The AMD SDK currently installs both x86 and x86_64 libraries
        # This is only a hack to find out architecture
        if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
            set(OPENCL_LIB_DIR "$ENV{ATISTREAMSDKROOT}/lib/x86_64")
        else (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
            set(OPENCL_LIB_DIR "$ENV{ATISTREAMSDKROOT}/lib/x86")
        endif (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")
        find_library(OPENCL_LIBRARIES OpenCL.lib ${OPENCL_LIB_DIR})
        get_filename_component(_OPENCL_INC_CAND ${OPENCL_LIB_DIR}/../../include ABSOLUTE)
        # On Win32 search relative to the library
        find_path(OPENCL_INCLUDE_DIRS CL/cl.h PATHS "${_OPENCL_INC_CAND}")
        find_path(_OPENCL_CPP_INCLUDE_DIRS CL/cl.hpp PATHS "${_OPENCL_INC_CAND}")
    else (WIN32)
            # Unix style platforms
            find_library(OPENCL_LIBRARIES OpenCL
              ENV LD_LIBRARY_PATH
              /usr/lib/nvidia-current
              /opt/nvidia-current
              /opt/AMDAPP/lib
            )
            get_filename_component(OPENCL_LIB_DIR ${OPENCL_LIBRARIES} PATH)
            get_filename_component(_OPENCL_INC_CAND ${OPENCL_LIB_DIR}/../../include ABSOLUTE)
            # The AMD SDK currently does not place its headers
            # in /usr/include, therefore also search relative
            # to the library
            find_path(OPENCL_INCLUDE_DIRS CL/cl.h PATHS
                ${_OPENCL_INC_CAND}
                /usr/local/cuda/include
                /opt/cuda/include
                /opt/AMDAPP/include)
            find_path(_OPENCL_CPP_INCLUDE_DIRS CL/cl.hpp PATHS ${_OPENCL_INC_CAND})
    endif (WIN32)
endif (APPLE)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenCL DEFAULT_MSG OPENCL_LIBRARIES OPENCL_INCLUDE_DIRS)

if (_OPENCL_CPP_INCLUDE_DIRS)
    set(OPENCL_HAS_CPP_BINDINGS TRUE)
    list(APPEND OPENCL_INCLUDE_DIRS ${_OPENCL_CPP_INCLUDE_DIRS})
    # This is often the same, so clean up
    list(REMOVE_DUPLICATES OPENCL_INCLUDE_DIRS)
endif (_OPENCL_CPP_INCLUDE_DIRS)
mark_as_advanced(
  OPENCL_LIBRARIES
  OPENCL_INCLUDE_DIRS
  _OPENCL_CPP_INCLUDE_DIRS
)
