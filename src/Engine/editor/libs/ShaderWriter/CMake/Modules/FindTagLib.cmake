FIND_PATH(TagLib_ROOT_DIR taglib/tag.h
	HINTS
	PATH_SUFFIXES
		taglib
	PATHS
		taglib
		/usr/local
		/usr
)

FIND_PATH(TagLib_INCLUDE_DIR taglib/tag.h 
	HINTS
	PATH_SUFFIXES
		include
	PATHS
		${TagLib_ROOT_DIR}
)

FIND_PATH(TagLib_LIBRARY_DIR libtag.so tag.lib
	HINTS
	PATH_SUFFIXES
		lib64
		lib
		lib/x86_64-linux-gnu
	PATHS
		${TagLib_ROOT_DIR}
)

FIND_LIBRARY(TagLib_LIBRARY
	NAMES
		libtag.so
		tag.lib
	HINTS
	PATH_SUFFIXES
		lib64
		lib
	PATHS
		${TagLib_LIBRARY_DIR}
)

MARK_AS_ADVANCED( TagLib_LIBRARY_DIR )
MARK_AS_ADVANCED( TagLib_LIBRARY )

SET( TagLib_LIBRARY_DIRS ${TagLib_LIBRARY_DIR} )
SET( TagLib_LIBRARIES ${TagLib_LIBRARY} )

find_package_handle_standard_args( TagLib DEFAULT_MSG TagLib_LIBRARIES TagLib_INCLUDE_DIR )
