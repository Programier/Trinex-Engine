if( WIN32 )
	find_path( DXC_ROOT_DIR inc/dxcapi.h
	  HINTS
		ENV DXSDK_DIR
	)
	find_path( DXC_INCLUDE_DIR dxcapi.h 
		HINTS
		PATH_SUFFIXES inc
		PATHS
		${DXC_ROOT_DIR}
	)
	find_path( DXC_LIBRARY_DIR dxcompiler.lib
		HINTS
		PATH_SUFFIXES lib/x64
		PATHS
		${DXC_ROOT_DIR}
	)
	find_library( DXC_LIBRARY
		NAMES dxcompiler.lib
		HINTS
		PATH_SUFFIXES lib/x64
		PATHS
		${DXC_LIBRARY_DIR}
	)
	mark_as_advanced( DXC_LIBRARY_DIR DXC_LIBRARY )
	find_package_handle_standard_args( DXC DEFAULT_MSG DXC_LIBRARY DXC_INCLUDE_DIR )

	if ( DXC_FOUND )
		if ( NOT TARGET DXC::dxc )
			add_library( DXC::dxc UNKNOWN IMPORTED )
			set_target_properties( DXC::dxc PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${DXC_INCLUDE_DIR}"
			)
			set_target_properties( DXC::dxc PROPERTIES
				IMPORTED_LINK_INTERFACE_LANGUAGES "C"
				IMPORTED_LOCATION "${DXC_LIBRARY}" )
		endif()
	endif ()
endif()
