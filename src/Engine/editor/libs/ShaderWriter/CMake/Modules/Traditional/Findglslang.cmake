# Find glslang
# ------------
#
# Finds the glslang library.
#
# This module defines:
#
# ::
#
#   glslang_FOUND        - True if glslang library is found.
#   glslang_INCLUDE_DIR  - The glslang include directiories.
#   glslang_LIBRARIES    - The glslang libraries.
#   glslang_REVISION     - The glslang version string.
#
# COMPONENTS directive is supported, the supported components are:
#
#   OSDependent
#   OGLCompiler
#   HLSL
#   SPVRemapper
#

find_package( PackageHandleStandardArgs )

set( PLATFORM "x86" )
if ( MSVC )
	if( (CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES Win64) )
		set( PLATFORM "x64" )
	endif()
else()
	if( (${CMAKE_SIZEOF_VOID_P} EQUAL 8) AND NOT MINGW )
		set( PLATFORM "x64" )
	endif()
endif()

if ( NOT glslang_FIND_COMPONENTS )
	set( glslang_FIND_COMPONENTS OSDependent OGLCompiler SPVRemapper )
endif ()

find_path( glslang_DIR include/glslang/Public/ShaderLang.h
	HINTS
	PATHS
		/usr/local
		/usr
)

if ( glslang_DIR )
	find_path( glslang_INCLUDE_DIR glslang/Public/ShaderLang.h
		HINTS
		PATH_SUFFIXES
			include
		PATHS
			${glslang_DIR}
	)

	find_path( glslang_LIBRARY_RELEASE_DIR glslang.lib libglslang.a
		HINTS
		PATH_SUFFIXES
			lib/${PLATFORM}/Release
			lib/${PLATFORM}
			lib/Release
			lib
			/usr/local/lib
			/usr/lib
		PATHS
			${glslang_DIR}
	)

	find_path( glslang_LIBRARY_DEBUG_DIR glslangd.lib libglslangd.a
		HINTS
		PATH_SUFFIXES
			lib/${PLATFORM}
			lib
			/usr/local/lib
			/usr/lib
		PATHS
			${glslang_DIR}
	)

	if ( NOT glslang_LIBRARY_DEBUG_DIR )
		find_path( glslang_LIBRARY_DEBUG_DIR glslang.lib libglslang.a
			HINTS
			PATH_SUFFIXES
				lib/${PLATFORM}/Debug
				lib/Debug
				/usr/local/lib
				/usr/lib
			PATHS
				${glslang_DIR}
		)
	endif ()

	find_library( glslang_LIBRARY_RELEASE
		NAMES
			glslang
		HINTS
		PATHS
			${glslang_LIBRARY_RELEASE_DIR}
	)

	find_library( glslang_LIBRARY_DEBUG
		NAMES
			glslangd
			glslang
		HINTS
		PATHS
			${glslang_LIBRARY_DEBUG_DIR}
	)
	mark_as_advanced( glslang_LIBRARY_RELEASE )
	mark_as_advanced( glslang_LIBRARY_DEBUG )
	mark_as_advanced( glslang_DIR )
	find_package_handle_standard_args( glslang DEFAULT_MSG glslang_LIBRARY_RELEASE glslang_LIBRARY_DEBUG glslang_INCLUDE_DIR )

	if ( glslang_FOUND )
		foreach( COMPONENT ${glslang_FIND_COMPONENTS} )
			find_library( glslang_${COMPONENT}_LIBRARY_RELEASE
				NAMES
					${COMPONENT}
				HINTS
				PATHS
					${glslang_LIBRARY_RELEASE_DIR}
			)

			find_library( glslang_${COMPONENT}_LIBRARY_DEBUG
				NAMES
					${COMPONENT}d
					${COMPONENT}
				HINTS
				PATHS
					${glslang_LIBRARY_DEBUG_DIR}
			)
			mark_as_advanced( glslang_${COMPONENT}_LIBRARY_RELEASE )
			mark_as_advanced( glslang_${COMPONENT}_LIBRARY_DEBUG )
			find_package_handle_standard_args( glslang_${COMPONENT} DEFAULT_MSG glslang_${COMPONENT}_LIBRARY_RELEASE glslang_${COMPONENT}_LIBRARY_DEBUG )

			if ( glslang_${COMPONENT}_FOUND )
				add_library( glslang::${COMPONENT} UNKNOWN IMPORTED )
				if ( glslang_${COMPONENT}_LIBRARY_DEBUG )
					set( glslang_${COMPONENT}_LIBRARIES
						optimized ${glslang_${COMPONENT}_LIBRARY_RELEASE}
						debug ${glslang_${COMPONENT}_LIBRARY_DEBUG}
						CACHE STRING "glslang ${COMPONENT} library" FORCE
					)
					set_property( TARGET glslang::${COMPONENT} APPEND PROPERTY
						IMPORTED_CONFIGURATIONS DEBUG )
					set_target_properties( glslang::${COMPONENT} PROPERTIES
						IMPORTED_LOCATION_DEBUG "${glslang_${COMPONENT}_LIBRARY_DEBUG}" )
					set_property( TARGET glslang::${COMPONENT} APPEND PROPERTY
						IMPORTED_CONFIGURATIONS RELEASE)
					set_target_properties(glslang::${COMPONENT} PROPERTIES
						IMPORTED_LOCATION_RELEASE "${glslang_${COMPONENT}_LIBRARY_RELEASE}" )
				else ()
					set( glslang_${COMPONENT}_LIBRARIES
						${glslang_${COMPONENT}_LIBRARY_RELEASE}
						CACHE STRING "glslang ${COMPONENT} library" FORCE
					)
					set_target_properties( glslang::${COMPONENT} PROPERTIES
						IMPORTED_LOCATION "${glslang_${COMPONENT}_LIBRARY_RELEASE}" )
				endif ()
				set_target_properties( glslang::${COMPONENT} PROPERTIES
					INTERFACE_INCLUDE_DIRECTORIES "${glslang_INCLUDE_DIR}" )
				set( glslang_LIBRARIES
					${glslang_LIBRARIES}
					${glslang_${COMPONENT}_LIBRARIES}
					CACHE STRING "glslang libraries" FORCE
				)
				mark_as_advanced( glslang_${COMPONENT}_LIBRARIES )

				if( NOT TARGET glslang::${COMPONENT} )
					add_library( glslang::glslang UNKNOWN IMPORTED )
					set_target_properties( glslang::${COMPONENT} PROPERTIES
						INTERFACE_INCLUDE_DIRECTORIES "${glslang_INCLUDE_DIR}" )

					if ( ZLIB_LIBRARY_RELEASE )
						set_property( TARGET glslang::${COMPONENT} APPEND PROPERTY
							IMPORTED_CONFIGURATIONS RELEASE)
						set_target_properties(glslang::${COMPONENT} PROPERTIES
							IMPORTED_LOCATION_RELEASE "${ZLIB_LIBRARY_RELEASE}" )
					endif ()

					if ( ZLIB_LIBRARY_DEBUG )
					endif ()

					if ( NOT ZLIB_LIBRARY_RELEASE AND NOT ZLIB_LIBRARY_DEBUG )
						set_property( TARGET glslang::${COMPONENT} APPEND PROPERTY
							IMPORTED_LOCATION "${ZLIB_LIBRARY}" )
					endif ()
				endif ()
			endif ()

			if ( glslang_FOUND AND NOT glslang_${COMPONENT}_FOUND )
				set( glslang_FOUND FALSE )
			endif ()
		endforeach ()
	endif ()
	if ( glslang_FOUND )
		set( glslang_INCLUDE_DIR "${glslang_INCLUDE_DIR}" CACHE STRING "glslang include directories" FORCE )
		unset( glslang_LIBRARY_RELEASE_DIR CACHE )
		unset( glslang_LIBRARY_DEBUG_DIR CACHE )

		set( glslang_REVISION_H ${glslang_INCLUDE_DIR}/glslang/Include/revision.h )
		if ( EXISTS glslang_REVISION_H )
			file( STRINGS "${glslang_REVISION_H}" glslang_REVISION_STR REGEX "^#[\t ]*define[\t ]+glslang_REVISION[\t ]+\"[0-9a-zA-Z .-]+\"$" )
			file( STRINGS "${glslang_REVISION_H}" glslang_DATE_STR REGEX "^#[\t ]*define[\t ]+glslang_DATE[\t ]+\"[0-9a-zA-Z .-]+\"$" )
			unset( glslang_REVISION CACHE )
			if ( glslang_REVISION_STR MATCHES "^#[\t ]*define[\t ]+glslang_REVISION[\t ]+\"[0-9a-zA-Z .-]+\"$" )
				if ( glslang_DATE_STR MATCHES "^#[\t ]*define[\t ]+glslang_DATE[\t ]+\"[0-9a-zA-Z .-]+\"$" )
					string( REGEX REPLACE "^#[\t ]*define[\t ]+glslang_REVISION[\t ]+\"([0-9a-zA-Z .-]+)\"$" "\\1" glslang_REVISION_STR "${glslang_REVISION_STR}" )
					string( REGEX REPLACE "^#[\t ]*define[\t ]+glslang_DATE[\t ]+\"([0-9a-zA-Z .-]+)\"$" "\\1" glslang_DATE_STR "${glslang_DATE_STR}" )
					set( glslang_REVISION "${glslang_REVISION_STR}.${glslang_DATE_STR}" CACHE STRING "glslang revision string" FORCE )
					unset( glslang_REVISION_STR )
					unset( glslang_DATE_STR )
				endif ()
			endif ()
		endif ()
	endif ()
else ()
	set( glslang_FOUND FALSE )
endif ()
