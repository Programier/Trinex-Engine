# FindAshes
# ---------
#
# Locate Ashes library
#
# This module defines
#
# ::
#
#   Ashes_LIBRARIES, the libraries to link against
#   Ashes_FOUND, if false, do not try to link to Ashes
#   Ashes_INCLUDE_DIR, where to find headers.
#

find_path( Ashes_ROOT_DIR include/Ashes/AshesPrerequisites.hpp
	HINTS
	PATH_SUFFIXES
		Ashes
	PATHS
		/usr/local
		/usr
)

find_path( Ashes_INCLUDE_DIR Ashes/AshesPrerequisites.hpp
	HINTS
	PATH_SUFFIXES
		include
	PATHS
		${Ashes_ROOT_DIR}
		/usr/local/include
		/usr/include
)

if (CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES Win64)
	if( MSVC )
		find_path( Ashes_LIBRARY_RELEASE_DIR Ashes.lib
			HINTS
			PATH_SUFFIXES
				lib/x64
				lib/x64/Release
				lib/Release/x64
				lib
			PATHS
				${Ashes_ROOT_DIR}
		)
		find_library( Ashes_LIBRARY_RELEASE
			NAMES
				Ashes.lib
			HINTS
			PATHS
				${Ashes_LIBRARY_RELEASE_DIR}
		)

		find_path( Ashes_LIBRARY_DEBUG_DIR Ashesd.lib
			HINTS
			PATH_SUFFIXES
				lib/x64
				lib/x64/Debug
				lib/Debug/x64
				lib
			PATHS
				${Ashes_ROOT_DIR}
				${Ashes_LIBRARY_RELEASE_DIR}
		)
		find_library( Ashes_LIBRARY_DEBUG
			NAMES
				Ashesd.lib
			HINTS
			PATHS
				${Ashes_LIBRARY_DEBUG_DIR}
		)
	else()
		find_path( Ashes_LIBRARY_RELEASE_DIR libAshes.so libAshes.lib
			HINTS
			PATH_SUFFIXES
				lib64
				lib/x64/Release
				lib/Release/x64
				lib
			PATHS
				${Ashes_ROOT_DIR}
		)
		find_library( Ashes_LIBRARY_RELEASE
			NAMES
				libAshes.so
				libAshes.dll.a
			HINTS
			PATHS
				${Ashes_LIBRARY_RELEASE_DIR}
		)

		find_path( Ashes_LIBRARY_DEBUG_DIR libAshes.so libAshes.lib
			HINTS
			PATH_SUFFIXES
				lib64
				lib/x64/Debug
				lib/Debug/x64
				lib
			PATHS
				${Ashes_ROOT_DIR}
		)
		find_library( Ashes_LIBRARY_DEBUG
			NAMES
				libAshes.so
				libAshes.dll.a
			HINTS
			PATHS
				${Ashes_LIBRARY_DEBUG_DIR}
		)
	endif()
else()
	if( MSVC )
		find_path( Ashes_LIBRARY_RELEASE_DIR Ashes.lib
		HINTS
		PATH_SUFFIXES
			lib/x86
			lib/x86/Release
			lib/Release/x86
			lib
		PATHS
			${Ashes_ROOT_DIR}
		)
		find_library( Ashes_LIBRARY_RELEASE
			NAMES
				Ashes.lib
			HINTS
			PATHS
				${Ashes_LIBRARY_RELEASE_DIR}
		)

		find_path( Ashes_LIBRARY_DEBUG_DIR Ashesd.lib
		HINTS
		PATH_SUFFIXES
			lib/x86
			lib/x86/Debug
			lib/Debug/x86
			lib
		PATHS
			${Ashes_ROOT_DIR}
			${Ashes_LIBRARY_RELEASE_DIR}
		)
		find_library( Ashes_LIBRARY_DEBUG
			NAMES
				Ashesd.lib
			HINTS
			PATHS
				${Ashes_LIBRARY_DEBUG_DIR}
		)
	else()
		find_path( Ashes_LIBRARY_RELEASE_DIR libAshes.so
			HINTS
			PATH_SUFFIXES
				lib/x86/Release
				lib/Release/x86
				lib
			PATHS
				${Ashes_ROOT_DIR}
		)
		find_library(Ashes_LIBRARY_RELEASE
			NAMES
				libAshes.so
			HINTS
			PATHS
				${Ashes_LIBRARY_RELEASE_DIR}
		)

		find_path( Ashes_LIBRARY_DEBUG_DIR libAshes.so
			HINTS
			PATH_SUFFIXES
				lib/x86/Debug
				lib/Debug/x86
				lib
			PATHS
				${Ashes_ROOT_DIR}
				${Ashes_LIBRARY_RELEASE_DIR}
		)
		find_library(Ashes_LIBRARY_DEBUG
			NAMES
				libAshes.so
			HINTS
			PATHS
				${Ashes_LIBRARY_DEBUG_DIR}
		)
	endif()
endif()

mark_as_advanced( Ashes_ROOT_DIR )
find_package_handle_standard_args( Ashes DEFAULT_MSG Ashes_LIBRARY_RELEASE Ashes_INCLUDE_DIR )

if ( Ashes_FOUND )
	if (MSVC)
		if ( Ashes_LIBRARY_DEBUG )
			set( Ashes_LIBRARIES optimized ${Ashes_LIBRARY_RELEASE} debug ${Ashes_LIBRARY_DEBUG} CACHE STRING "Ashes libraries" )
		else()
			set( Ashes_LIBRARIES ${Ashes_LIBRARY_RELEASE} CACHE STRING "Ashes libraries" )
		endif()
	else ()
		if ( Ashes_LIBRARY_DEBUG )
			set( Ashes_LIBRARIES optimized ${Ashes_LIBRARY_RELEASE} debug ${Ashes_LIBRARY_DEBUG} CACHE STRING "Ashes libraries" )
		else()
			set( Ashes_LIBRARIES ${Ashes_LIBRARY_RELEASE} CACHE STRING "Ashes libraries" )
		endif()
	endif ()
	unset( Ashes_LIBRARY_RELEASE_DIR CACHE )
	unset( Ashes_LIBRARY_DEBUG_DIR CACHE )
	unset( Ashes_LIBRARY_RELEASE CACHE )
	unset( Ashes_LIBRARY_DEBUG CACHE )
endIF ()