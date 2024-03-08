find_package( PackageHandleStandardArgs )

if ( MSVC )
	if ( NOT ATL_DIR )
		if ( MSVC14 )
			get_filename_component( VS_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\14.0\\Setup\\VS;ProductDir]" REALPATH )
		elseif ( MSVC12 )
			get_filename_component( VS_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\12.0\\Setup\\VS;ProductDir]" REALPATH )
		elseif ( MSVC11 )
			get_filename_component( VS_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\11.0\\Setup\\VS;ProductDir]" REALPATH )
		elseif ( MSVC10 )
			get_filename_component( VS_DIR "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\10.0\\Setup\\VS;ProductDir]" REALPATH )
		else()
			message( STATUS "!! Could not find a compatible Visual Studio directory!" )
		endif()
		if ( VS_DIR )
			set( ATL_DIR ${VS_DIR}/VC )
		endif ()
	endif()

	if ( ATL_DIR )
		FIND_PATH( ATL_ROOT_DIR include/atlbase.h
			HINTS
			PATH_SUFFIXES
				atlmfc
			PATHS
				${ATL_DIR}
		)

		FIND_PATH( ATL_INCLUDE_DIR atlbase.h 
			HINTS
			PATH_SUFFIXES
				include
			PATHS
				${ATL_ROOT_DIR}
		)
	endif()

	mark_as_advanced( ATL_ROOT_DIR ATL_INCLUDE_DIR )
	find_package_handle_standard_args( ATL DEFAULT_MSG ATL_INCLUDE_DIR )
endif()