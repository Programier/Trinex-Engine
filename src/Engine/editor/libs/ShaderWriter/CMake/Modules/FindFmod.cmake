FIND_PATH( Fmod_ROOT_DIR inc/fmod.hpp include/fmod.hpp
	HINTS
	PATH_SUFFIXES
		fmod/api
	PATHS
		/usr/local
		/usr
)

FIND_PATH( Fmod_INCLUDE_DIR fmod.hpp
	HINTS
	PATH_SUFFIXES
		include
		inc
	PATHS
		${Fmod_ROOT_DIR}
)

FIND_PATH( Fmod_LIBRARY_DIR fmod64_vc.lib libfmod.so
	HINTS
	PATH_SUFFIXES
		lib64
		lib
		lib/x64
		lib/x86_64
	PATHS
		${Fmod_ROOT_DIR}
)

FIND_LIBRARY(Fmod_LIBRARY
	NAMES
		fmod64_vc.lib
		libfmod.so
	HINTS
	PATH_SUFFIXES
		lib64
		lib
	PATHS
		${Fmod_LIBRARY_DIR}
)

MARK_AS_ADVANCED( Fmod_LIBRARY_DIR )
MARK_AS_ADVANCED( Fmod_LIBRARY )

SET( Fmod_LIBRARY_DIRS ${Fmod_LIBRARY_DIR} )
SET( Fmod_LIBRARIES ${Fmod_LIBRARY} )

find_package_handle_standard_args( Fmod DEFAULT_MSG Fmod_LIBRARIES Fmod_INCLUDE_DIR )

if (Fmod_FOUND )
	if ( NOT TARGET Fmod::Fmod )
		add_library( Fmod::Fmod UNKNOWN IMPORTED )
		set_target_properties(Fmod::Fmod PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${Fmod_INCLUDE_DIR}" )
		if ( Fmod_LIBRARY )
			set_target_properties( Fmod::Fmod PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${Fmod_LIBRARY}" )
		endif()
	endif()
endif ()
