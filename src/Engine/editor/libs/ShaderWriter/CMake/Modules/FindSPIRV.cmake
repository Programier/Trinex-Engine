# Find SPIRV
# ----------
#
# Finds the SPIRV library.
#
# This module defines:
#
# ::
#
#   SPIRV_FOUND        - True if SPIRV library is found.
#   SPIRV_INCLUDE_DIR  - The SPIRV include directiories.
#   SPIRV_LIBRARIES    - The SPIRV libraries.
#
# COMPONENTS directive is supported, the supported components are:
#
#   SPIRV-Tools
#   SPIRV-Tools-opt
#   SPIRV-Tools-shared
#   SPIRV-Tools-link
#   SPIRV-Tools-comp
#

find_package( PackageHandleStandardArgs )

set( PLATFORM "x86" )
set( SDK_PLATFORM "32" )
if ( MSVC )
	if( (CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES Win64) )
		set( PLATFORM "x64" )
		set( SDK_PLATFORM "" )
	endif()
else()
	if( (${CMAKE_SIZEOF_VOID_P} EQUAL 8) AND NOT MINGW )
		set( PLATFORM "x64" )
		set( SDK_PLATFORM "" )
	endif()
endif()

if ( NOT SPIRV_FIND_COMPONENTS )
	set( SPIRV_FIND_COMPONENTS SPIRV-Tools SPIRV-Tools-opt )
endif ()

find_path( SPIRV_ROOT_DIR include/SPIRV/spirv.hpp include/vulkan/spirv.hpp
	HINTS
	PATHS
		/usr/local
		/usr
)

if ( SPIRV_ROOT_DIR )
	find_path( SPIRV_INCLUDE_DIR SPIRV/spirv.hpp vulkan/spirv.hpp
		HINTS
		PATH_SUFFIXES
			include
		PATHS
			${SPIRV_ROOT_DIR}
			/usr/local/include
			/usr/include
	)

	find_path( SPIRV_LIBRARY_RELEASE_DIR SPIRV.lib libSPIRV.a
		HINTS
		PATH_SUFFIXES
			lib/${PLATFORM}/Release
			lib/${PLATFORM}
			lib/Release
			lib
			Lib${SDK_PLATFORM}
			/usr/local/lib
			/usr/lib
		PATHS
			${SPIRV_ROOT_DIR}
	)

	find_path( SPIRV_LIBRARY_DEBUG_DIR SPIRVd.lib libSPIRVd.a
		HINTS
		PATH_SUFFIXES
			lib/${PLATFORM}/Debug
			lib/${PLATFORM}
			lib/Debug
			lib
			Lib${SDK_PLATFORM}
			/usr/local/lib
			/usr/lib
		PATHS
			${SPIRV_ROOT_DIR}
	)

	if ( NOT SPIRV_LIBRARY_DEBUG_DIR )
		find_path( SPIRV_LIBRARY_DEBUG_DIR SPIRV.lib libSPIRV.a
			HINTS
			PATH_SUFFIXES
				lib/${PLATFORM}/Debug
				lib/${PLATFORM}
				/usr/local/lib
				/usr/lib
			PATHS
				${SPIRV_ROOT_DIR}
		)
	endif ()

	find_library( SPIRV_LIBRARY_RELEASE
		NAMES
			SPIRV
		HINTS
		PATHS
			${SPIRV_LIBRARY_RELEASE_DIR}
	)

	find_library( SPIRV_LIBRARY_DEBUG
		NAMES
			SPIRVd
			SPIRV
		HINTS
		PATHS
			${SPIRV_LIBRARY_DEBUG_DIR}
	)

	mark_as_advanced( SPIRV_LIBRARY_RELEASE )
	mark_as_advanced( SPIRV_LIBRARY_DEBUG )
	mark_as_advanced( SPIRV_ROOT_DIR )
	find_package_handle_standard_args( SPIRV DEFAULT_MSG SPIRV_LIBRARY_RELEASE SPIRV_LIBRARY_DEBUG SPIRV_INCLUDE_DIR )

	if ( SPIRV_FOUND )
		if ( SPIRV_LIBRARY_DEBUG )
			set( SPIRV_LIBRARIES
				optimized ${SPIRV_LIBRARY_RELEASE}
				debug ${SPIRV_LIBRARY_DEBUG}
				CACHE STRING "SPIRV libraries" FORCE
			)
		else ()
			set( SPIRV_LIBRARIES
				${SPIRV_LIBRARY_RELEASE}
				CACHE STRING "SPIRV libraries" FORCE
			)
		endif ()
		foreach( COMPONENT ${SPIRV_FIND_COMPONENTS} )
			find_library( SPIRV_${COMPONENT}_LIBRARY_RELEASE
				NAMES
					${COMPONENT}
				HINTS
				PATHS
					${SPIRV_LIBRARY_RELEASE_DIR}
			)

			find_library( SPIRV_${COMPONENT}_LIBRARY_DEBUG
				NAMES
					${COMPONENT}d
					${COMPONENT}
				HINTS
				PATHS
					${SPIRV_LIBRARY_DEBUG_DIR}
			)
			mark_as_advanced( SPIRV_${COMPONENT}_LIBRARY_RELEASE )
			mark_as_advanced( SPIRV_${COMPONENT}_LIBRARY_DEBUG )
			find_package_handle_standard_args( SPIRV_${COMPONENT} DEFAULT_MSG SPIRV_${COMPONENT}_LIBRARY_RELEASE SPIRV_${COMPONENT}_LIBRARY_DEBUG )

			if ( SPIRV_${COMPONENT}_FOUND )
				if ( SPIRV_${COMPONENT}_LIBRARY_DEBUG )
					set( SPIRV_${COMPONENT}_LIBRARIES
						optimized ${SPIRV_${COMPONENT}_LIBRARY_RELEASE}
						debug ${SPIRV_${COMPONENT}_LIBRARY_DEBUG}
						CACHE STRING "SPIRV ${COMPONENT} library" FORCE
					)
				else ()
					set( SPIRV_${COMPONENT}_LIBRARIES
						${SPIRV_${COMPONENT}_LIBRARY_RELEASE}
						CACHE STRING "SPIRV ${COMPONENT} library" FORCE
					)
				endif ()
				mark_as_advanced( SPIRV_${COMPONENT}_LIBRARIES )
				set( SPIRV_LIBRARIES
					${SPIRV_LIBRARIES}
					${SPIRV_${COMPONENT}_LIBRARIES}
					CACHE STRING "SPIRV libraries" FORCE
				)
			endif ()

			if ( SPIRV_FOUND AND NOT SPIRV_${COMPONENT}_FOUND )
				set( SPIRV_FOUND FALSE )
			endif ()
		endforeach ()
	endif ()
	if ( SPIRV_FOUND )
		set( SPIRV_INCLUDE_DIR "${SPIRV_INCLUDE_DIR}" CACHE STRING "SPIRV include directories" FORCE )
		unset( SPIRV_LIBRARY_RELEASE_DIR CACHE )
		unset( SPIRV_LIBRARY_DEBUG_DIR CACHE )

		set( SPIRV_REVISION_H ${SPIRV_INCLUDE_DIR}/SPIRV/spirv.hpp )
		file( STRINGS "${SPIRV_REVISION_H}" SPV_VERSION_STR REGEX "^#[\t ]*define[\t ]+SPV_VERSION[\t ]+0x[0-9][0-9][0-9]+$" )
		file( STRINGS "${SPIRV_REVISION_H}" SPV_REVISION_STR REGEX "^#[\t ]*define[\t ]+SPV_REVISION[\t ]+[0-9]+$" )
		unset( SPIRV_VERSION CACHE )
		if ( SPV_VERSION_STR MATCHES "^#[\t ]*define[\t ]+SPV_VERSION[\t ]+0x[0-9][0-9][0-9]+$" )
			if ( SPV_REVISION_STR MATCHES "^#[\t ]*define[\t ]+SPV_REVISION[\t ]+[0-9]+$" )
				string( REGEX REPLACE "^#[\t ]*define[\t ]+SPV_VERSION[\t ]+0x([0-9][0-9])[0-9]+$" "\\1" SPV_VERSION_MAJOR_STR "${SPV_VERSION_STR}" )
				string( REGEX REPLACE "^#[\t ]*define[\t ]+SPV_VERSION[\t ]+0x[0-9][0-9]([0-9]+)$" "\\1" SPV_VERSION_MINOR_STR "${SPV_VERSION_STR}" )
				string( REGEX REPLACE "^#[\t ]*define[\t ]+SPV_REVISION[\t ]+([0-9]+)$" "\\1" SPV_REVISION_STR "${SPV_REVISION_STR}" )
				set( SPIRV_VERSION "${SPV_VERSION_MAJOR_STR}.${SPV_VERSION_MINOR_STR}.${SPV_REVISION_STR}" CACHE STRING "SPIR-V revision string" FORCE )
				unset( SPV_VERSION_STR )
				unset( SPV_REVISION_STR )
			endif ()
		endif ()
	endif ()
else ()
	set( SPIRV_FOUND FALSE )
endif ()

