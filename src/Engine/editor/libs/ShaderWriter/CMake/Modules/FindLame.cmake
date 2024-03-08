FIND_PATH(Lame_ROOT_DIR include/lame.h include/lame/lame.h
	HINTS
	PATH_SUFFIXES
		lame-3.99.5
	PATHS
		/usr/local
		/usr
)

FIND_PATH(Lame_INCLUDE_DIR lame.h
	HINTS
	PATH_SUFFIXES
		include
		include/lame
	PATHS
		${Lame_ROOT_DIR}
)

FIND_PATH(Lame_LIBRARY_DIR libmp3lame.lib libmp3lame.so
	HINTS
	PATH_SUFFIXES
		output/ReleaseSSE2
		output/Release
		lib64
		lib
		lib/x86_64-linux-gnu
	PATHS
		${Lame_ROOT_DIR}
)

FIND_LIBRARY(Lame_LIBRARY
	NAMES
		libmp3lame.so
		libmp3lame.lib
	HINTS
	PATHS
		${Lame_LIBRARY_DIR}
)

MARK_AS_ADVANCED( Lame_LIBRARY_DIR )
MARK_AS_ADVANCED( Lame_LIBRARY )

SET( Lame_LIBRARY_DIRS ${Lame_LIBRARY_DIR} )
SET( Lame_LIBRARIES ${Lame_LIBRARY} )

find_package_handle_standard_args( Lame DEFAULT_MSG Lame_LIBRARIES Lame_INCLUDE_DIR )
