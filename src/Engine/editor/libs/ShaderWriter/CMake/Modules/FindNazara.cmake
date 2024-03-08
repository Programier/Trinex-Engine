# FindNazara
# ------------
#
# Locate Nazara library
#
# This module defines
#
# ::
#
#   Nazara_LIBRARIES, the libraries to link against
#   Nazara_FOUND, if false, do not try to link to Nazara
#   Nazara_INCLUDE_DIRS, where to find headers.
#

find_package( PackageHandleStandardArgs )

set( _PLATFORM "x86" )
if ( MSVC )
	if( (CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES Win64) )
		set( _PLATFORM "x64" )
	endif()
else()
	if( (${CMAKE_SIZEOF_VOID_P} EQUAL 8) AND NOT MINGW )
		set( _PLATFORM "x64" )
	endif()
endif()

if ( NOT Nazara_FIND_COMPONENTS )
	set( Nazara_FIND_COMPONENTS Audio Core Graphics Lua Network Noise Physics2D Physics3D Platform Renderer SDK Utility )
endif ()

set( Nazara_FOUND TRUE )

set( Nazara_INCLUDE_DIRS "" CACHE STRING "Nazara include directories" FORCE )
set( Nazara_LIBRARIES "" CACHE STRING "Nazara libraries" FORCE )

find_path( Nazara_ROOT_DIR include/Nazara/Prerequesites.hpp
	HINTS
	PATH_SUFFIXES
		Nazara
	PATHS
		/usr/local
		/usr
)

if ( Nazara_ROOT_DIR )
	foreach( COMPONENT ${Nazara_FIND_COMPONENTS} )
		set( ABI_Name )
		set( ABI_Name_Debug "-d" )

		if ( "${COMPONENT}" STREQUAL "SDK" )
			set( COMPONENT_INCLUDE_NAME NDK/Sdk.hpp )
		elseif ( "${COMPONENT}" STREQUAL "SDKServer" )
		else ()
			set( COMPONENT_INCLUDE_NAME Nazara/${COMPONENT}.hpp )
		endif ()

		find_path( Nazara_${COMPONENT}_INCLUDE_DIR ${COMPONENT_INCLUDE_NAME}
			HINTS
			PATHS
				${Nazara_ROOT_DIR}/include
				/usr/local/include
				/usr/include
		)

		set( COMPONENT_LIB_NAME Nazara${COMPONENT} )

		if( MSVC )
			find_path( Nazara_${COMPONENT}_LIBRARY_RELEASE_DIR ${COMPONENT_LIB_NAME}${ABI_Name}.lib
				HINTS
				PATH_SUFFIXES
					lib/${_PLATFORM}
				PATHS
					${Nazara_ROOT_DIR}
			)
			find_path( Nazara_${COMPONENT}_LIBRARY_DEBUG_DIR ${COMPONENT_LIB_NAME}${ABI_Name_Debug}.lib
				HINTS
				PATH_SUFFIXES
					lib/${_PLATFORM}
				PATHS
					${Nazara_ROOT_DIR}
			)

			find_library( Nazara_${COMPONENT}_LIBRARY_RELEASE
				NAMES
					${COMPONENT_LIB_NAME}${ABI_Name}.lib
				HINTS
				PATHS
					${Nazara_${COMPONENT}_LIBRARY_RELEASE_DIR}
			)
			find_library( Nazara_${COMPONENT}_LIBRARY_DEBUG
				NAMES
					${COMPONENT_LIB_NAME}${ABI_Name_Debug}.lib
				HINTS
				PATHS
					${Nazara_${COMPONENT}_LIBRARY_DEBUG_DIR}
			)
		else()
			find_path(Nazara_${COMPONENT}_LIBRARY_RELEASE_DIR lib${COMPONENT_LIB_NAME}${ABI_Name}.so
				HINTS
				PATH_SUFFIXES
					lib64
					lib
					lib/${_PLATFORM}
				PATHS
					${Nazara_${COMPONENT}_ROOT_DIR}
			)
			find_path(Nazara_${COMPONENT}_LIBRARY_DEBUG_DIR lib${COMPONENT_LIB_NAME}${ABI_Name_Debug}.so
				HINTS
				PATH_SUFFIXES
					lib64
					lib
					lib/${_PLATFORM}
				PATHS
					${Nazara_ROOT_DIR}
			)

			find_library( Nazara_${COMPONENT}_LIBRARY_RELEASE
				NAMES
					lib${COMPONENT_LIB_NAME}${ABI_Name}.so
				HINTS
				PATHS
					${Nazara_${COMPONENT}_LIBRARY_RELEASE_DIR}
			)

			find_library(Nazara_${COMPONENT}_LIBRARY_DEBUG
				NAMES
					lib${COMPONENT_LIB_NAME}${ABI_Name_Debug}.so
				HINTS
				PATHS
					${Nazara_${COMPONENT}_LIBRARY_DEBUG_DIR}
			)
		endif()

		mark_as_advanced( Nazara_${COMPONENT}_LIBRARY_RELEASE )
		mark_as_advanced( Nazara_${COMPONENT}_LIBRARY_DEBUG )
		find_package_handle_standard_args( Nazara_${COMPONENT} DEFAULT_MSG Nazara_${COMPONENT}_LIBRARY_RELEASE Nazara_${COMPONENT}_INCLUDE_DIR )

		if ( Nazara_${COMPONENT}_FOUND )
			if (MSVC)
				if ( Nazara_${COMPONENT}_LIBRARY_DEBUG )
					set( Nazara_${COMPONENT}_LIBRARIES optimized ${Nazara_${COMPONENT}_LIBRARY_RELEASE} debug ${Nazara_${COMPONENT}_LIBRARY_DEBUG} CACHE STRING "Nazara ${COMPONENT} library" )
					set( Nazara_${COMPONENT}_LIBRARY_DIRS ${Nazara_${COMPONENT}_LIBRARY_RELEASE_DIR} ${Nazara_${COMPONENT}_LIBRARY_DEBUG_DIR} )
				else ()
					set( Nazara_${COMPONENT}_LIBRARIES ${Nazara_${COMPONENT}_LIBRARY_RELEASE} CACHE STRING "Nazara ${COMPONENT} library" )
					set( Nazara_${COMPONENT}_LIBRARY_DIRS ${Nazara_${COMPONENT}_LIBRARY_RELEASE_DIR})
				endif ()
			else ()
				if ( Nazara_${COMPONENT}_LIBRARY_DEBUG )
					set( Nazara_${COMPONENT}_LIBRARIES optimized ${Nazara_${COMPONENT}_LIBRARY_RELEASE} debug ${Nazara_${COMPONENT}_LIBRARY_DEBUG} CACHE STRING "Nazara ${COMPONENT} library" )
					set( Nazara_${COMPONENT}_LIBRARY_DIRS ${Nazara_${COMPONENT}_LIBRARY_RELEASE_DIR} ${Nazara_${COMPONENT}_LIBRARY_DEBUG_DIR} )
				else ()
					set( Nazara_${COMPONENT}_LIBRARIES ${Nazara_${COMPONENT}_LIBRARY_RELEASE} CACHE STRING "Nazara ${COMPONENT} library" )
					set( Nazara_${COMPONENT}_LIBRARY_DIRS ${Nazara_${COMPONENT}_LIBRARY_RELEASE_DIR})
				endif ()
			endif ()
			set( Nazara_INCLUDE_DIRS
				${Nazara_INCLUDE_DIRS}
				${Nazara_${COMPONENT}_INCLUDE_DIR}
				CACHE STRING "Nazara include directories" FORCE
			)
			set( Nazara_LIBRARIES
				${Nazara_LIBRARIES}
				${Nazara_${COMPONENT}_LIBRARIES}
				CACHE STRING "Nazara libraries" FORCE
			)
		endif ()

		if ( Nazara_FOUND AND NOT Nazara_${COMPONENT}_FOUND )
			set( Nazara_FOUND FALSE )
		endif ()

		unset( Nazara_${COMPONENT}_LIBRARY_RELEASE_DIR CACHE )
		unset( Nazara_${COMPONENT}_LIBRARY_DEBUG_DIR CACHE )
		unset( Nazara_${COMPONENT}_LIBRARY_DEBUG_DIR CACHE )
	endforeach ()
else ()
	set( Nazara_FOUND FALSE )
endif ()
