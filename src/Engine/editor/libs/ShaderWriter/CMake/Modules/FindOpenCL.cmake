# - Try to find OpenCL
# This module tries to find an OpenCL implementation on your system. Currently
# it supports searching system locations or detecting environment variables
# for the following implementations:
#  AMD Advanced Parallel Processing SDK
#  NVIDIA CUDA Toolkit
#  Intel OpenCL SDK
#  Generic system installed version
#  Custom location
#
# To set manually the paths, define these environment or CMake variables:
#  OPENCL_ROOT         - Root path containing include/CL/cl.h
#
# Once done this will define
#  OPENCL_FOUND              - System has an OpenCL library
#  OPENCL_INCLUDE_DIRS       - The OpenCL include directories needed
#  OPENCL_LIBRARIES          - Link libraries needed for OpenCL
#  OPENCL_VERSION_STRING     - Version of OpenCL that was found
#  OPENCL_HAS_CXX            - Whether or not C++ bindings are available
#  OPENCL_CXX_VERSION_STRING - Version of the C++ bindings if available
#  OPENCL_CXX_DEFINITIONS    - Compiler defines needed for the C++ bindings
#                             (May be nexessary if C++ bindings are of a
#                              different version than the C API; i.e OpenCL 1.2
#                              but with C++ bindings for 1.1)
#
macro( msg_debug msg )
#	message( STATUS ${msg} )
endmacro( msg_debug )

if(NOT OPENCL_FOUND)
  include(CheckTypeSize)
  CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P)

  # User specified OpenCL location
  if(OPENCL_ROOT)
    msg_debug("OpenCL: Searching in custom location")
    set(_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
    set(CMAKE_FIND_ROOT_PATH ${OPENCL_ROOT})
    set(_OPENCL_ROOT_OPTS ONLY_CMAKE_FIND_ROOT_PATH NO_DEFAULT_PATH)
  elseif(NOT "$ENV{OPENCL_ROOT}" STREQUAL "")
    msg_debug("OpenCL: Searching in custom location")
    set(_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
    set(CMAKE_FIND_ROOT_PATH $ENV{OPENCL_ROOT})
    set(_OPENCL_ROOT_OPTS ONLY_CMAKE_FIND_ROOT_PATH NO_DEFAULT_PATH)

  # AMD APP SDK
  elseif(NOT "$ENV{AMDAPPSDKROOT}" STREQUAL "")
    msg_debug("OpenCL: Searching for AMD APP SDK")
    set(_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
    set(CMAKE_FIND_ROOT_PATH $ENV{AMDAPPSDKROOT})
    set(_OPENCL_ROOT_OPTS ONLY_CMAKE_FIND_ROOT_PATH NO_DEFAULT_PATH)
    if(SIZEOF_VOID_P EQUAL 4)
      set(_OPENCL_LIB_OPTS PATH_SUFFIXES x86)
    else()
      #set(_OPENCL_LIB_OPTS PATH_SUFFIX x86_64)
      set(_OPENCL_LIB_OPTS PATH_SUFFIXES x86_64)
    endif()

  # NVIDIA CUDA
  elseif(NOT "$ENV{CUDA_PATH}" STREQUAL "")
    msg_debug("OpenCL: Searching for NVIDIA CUDA SDK")
    set(_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
    set(CMAKE_FIND_ROOT_PATH $ENV{CUDA_PATH})
    set(_OPENCL_ROOT_OPTS ONLY_CMAKE_FIND_ROOT_PATH NO_DEFAULT_PATH)
    if(WIN32)
      if(SIZEOF_VOID_P EQUAL 4)
        set(_OPENCL_LIB_OPTS PATH_SUFFIX Win32)
      else()
        set(_OPENCL_LIB_OPTS PATH_SUFFIX Win64)
      endif()
    else()
      if(SIZEOF_VOID_P EQUAL 4)
        set(_OPENCL_LIB_DIR_SUFFIX)
      else()
        set(_OPENCL_LIB_DIR_SUFFIX 64)
      endif()
    endif()

  # Intel OpenCL SDK
  elseif(NOT "$ENV{INTELOCLSDKROOT}" STREQUAL "")
    msg_debug("OpenCL: Searching for Intel OpenCL SDK")
    set(_CMAKE_FIND_ROOT_PATH ${CMAKE_FIND_ROOT_PATH})
    set(CMAKE_FIND_ROOT_PATH $ENV{INTELOCLSDKROOT})
    set(_OPENCL_ROOT_OPTS ONLY_CMAKE_FIND_ROOT_PATH NO_DEFAULT_PATH)
    if(WIN32)
      if(SIZEOF_VOID_P EQUAL 4)
        set(_OPENCL_LIB_OPTS PATH_SUFFIX x86)
      else()
        set(_OPENCL_LIB_OPTS PATH_SUFFIX x64)
      endif()
    else()
      if(SIZEOF_VOID_P EQUAL 4)
        set(_OPENCL_LIB_DIR_SUFFIX)
      else()
        set(_OPENCL_LIB_DIR_SUFFIX 64)
      endif()
    endif()

  # System location
  else()
    msg_debug("OpenCL: Searching in system location")
  endif()

  if(APPLE)
    set(_OPENCL_INCLUDE_BASE OpenCL)
  else()
    set(_OPENCL_INCLUDE_BASE CL)
  endif()

  # Find the headers
  find_path(OPENCL_INCLUDE_DIR ${_OPENCL_INCLUDE_BASE}/cl.h
    PATHS /include
    ${_OPENCL_ROOT_OPTS}
  )
  if(OPENCL_INCLUDE_DIR)
    # Interrogate the C header for version information
    set(CMAKE_REQUIRED_INCLUDES ${OPENCL_INCLUDE_DIR})

    include(CheckSymbolExists)
    foreach(_MINOR_VER 0 1 2 3)
      CHECK_SYMBOL_EXISTS(CL_VERSION_1_${_MINOR_VER} "CL/cl.h" _OPENCL_VER)
      if(_OPENCL_VER)
        set(OPENCL_VERSION_STRING "1.${_MINOR_VER}")
        unset(_OPENCL_VER CACHE)
      else()
        break()
      endif()
    endforeach()

    if(EXISTS ${OPENCL_INCLUDE_DIR}/${_OPENCL_INCLUDE_BASE}/cl.hpp)
      set(OPENCL_HAS_CXX TRUE)

      # Interrogate the C++ header for seperate version information
      file(STRINGS ${OPENCL_INCLUDE_DIR}/${_OPENCL_INCLUDE_BASE}/cl.hpp
        _OPENCL_VER REGEX "version 1\\.[0-3]"
      )
      string(REGEX MATCH "1\\.([0-9])" OPENCL_CXX_VERSION_STRING
        "${_OPENCL_VER}"
      )
      set(_MINOR_VER ${CMAKE_MATCH_1})
      if(OPENCL_CXX_VERSION_STRING VERSION_LESS OPENCL_VERSION_STRING)
        set(OPENCL_CXX_DEFINITIONS -DCL_USE_DEPRECATED_OPENCL_1_${_MINOR_VER}_APIS)
      elseif("${OPENCL_VERSION_STRING}" STREQUAL "")
        set(OPENCL_VERSION_STRING ${OPENCL_CXX_VERSION_STRING})
      endif()
    else()
      set(OPENCL_HAS_CXX FALSE)
    endif()

    unset(CMAKE_REQUIRED_INCLUDES)
  endif()

  # Find the library
  find_library(OPENCL_LIBRARY OpenCL
    PATHS /lib${_OPENCL_LIB_DIR_SUFFIX}
    ${_OPENCL_LIB_OPTS}
    ${_OPENCL_ROOT_OPTS}
  )

  # Restore the original search paths
  set(CMAKE_FIND_ROOT_PATH ${_CMAKE_FIND_ROOT_PATH})

  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenCL
    REQUIRED_VARS OPENCL_INCLUDE_DIR OPENCL_LIBRARY
    VERSION_VAR OPENCL_VERSION_STRING
  )
  if(OPENCL_FOUND)
    set(OPENCL_INCLUDE_DIRS ${OPENCL_INCLUDE_DIR})
    set(OPENCL_LIBRARIES ${OPENCL_LIBRARY})
  endif()
endif()