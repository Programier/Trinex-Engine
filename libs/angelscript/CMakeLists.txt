cmake_minimum_required(VERSION 3.5)

option(AS_NO_EXCEPTIONS "Disable exception handling in script context" OFF)

set(ARCHFLAGS "${CMAKE_SYSTEM_PROCESSOR}")

if (APPLE AND NOT IOS)
  if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
    set(ARCHFLAGS "aarch64")
  endif ()
endif ()

# find_package(Threads)

set(ANGELSCRIPT_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_atomic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_builder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_bytecode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_mips.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_x86.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_x64_gcc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_x64_msvc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_x64_mingw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_compiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_configgroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_datatype.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_gc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_generic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_globalproperty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_memory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_module.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_objecttype.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_outputbuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_restore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_scriptcode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_scriptengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_scriptfunction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_scriptnode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_scriptobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_string_util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_tokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_typeinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/as_variablescope.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptmath/scriptmath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptmath/scriptmathcomplex.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptdictionary/scriptdictionary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scripthelper/scripthelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scripthandle/scripthandle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptany/scriptany.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptarray/scriptarray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptgrid/scriptgrid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/weakref/weakref.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/datetime/datetime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/debugger/debugger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/serializer/serializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/contextmgr/contextmgr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptstdstring/scriptstdstring_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptstdstring/scriptstdstring.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptbuilder/scriptbuilder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptfile/scriptfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/add_on/scriptfile/scriptfilesystem.cpp
)

if(MSVC AND CMAKE_CL_64)
    enable_language(ASM_MASM)
    if(CMAKE_ASM_MASM_COMPILER_WORKS)
        set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_x64_msvc_asm.asm)
    else()
        message(FATAL ERROR "MSVC x86_64 target requires a working assembler")
    endif()
endif()

if(${ARCHFLAGS} MATCHES "^arm")
    enable_language(ASM)
    if(CMAKE_ASM_COMPILER_WORKS)
        set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_arm.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_arm_gcc.S)
        set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_arm_gcc.S APPEND PROPERTY COMPILE_FLAGS " -Wa,-mimplicit-it=always")
    else()
        message(FATAL ERROR "ARM target requires a working assembler")
    endif()
endif()

if(${ARCHFLAGS} MATCHES "^aarch64")
    enable_language(ASM)
    if(CMAKE_ASM_COMPILER_WORKS)
        if(NOT APPLE)
            set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_arm64.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_arm64_gcc.S)
        else()
            set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_arm64.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source/as_callfunc_arm64_xcode.S)
        endif()
    else()
        message(FATAL ERROR "ARM target requires a working assembler")
    endif()
endif()

add_trinex_sources(${ANGELSCRIPT_SOURCE})
add_trinex_private_dependencies(-DANGELSCRIPT_EXPORT -D_LIB)

if(AS_NO_EXCEPTIONS)
        add_trinex_private_dependencies(AS_NO_EXCEPTIONS)
endif()

add_trinex_private_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source/)
add_trinex_public_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
