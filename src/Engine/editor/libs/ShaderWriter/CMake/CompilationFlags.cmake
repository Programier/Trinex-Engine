set( PROJECTS_COMPILER "Unknown" )

option( PROJECTS_WARNINGS_AS_ERRORS "Turns compiler warnings to errors" OFF )

if ( MSVC AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
	if ( NOT DEFINED PROJECTS_USE_MP )
		option( PROJECTS_USE_MP "Enable multi-processes compilation" ON )
	endif ()
	set( PROJECTS_COMPILER "MSVC" )
	set( PROJECTS_COMPILER_MSVC ON )
elseif ( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
	if ( MSVC )
		set( PROJECTS_COMPILER "Clang-cl" )
		set( PROJECTS_COMPILER_CLANG_CL ON )
	else ()
		set( PROJECTS_COMPILER "Clang" )
	endif ()
	set( PROJECTS_COMPILER_CLANG ON )
elseif ( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
	set( PROJECTS_COMPILER "GCC" )
	set( PROJECTS_COMPILER_GCC ON )
endif ()

if ( WIN32 )
	set( PROJECTS_OS "Windows" )
	set( PROJECTS_OS_WINDOWS ON )
elseif ( ANDROID OR ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
	set( PROJECTS_OS "Android" )
	set( PROJECTS_OS_ANDROID ON )
elseif ( APPLE )
	set( PROJECTS_OS "Apple" )
	set( PROJECTS_OS_APPLE ON )
else ()
	set( PROJECTS_OS "Linux" )
	set( PROJECTS_OS_LINUX ON )
endif ()

string( FIND ${CMAKE_C_COMPILER} "arm" IS_ARM )

if ( IS_ARM GREATER -1 )
	if ( MSVC )
		if( (CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES Win64) )
			set( PROJECTS_PLATFORM "arm64" )
			set( PROJECTS_PLATFORM_ARM64 ON )
		else()
			set( PROJECTS_PLATFORM "arm" )
			set( PROJECTS_PLATFORM_ARM32 ON )
		endif()
	elseif ( ANDROID OR ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
		set( PROJECTS_PLATFORM ${CMAKE_ANDROID_ARCH_ABI} )
	else()
		if( ${CMAKE_SIZEOF_VOID_P} EQUAL 8 )
			set( PROJECTS_PLATFORM "arm64" )
			set( PROJECTS_PLATFORM_ARM64 ON )
		else()
			set( PROJECTS_PLATFORM "arm" )
			set( PROJECTS_PLATFORM_ARM32 ON )
		endif()
	endif()
else ()
	if ( MSVC )
		if( (CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES Win64) )
			set( PROJECTS_PLATFORM "x64" )
			set( PROJECTS_PLATFORM_64 ON )
		else()
			set( PROJECTS_PLATFORM "x86" )
			set( PROJECTS_PLATFORM_32 ON )
		endif()
	elseif ( ANDROID OR ${CMAKE_SYSTEM_NAME} STREQUAL "Android" )
		set( PROJECTS_PLATFORM ${CMAKE_ANDROID_ARCH_ABI} )
		if( ${CMAKE_SIZEOF_VOID_P} EQUAL 8 )
			set( PROJECTS_PLATFORM_ARM64 ON )
		else()
			set( PROJECTS_PLATFORM_ARM32 ON )
		endif()
	else()
		if( ${CMAKE_SIZEOF_VOID_P} EQUAL 8 )
			set( PROJECTS_PLATFORM_64 ON )
			if ( WIN32 )
				set( PROJECTS_PLATFORM "x64" )
			else ()
				set( PROJECTS_PLATFORM "amd64" )
			endif ()
		else()
			set( PROJECTS_PLATFORM "x86" )
			set( PROJECTS_PLATFORM_32 ON )
		endif()
	endif()
endif()

function( add_target_compilation_flags TARGET_NAME ) #ARGS
	get_target_property( TEMP ${TARGET_NAME} COMPILE_FLAGS )

	if ( TEMP STREQUAL "TEMP-NOTFOUND" )
		SET( TEMP "" )
	else ()
		SET( TEMP "${TEMP} " )
	endif ()

	foreach ( ARG ${ARGN} )
		SET( TEMP "${TEMP}${ARG} " )
	endforeach ()

	set_target_properties( ${TARGET_NAME} PROPERTIES COMPILE_FLAGS ${TEMP} )
endfunction( add_target_compilation_flags )

function( compute_compiler_warning_flags C_DEFINITIONS C_FLAGS CXX_DEFINITIONS CXX_FLAGS LNK_FLAGS )
	if ( PROJECTS_COMPILER_GCC )
		set( _C_FLAGS
			-pedantic
			-Wall
			-Warray-bounds
			-Wcast-align
			-Wcast-qual
			-Wconditionally-supported
			-Wconversion
			-Wdisabled-optimization
			-Wdouble-promotion
			-Wextra
			-Wfloat-conversion
			-Wformat-security
			-Wformat=2
			-Wlogical-op
			-Wmissing-declarations
			-Wmissing-include-dirs
			-Wno-double-promotion
			-Wno-undef
			-Wno-unknown-pragmas
			-Wno-unused-parameter
			-Wpacked
			-Wredundant-decls
			-Wstrict-aliasing
			-Wstrict-null-sentinel
			-Wunused-macros
			-Wno-pragmas
		)
		if ( PROJECTS_WARNINGS_AS_ERRORS )
			set( _C_FLAGS
				${_C_FLAGS}
				-Werror
			)
		endif ()
		set( _CXX_FLAGS
			-Wnon-virtual-dtor
			# -Wold-style-cast
			-Wopenmp-simd
			-Woverloaded-virtual
			-Wtrampolines
			-Wuninitialized
			-Wvector-operation-performance
			-Wvla
			-Wzero-as-null-pointer-constant

			-Wno-comment
			-Wno-format-nonliteral
			-Wno-useless-cast
		)
		if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5 )
			set( _C_FLAGS
				${_C_FLAGS}
				-fdiagnostics-color=auto
				-fsized-deallocation
				-Warray-bounds=2
				-Wsized-deallocation
			)
			set( _CXX_FLAGS
				${_CXX_FLAGS}
				-Wformat-signedness
			)
		endif ()
		if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 6 )
			set( _C_FLAGS
				${_C_FLAGS}
				-Wduplicated-cond
				-Wnull-dereference
			)
		endif ()
		if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7 )
			set( _C_FLAGS
				${_C_FLAGS}
				-Walloca
				-Wshadow=compatible-local
			)
			set( _CXX_FLAGS
				${_CXX_FLAGS}
				-Waligned-new
				-Walloc-zero
				-Wformat-overflow
			)
		endif ()
	elseif ( PROJECTS_COMPILER_CLANG )
		set( _C_FLAGS
			-Weverything

			-Wno-covered-switch-default
			-Wno-documentation
			-Wno-documentation-unknown-command
			-Wno-double-promotion
			-Wno-float-equal
			-Wno-gnu-anonymous-struct
			-Wno-missing-braces
			-Wno-mismatched-tags
			-Wno-padded
			-Wno-reserved-id-macro
			-Wno-switch-enum
			-Wno-shadow-field
			-Wno-undef
			-Wno-unknown-pragmas
			-Wno-unused-parameter
			-Wno-unknown-warning-option
			-Wno-extra-semi-stmt
		)
		if ( PROJECTS_WARNINGS_AS_ERRORS )
			set( _C_FLAGS
				${_C_FLAGS}
				-Werror
			)
		endif ()
		set( _CXX_FLAGS
			-Wno-c++98-compat
			-Wno-c++98-compat-pedantic
			-Wno-return-std-move-in-c++11
			-Wno-exit-time-destructors
			-Wno-format-nonliteral
			-Wno-global-constructors
			-Wno-sign-conversion
			-Wno-weak-vtables
			-Wno-weak-vtables
		)
		if ( PROJECTS_COMPILER_CLANG_CL )
			set( _CXX_FLAGS
				${_CXX_FLAGS}
				-Wno-language-extension-token
				-Wno-nonportable-system-include-path
				-Wno-zero-as-null-pointer-constant
			)
		else ()
			set( _C_FLAGS
				${_C_FLAGS}
				-pedantic
			)
		endif ()
		if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 5 )
			set( _CXX_FLAGS
				${_CXX_FLAGS}
				-Wno-unused-template
				-Wno-shadow-field-in-constructor
				-Wno-inconsistent-missing-destructor-override
			)
		endif ()
	elseif ( PROJECTS_COMPILER_MSVC )
		set( _CXX_FLAGS
			/Wall

			/wd4619 # there is no warning number 'XXXX'

			/wd4061 # Enum value in a switch not explicitly handled by a case label
			/wd4068 # Unknown pragma
			/wd4100 # Unused parameter.
			/wd4263 # Member function does not override any base class virtual member function
			/wd4264 # No override available for virtual member function from base 'class'; function is hidden
			/wd4266 # No override available for virtual member function from base 'type'; function is hidden
			/wd4505 # Unreferenced local function has been removed
			/wd4571 # SEH exceptions aren't caught since Visual C++ 7.1
			/wd4623 # Default constructor was implicitly defined as deleted because a base class default constructor is inaccessible or deleted
			/wd4625 # Copy constructor was implicitly defined as deleted because a base class copy constructor is inaccessible or deleted
			/wd4626 # Assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted
			/wd4866 # Compiler may not enforce left-to-right evaluation order for call to <operator_name>
			/wd4868 # Compiler may not enforce left-to-right evaluation order in braced initializer list
			/wd5045 # Spectre mitigation
			/wd5246 # The initialization of a subobject should be wrapped in braces

			# Warnings triggered by MSVC's standard library
			/wd4355 # 'this' used in base member initializing list
			/wd4514 # Unreferenced inline function has been removed
			/wd4548 # Expression before comma has no effect
			/wd4668 # Preprocessor macro not defined
			/wd4710 # Function not inlined
			/wd4711 # Function inlined
			/wd4774 # Format string is not a string literal
			/wd4820 # Added padding to members
			/wd5026 # Move constructor implicitly deleted
			/wd5027 # Move assignment operator implicitly deleted
			/wd5039 # Pointer/ref to a potentially throwing function passed to an 'extern "C"' function (with -EHc)
			/wd5220 # Non-static volatile member doesn't imply non-trivial move/copy ctor/operator=
			/wd5264 # 'const' variable is not used

			/wd4599 # precompiled header argument does not match command line
		)
		if ( PROJECTS_USE_MP )
			set( _CXX_FLAGS
				${_CXX_FLAGS}
				/MP # Enabling multi-processes compilation
			)
		endif ()
		if ( PROJECTS_WARNINGS_AS_ERRORS )
			set( _CXX_FLAGS
				${_CXX_FLAGS}
				/WX
			)
		endif ()
		string( REGEX REPLACE "/Z[iI7]" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" )
		set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI" CACHE STRING "Flags used by the CXX compiler during DEBUG builds." FORCE )
		string( REGEX REPLACE "/W[0-4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" )
		string( REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
		string( REGEX REPLACE "/Zm[0-9]*" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm300" )
	endif ()
	if ( PROJECTS_COMPILER_MSVC OR PROJECTS_COMPILER_CLANG_CL )
		set( _LNK_FLAGS
			${_LNK_FLAGS}
			# LINK Warnings
			/IGNORE:4099 # pdb not found
		)
	endif ()
	set( _CXX_DEFINITIONS ${_C_DEFINITIONS} ${_CXX_DEFINITIONS} )
	set( _CXX_FLAGS ${_C_FLAGS} ${_CXX_FLAGS} )

	set( ${C_DEFINITIONS} "${_C_DEFINITIONS}" PARENT_SCOPE )
	set( ${C_FLAGS} "${_C_FLAGS}" PARENT_SCOPE )
	set( ${CXX_DEFINITIONS} "${_CXX_DEFINITIONS}" PARENT_SCOPE )
	set( ${CXX_FLAGS} "${_CXX_FLAGS}" PARENT_SCOPE )
	set( ${LNK_FLAGS} "${_LNK_FLAGS}" PARENT_SCOPE )
endfunction( compute_compiler_warning_flags )

function( compute_compiler_flags C_DEFINITIONS C_FLAGS CXX_DEFINITIONS CXX_FLAGS LNK_FLAGS )
	compute_compiler_warning_flags( C_WARN_DEFS C_WARN_FLAGS CXX_WARN_DEFS CXX_WARN_FLAGS LNK_WARN_FLAGS )
	if ( PROJECTS_COMPILER_GCC )
		if ( NOT ANDROID )
			set( SSE2_FLAG -msse2 )
		endif ()
		set( _C_FLAGS
			${SSE2_FLAG}
		)
	endif ()
	if ( PROJECTS_COMPILER_MSVC OR PROJECTS_COMPILER_CLANG_CL )
		set( _CXX_FLAGS
			${_CXX_FLAGS}
			/permissive- # Improving standard compliance
			/EHsc # Enabling exceptions
			/utf-8 # Forcing MSVC to actually handle files as UTF-8
		)
		set( _CXX_DEFINITIONS
			${_CXX_DEFINITIONS}
			_CRT_SECURE_NO_WARNINGS
			_SCL_SECURE_NO_WARNINGS
			NOMINMAX # Preventing definitions of min & max macros
			_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING # Ignoring std::codecvt deprecation warnings
		)
	endif ()
	set( _C_DEFINITIONS ${C_WARN_DEFS} ${_C_DEFINITIONS} )
	set( _C_FLAGS ${C_WARN_FLAGS} ${_C_FLAGS} )
	set( _CXX_DEFINITIONS ${CXX_WARN_DEFS} ${_CXX_DEFINITIONS} )
	set( _CXX_FLAGS ${CXX_WARN_FLAGS} ${_CXX_FLAGS} )
	set( _LNK_FLAGS ${LNK_WARN_FLAGS} ${_LNK_FLAGS} )

	set( ${C_DEFINITIONS} "${_C_DEFINITIONS}" PARENT_SCOPE )
	set( ${C_FLAGS} "${_C_FLAGS}" PARENT_SCOPE )
	set( ${CXX_DEFINITIONS} "${_CXX_DEFINITIONS}" PARENT_SCOPE )
	set( ${CXX_FLAGS} "${_CXX_FLAGS}" PARENT_SCOPE )
	set( ${LNK_FLAGS} "${_LNK_FLAGS}" PARENT_SCOPE )
endfunction( compute_compiler_flags )

function( compute_platform_flags C_DEFINITIONS C_FLAGS CXX_DEFINITIONS CXX_FLAGS LNK_FLAGS )
	set( _LNK_FLAGS )
	if ( MSVC )
		if ( PROJECTS_PLATFORM_ARM64 )
			set( _LNK_FLAGS "/MACHINE:ARM64" )
		elseif ( PROJECTS_PLATFORM_ARM32 )
			set( _LNK_FLAGS "/MACHINE:ARM" )
		elseif ( PROJECTS_PLATFORM_64 )
			set( _LNK_FLAGS "/MACHINE:X64" )
		else ()
			set( _LNK_FLAGS "/MACHINE:X86" )
		endif ()
	elseif ( NOT ANDROID )
		if( ${CMAKE_SIZEOF_VOID_P} EQUAL 8 )
			set( _LNK_FLAGS -m64 )
		else()
			set( _LNK_FLAGS -m32 )
		endif()
	endif()
	set( ${C_DEFINITIONS} "${_C_DEFINITIONS}" PARENT_SCOPE )
	set( ${C_FLAGS} "${_C_FLAGS}" PARENT_SCOPE )
	set( ${CXX_DEFINITIONS} "${_CXX_DEFINITIONS}" PARENT_SCOPE )
	set( ${CXX_FLAGS} "${_CXX_FLAGS}" PARENT_SCOPE )
	set( ${LNK_FLAGS} "${_LNK_FLAGS}" PARENT_SCOPE )
endfunction( compute_platform_flags )

function( compute_target_compilation_common_flags TARGET_NAME TARGET_TYPE TARGET_C_DEFINITIONS TARGET_C_FLAGS TARGET_CXX_DEFINITIONS TARGET_CXX_FLAGS TARGET_LNK_FLAGS )
	string( COMPARE EQUAL ${TARGET_TYPE} "dll" IS_DLL )
	string( COMPARE EQUAL ${TARGET_TYPE} "api_dll" IS_API_DLL )
	set( C_TGT_DEFS )
	set( C_TGT_FLAGS )
	set( CXX_TGT_DEFS )
	set( CXX_TGT_FLAGS )
	if ( IS_DLL OR IS_API_DLL )
		set( C_TGT_DEFS
			${TARGET_NAME}_EXPORTS
			${TARGET_NAME}_SHARED
		)
		set( CXX_TGT_DEFS
			${TARGET_NAME}_EXPORTS
			${TARGET_NAME}_SHARED
		)
	endif ()
	set( C_COMPILER_DEFS )
	set( C_COMPILER_FLAGS )
	set( CXX_COMPILER_DEFS )
	set( CXX_COMPILER_FLAGS )
	set( LNK_COMPILER_FLAGS )
	compute_compiler_flags( C_COMPILER_DEFS C_COMPILER_FLAGS CXX_COMPILER_DEFS CXX_COMPILER_FLAGS LNK_COMPILER_FLAGS )
	set( C_PLATFORM_DEFS )
	set( C_PLATFORM_FLAGS )
	set( CXX_PLATFORM_DEFS )
	set( CXX_PLATFORM_FLAGS )
	set( LNK_PLATFORM_FLAGS )
	compute_platform_flags( C_PLATFORM_DEFS C_PLATFORM_FLAGS CXX_PLATFORM_DEFS CXX_PLATFORM_FLAGS LNK_PLATFORM_FLAGS )
	set( C_TGT_DEFS ${C_TGT_DEFS} ${C_COMPILER_DEFS} ${C_PLATFORM_DEFS} )
	set( C_TGT_FLAGS ${C_TGT_FLAGS} ${C_COMPILER_FLAGS} ${C_PLATFORM_FLAGS} )
	set( CXX_TGT_DEFS ${CXX_TGT_DEFS} ${CXX_COMPILER_DEFS} ${CXX_PLATFORM_DEFS} )
	set( CXX_TGT_FLAGS ${CXX_TGT_FLAGS} ${CXX_COMPILER_FLAGS} ${CXX_PLATFORM_FLAGS} )
	set( LNK_TGT_FLAGS ${LNK_COMPILER_FLAGS} ${LNK_PLATFORM_FLAGS} )

	set( ${TARGET_C_DEFINITIONS} "${C_TGT_DEFS}" PARENT_SCOPE )
	set( ${TARGET_C_FLAGS} "${C_TGT_FLAGS}" PARENT_SCOPE )
	set( ${TARGET_CXX_DEFINITIONS} "${CXX_TGT_DEFS}" PARENT_SCOPE )
	set( ${TARGET_CXX_FLAGS} "${CXX_TGT_FLAGS}" PARENT_SCOPE )
	set( ${TARGET_LNK_FLAGS} "${LNK_TGT_FLAGS}" PARENT_SCOPE )
endfunction( compute_target_compilation_common_flags )

function( compute_compilation_flags TARGET_NAME TARGET_TYPE OPT_C_FLAGS OPT_CXX_FLAGS OPT_LINK_FLAGS TARGET_C_FLAGS TARGET_C_DEFS TARGET_CXX_FLAGS TARGET_CXX_DEFS TARGET_LINK_FLAGS )
	string( COMPARE EQUAL ${TARGET_TYPE} "dll" IS_DLL )
	string( COMPARE EQUAL ${TARGET_TYPE} "api_dll" IS_API_DLL )
	set( _OPT_C_FLAGS "${OPT_C_FLAGS}" )
	set( _OPT_CXX_FLAGS "${OPT_CXX_FLAGS}" )
	set( _OPT_LINK_FLAGS "${OPT_LINK_FLAGS}" )
	compute_target_compilation_common_flags( ${TARGET_NAME} ${TARGET_TYPE}
		TGT_C_DEFS
		TGT_C_FLAGS
		TGT_CXX_DEFS
		TGT_CXX_FLAGS
		TGT_LNK_FLAGS
	)
	#We complete C/C++ compilation flags with configuration dependant ones and optional ones
	set( _C_FLAGS ${TGT_C_FLAGS} ${_OPT_C_FLAGS} )
	set( _CXX_FLAGS ${TGT_CXX_FLAGS} ${_OPT_CXX_FLAGS} )
	set( _LINK_FLAGS ${TGT_LNK_FLAGS} ${_OPT_LINK_FLAGS} )

	set( ${TARGET_C_FLAGS} "${_C_FLAGS}" PARENT_SCOPE )
	set( ${TARGET_C_DEFS} "${TGT_C_DEFS}" PARENT_SCOPE )
	set( ${TARGET_CXX_FLAGS} "${_CXX_FLAGS}" PARENT_SCOPE )
	set( ${TARGET_CXX_DEFS} "${TGT_CXX_DEFS}" PARENT_SCOPE )
	set( ${TARGET_LINK_FLAGS} "${_LINK_FLAGS}" PARENT_SCOPE )
endfunction( compute_compilation_flags )

function( target_add_compilation_flags TARGET_NAME )
	compute_compiler_flags( COMP_C_DEFS COMP_C_FLAGS COMP_CXX_DEFS COMP_CXX_FLAGS COMP_LNK_FLAGS )
	compute_platform_flags( PLAT_C_DEFS PLAT_C_FLAGS PLAT_CXX_DEFS PLAT_CXX_FLAGS PLAT_LNK_FLAGS )
	target_compile_definitions( ${TARGET_NAME}
		PRIVATE
			${COMP_C_DEFS}
			${COMP_CXX_DEFS}
			${PLAT_C_DEFS}
			${PLAT_CXX_DEFS}
	)
	target_compile_options( ${TARGET_NAME}
		PRIVATE
			${COMP_C_FLAGS}
			${COMP_CXX_FLAGS}
			${PLAT_C_FLAGS}
			${PLAT_CXX_FLAGS}
	)
	target_link_options( ${TARGET_NAME}
		PRIVATE
			${COMP_LNK_FLAGS}
			${PLAT_LNK_FLAGS}
	)
endfunction( target_add_compilation_flags )
