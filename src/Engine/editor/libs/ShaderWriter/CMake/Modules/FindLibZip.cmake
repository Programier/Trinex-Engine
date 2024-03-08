# FindLibZip
# ------------
#
# Locate libzip library
#
# This module defines
#
# ::
#
#   libzip_LIBRARY, the library to link against
#   libzip_FOUND, if false, do not try to link to libzip
#   libzip_INCLUDE_DIR, where to find headers.
#

find_path( libzip_DIR include/zip.h 
	HINTS
	PATHS
		/usr/local
		/usr
)
find_path( libzip_INCLUDE_DIR zip.h 
	HINTS
	PATH_SUFFIXES
		include
	PATHS
		${libzip_DIR}
)
find_path( libzip_LIBRARY_DIR_RELEASE
		libzip.so
		zip.lib
	HINTS
	PATHS
		${libzip_DIR}/lib
)
find_library( libzip_LIBRARY_RELEASE
	NAMES
		zip
	HINTS
	PATHS
		${libzip_LIBRARY_DIR_RELEASE}
)
find_path( libzip_LIBRARY_DIR_DEBUG
		libzip.so
		zip.lib
	HINTS
	PATH_SUFFIXES
	PATHS
		${libzip_DIR}/debug/lib
		${libzip_DIR}/lib
)
find_library( libzip_LIBRARY_DEBUG
	NAMES
		zip
	HINTS
	PATHS
		${libzip_LIBRARY_DIR_DEBUG}
)
find_path( libzip_ZIPCONF_DIR zipconf.h 
	HINTS
	PATH_SUFFIXES
		libzip/include
		include
	PATHS
		${libzip_LIBRARY_DIR_RELEASE}
		${libzip_LIBRARY_DIR_DEBUG}
		${libzip_INCLUDE_DIR}
)
find_package_handle_standard_args( libzip DEFAULT_MSG libzip_LIBRARY_RELEASE libzip_INCLUDE_DIR libzip_ZIPCONF_DIR )

if ( libzip_FOUND )
	set( libzip_INCLUDE_DIRS
		${libzip_INCLUDE_DIR}
		${libzip_ZIPCONF_DIR}
	)
	add_library( libzip STATIC IMPORTED )
	set_target_properties( libzip PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES ${libzip_INCLUDE_DIR}
		INTERFACE_INCLUDE_DIRECTORIES ${libzip_ZIPCONF_DIR}
	)

	if ( libzip_LIBRARY_DEBUG )
		set_property( TARGET libzip APPEND
			PROPERTY
				IMPORTED_CONFIGURATIONS DEBUG
		)
		set_target_properties( libzip PROPERTIES
			IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
			IMPORTED_LOCATION_DEBUG ${libzip_LIBRARY_DEBUG}
		)
		set_property( TARGET libzip APPEND
			PROPERTY
				IMPORTED_CONFIGURATIONS RELEASE
		)
		set_target_properties( libzip PROPERTIES
			IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
			IMPORTED_LOCATION_RELEASE ${libzip_LIBRARY_RELEASE}
		)
	else ()
		set_target_properties( libzip PROPERTIES
			IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
			IMPORTED_LOCATION_RELEASE ${libzip_LIBRARY_RELEASE}
		)
	endif ()

	unset( libzip_ROOT_DIR CACHE )
	unset( libzip_INCLUDE_DIR CACHE )
	unset( libzip_ZIPCONF_DIR CACHE )
	unset( libzip_LIBRARY_RELEASE CACHE )
	unset( libzip_LIBRARY_DIR_RELEASE CACHE )
	unset( libzip_LIBRARY_DEBUG CACHE )
	unset( libzip_LIBRARY_DIR_DEBUG CACHE )
else ()
	mark_as_advanced( libzip_ROOT_DIR )
	mark_as_advanced( libzip_INCLUDE_DIR )
	mark_as_advanced( libzip_ZIPCONF_DIR )
	mark_as_advanced( libzip_LIBRARY_RELEASE )
	mark_as_advanced( libzip_LIBRARY_DIR_RELEASE )
	mark_as_advanced( libzip_LIBRARY_DEBUG )
	mark_as_advanced( libzip_LIBRARY_DIR_DEBUG )
endif ()
