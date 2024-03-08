###########################################################
#
# Find Visual Leak Detector on client machine
#
## 1: Setup:
# The following variables are searched for defaults
#  VLD_ROOT_DIR:            Base directory of VLD tree to use.
#
## 2: Variable
# The following are set after configuration is done: 
#  
#  VLD_INCLUDE_DIR
#  VLD_LIBRARY
#
###########################################################

find_package( PackageHandleStandardArgs )

if( MSVC )
	find_path( VLD_ROOT_DIR include/vld.h 
		HINTS
		PATH_SUFFIXES
			include
			vld
			VisualLeakDetector
			"Visual Leak Detector"
		PATHS
		/usr/local
		/usr
		C:/ Z:/
	)

	find_path( VLD_INCLUDE_DIR vld.h
	  HINTS
	  PATH_SUFFIXES include
	  PATHS
	  ${VLD_ROOT_DIR}
	)

	if (CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES Win64)
		find_path( VLD_LIBRARY_DIR vld.lib
			HINTS
				PATH_SUFFIXES lib/Win64
				PATHS ${VLD_ROOT_DIR}
		)
	else ()
		find_path( VLD_LIBRARY_DIR vld.lib
			HINTS
				PATH_SUFFIXES lib/Win32
				PATHS ${VLD_ROOT_DIR}
		)
	endif ()

	find_library( VLD_LIBRARY
		NAMES vld.lib
		PATHS
			${VLD_LIBRARY_DIR}
	)

	if ( VLD_LIBRARY )
		if ( NOT TARGET vld::vld )
			add_library( vld::vld UNKNOWN IMPORTED )
			set_target_properties(vld::vld PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${VLD_INCLUDE_DIR}" )
			set_target_properties( vld::vld PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${VLD_LIBRARY}" )

			set_property( TARGET vld::vld APPEND PROPERTY
				IMPORTED_CONFIGURATIONS DEBUG
			)
			set_target_properties( vld::vld PROPERTIES
				IMPORTED_LOCATION_DEBUG "${VLD_LIBRARY}"
			)
			set_property( TARGET vld::vld APPEND PROPERTY
				IMPORTED_CONFIGURATIONS RELEASE
			)
			set_target_properties( vld::vld PROPERTIES
				IMPORTED_LOCATION_RELEASE "${VLD_LIBRARY}"
			)
		endif()
		set( VLD_LIBRARIES ${VLD_LIBRARY} CACHE STRING "VLD libraries" )
		unset( VLD_LIBRARY_DIR CACHE )
	else ()
		mark_as_advanced( VLD_LIBRARY_DIR )
	endif ()
endif()

find_package_handle_standard_args( VLD DEFAULT_MSG VLD_LIBRARY VLD_INCLUDE_DIR )