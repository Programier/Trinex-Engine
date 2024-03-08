# - Find mpg123
# Find the native mpg123 includes and libraries
#
#  mpg123_INCLUDE_DIRS - where to find mpg123.h, etc.
#  mpg123_LIBRARIES    - List of libraries when using mpg123.
#  mpg123_FOUND        - True if mpg123 found.

find_path( mpg123_DIR include/mpg123.h
	HINTS
	PATH_SUFFIXES
		mpg123
	PATHS
		/usr/local
		/usr
)
find_path( mpg123_INCLUDE_DIR mpg123.h 
	HINTS
	PATH_SUFFIXES
		include
	PATHS
		${mpg123_DIR}/include
)
find_path( mpg123_LIBRARY_RELEASE_DIR libmpg123.lib mpg123.lib libmpg123.a libmpg123.so
	HINTS
	PATHS
		${mpg123_DIR}/lib
)
find_path( mpg123_LIBRARY_DEBUG_DIR libmpg123.lib mpg123.lib libmpg123.a libmpg123.so
	HINTS
	PATHS
		${mpg123_DIR}/debug/lib
)
find_library( mpg123_LIBRARY_RELEASE
	NAMES
		mpg123 libmpg123
	PATHS
		${mpg123_LIBRARY_RELEASE_DIR}
)
find_library( mpg123_LIBRARY_DEBUG
	NAMES
		mpg123 libmpg123
	PATHS
		${mpg123_LIBRARY_DEBUG_DIR}
)

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( mpg123 DEFAULT_MSG mpg123_INCLUDE_DIR mpg123_LIBRARY_RELEASE )
mark_as_advanced( mpg123_INCLUDE_DIR mpg123_LIBRARY_RELEASE mpg123_LIBRARY_DEBUG )

if ( mpg123_FOUND )
	if ( NOT TARGET mpg123::mpg123 )
		add_library( mpg123::mpg123 UNKNOWN IMPORTED )
		set_target_properties( mpg123::mpg123 PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${mpg123_INCLUDE_DIR}" )
		if ( mpg123_LIBRARY_RELEASE AND mpg123_LIBRARY_DEBUG )
			set_property( TARGET mpg123::mpg123 APPEND PROPERTY
				IMPORTED_CONFIGURATIONS RELEASE )
			set_target_properties( mpg123::mpg123 PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
				IMPORTED_LOCATION_RELEASE "${mpg123_LIBRARY_RELEASE}" )
			set_property( TARGET mpg123::mpg123 APPEND PROPERTY
				IMPORTED_CONFIGURATIONS DEBUG )
			set_target_properties( mpg123::mpg123 PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
				IMPORTED_LOCATION_DEBUG "${mpg123_LIBRARY_DEBUG}" )
		elseif ( mpg123_LIBRARY_RELEASE )
			set_target_properties( mpg123::mpg123 PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${mpg123_LIBRARY_RELEASE}" )
		endif()
	endif()
	if ( mpg123_LIBRARY_DEBUG )
		set( mpg123_LIBRARIES optimized ${mpg123_LIBRARY_RELEASE} debug ${mpg123_LIBRARY_DEBUG} CACHE STRING "mpg123 libraries" )
	else ()
		set( mpg123_LIBRARIES ${mpg123_LIBRARY_RELEASE} CACHE STRING "mpg123 libraries" )
	endif ()
	set( mpg123_INCLUDE_DIRS ${mpg123_INCLUDE_DIR} )
endif ()
