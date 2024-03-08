include( Logging )

function( install_dll_ex _TARGET _COMPONENT _DLL_PATH_DEBUG _DLL_PATH_RELEASE )
	install(
		FILES ${_DLL_PATH_RELEASE}
		DESTINATION bin
		COMPONENT ${_COMPONENT}
		CONFIGURATIONS Release
	)
	install(
		FILES ${_DLL_PATH_RELEASE}
		DESTINATION bin/RelWithDebInfo
		COMPONENT ${_COMPONENT}
		CONFIGURATIONS RelWithDebInfo
	)
	install(
		FILES ${_DLL_PATH_DEBUG}
		DESTINATION bin/Debug
		COMPONENT ${_COMPONENT}
		CONFIGURATIONS Debug
	)
endfunction()

function( copy_and_install_ex _TARGET _COMPONENT _DLL_PATH_DEBUG _DLL_PATH_RELEASE _DLL_PATH_RELWITHDEBINFO )
	get_filename_component( _FILE ${_DLL_PATH_RELEASE} NAME_WE )
	get_filename_component( _LIB_NAME_DEBUG ${_DLL_PATH_DEBUG} NAME )
	get_filename_component( _LIB_NAME_RELEASE ${_DLL_PATH_RELEASE} NAME )
	get_filename_component( _LIB_NAME_RELWITHDEBINFO ${_DLL_PATH_RELWITHDEBINFO} NAME )
	msg_debug( "${_LIB_NAME_DEBUG} ${_DLL_PATH_DEBUG}" )
	msg_debug( "${_LIB_NAME_RELEASE} ${_DLL_PATH_RELEASE}" )
	msg_debug( "${_LIB_NAME_RELWITHDEBINFO} ${_DLL_PATH_RELWITHDEBINFO}" )
	msg_debug( "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}${_LIB_NAME_DEBUG}" )
	msg_debug( "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}${_LIB_NAME_RELEASE}" )
	msg_debug( "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}${_LIB_NAME_RELWITHDEBINFO}" )
	add_custom_command(
		TARGET ${_TARGET}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<$<CONFIG:Debug>:${_DLL_PATH_DEBUG}>
			$<$<CONFIG:Debug>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}${_LIB_NAME_DEBUG}>
			$<$<CONFIG:Release>:${_DLL_PATH_RELEASE}>
			$<$<CONFIG:Release>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}${_LIB_NAME_RELEASE}>
			$<$<CONFIG:RelWithDebInfo>:${_DLL_PATH_RELWITHDEBINFO}>
			$<$<CONFIG:RelWithDebInfo>:${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO}${_LIB_NAME_RELWITHDEBINFO}>
		COMMENT "Copying ${_FILE} into binary folder"
	)
	install_dll_ex( ${_TARGET}
		${_COMPONENT}
		${_DLL_PATH_DEBUG}
		${_DLL_PATH_RELEASE}
	)
endfunction()

function( copy_and_install _TARGET _DLL_PATH_DEBUG _DLL_PATH_RELEASE _DLL_PATH_RELWITHDEBINFO )
	copy_and_install_ex( ${_TARGET} ${_TARGET} ${_DLL_PATH_DEBUG} ${_DLL_PATH_RELEASE} ${_DLL_PATH_RELWITHDEBINFO} )
endfunction()

function( find_dll_config _OUTPUT _LIB_FULL_PATH_NAME _SUFFIX )
	get_filename_component( _DllName ${_LIB_FULL_PATH_NAME} NAME_WE )
	get_filename_component( _DllPath ${_LIB_FULL_PATH_NAME} PATH )

	if ( NOT "" STREQUAL "${_DllPath}" )
		get_filename_component( _PathLeaf1 ${_DllPath} NAME )
		get_filename_component( _DllPath ${_DllPath} PATH )
		get_filename_component( _PathLeaf2 ${_DllPath} NAME )
		get_filename_component( _DllPath ${_DllPath} PATH )
		get_filename_component( _PathLeaf3 ${_DllPath} NAME )
		get_filename_component( _DllPath ${_DllPath} PATH )
		set( _PathLeafs
			${_PathLeaf3}
			${_PathLeaf2}
			${_PathLeaf1}
		)
		set( _LookupPaths
			${_DllPath}/${_PathLeaf3}/${_PathLeaf2}/${_PathLeaf1}/bin
			${_DllPath}/${_PathLeaf3}/${_PathLeaf2}/${_PathLeaf1}/lib
			${_DllPath}/${_PathLeaf3}/${_PathLeaf2}/bin/${_PathLeaf1}
			${_DllPath}/${_PathLeaf3}/${_PathLeaf2}/lib/${_PathLeaf1}
			${_DllPath}/${_PathLeaf3}/bin/${_PathLeaf2}/${_PathLeaf1}
			${_DllPath}/${_PathLeaf3}/lib/${_PathLeaf2}/${_PathLeaf1}
			${_DllPath}/bin/${_PathLeaf3}/${_PathLeaf2}/${_PathLeaf1}
			${_DllPath}/lib/${_PathLeaf3}/${_PathLeaf2}/${_PathLeaf1}
			${_DllPath}/${_PathLeaf3}/${_PathLeaf1}/bin
			${_DllPath}/${_PathLeaf3}/${_PathLeaf1}/lib
			${_DllPath}/${_PathLeaf3}/bin/${_PathLeaf1}
			${_DllPath}/${_PathLeaf3}/lib/${_PathLeaf1}
			${_DllPath}/bin/${_PathLeaf3}/${_PathLeaf1}
			${_DllPath}/lib/${_PathLeaf3}/${_PathLeaf1}
			${_DllPath}/${_PathLeaf2}/${_PathLeaf1}/bin
			${_DllPath}/${_PathLeaf2}/${_PathLeaf1}/lib
			${_DllPath}/${_PathLeaf2}/bin/${_PathLeaf1}
			${_DllPath}/${_PathLeaf2}/lib/${_PathLeaf1}
			${_DllPath}/bin/${_PathLeaf2}/${_PathLeaf1}
			${_DllPath}/lib/${_PathLeaf2}/${_PathLeaf1}
			${_DllPath}/${_PathLeaf3}/${_PathLeaf2}/bin
			${_DllPath}/${_PathLeaf3}/${_PathLeaf2}/lib
			${_DllPath}/${_PathLeaf3}/bin/${_PathLeaf2}
			${_DllPath}/${_PathLeaf3}/lib/${_PathLeaf2}
			${_DllPath}/bin/${_PathLeaf3}/${_PathLeaf2}
			${_DllPath}/lib/${_PathLeaf3}/${_PathLeaf2}
			${_DllPath}/${_PathLeaf1}/bin
			${_DllPath}/${_PathLeaf1}/lib
			${_DllPath}/bin/${_PathLeaf1}
			${_DllPath}/lib/${_PathLeaf1}
			${_DllPath}/${_PathLeaf2}/bin
			${_DllPath}/${_PathLeaf2}/lib
			${_DllPath}/bin/${_PathLeaf2}
			${_DllPath}/lib/${_PathLeaf2}
			${_DllPath}/${_PathLeaf3}/bin
			${_DllPath}/${_PathLeaf3}/lib
			${_DllPath}/bin/${_PathLeaf3}
			${_DllPath}/lib/${_PathLeaf3}
			${_DllPath}/bin
			${_DllPath}/lib
		)
			msg_debug( "    _PathLeaf3 ${_PathLeaf3}" )
			msg_debug( "    _PathLeaf2 ${_PathLeaf2}" )
			msg_debug( "    _PathLeaf1 ${_PathLeaf1}" )
			msg_debug( "    DLL        ${_DllName}${_SUFFIX}" )
			msg_debug( "    DllPath    ${_DllPath}" )
			msg_debug( "    Found      ${_DllFile}" )
			msg_debug( "    Lookup     ${_LookupPaths}" )
		unset( _DllFile CACHE )
		find_file( _DllFile ${_DllName}${_SUFFIX}
			PATHS
				${_LookupPaths}
			NO_DEFAULT_PATH
		)
		if ( _DllFile )
			msg_debug( "    Found      ${_DllFile}" )
			set( ${_OUTPUT} ${_DllFile} PARENT_SCOPE )
		else ()
			msg_debug( "    _PathLeaf3 ${_PathLeaf3}" )
			msg_debug( "    _PathLeaf2 ${_PathLeaf2}" )
			msg_debug( "    _PathLeaf1 ${_PathLeaf1}" )
			msg_debug( "    DLL        ${_DllName}${_SUFFIX}" )
			msg_debug( "    DllPath    ${_DllPath}" )
			msg_debug( "    Found      ${_DllFile}" )
			msg_debug( "    Lookup     ${_LookupPaths}" )
		endif ()
		unset( _DllFile CACHE )
	endif ()
endfunction()

function( copy_dll_ex _TARGET _COMPONENT _LIB_FULL_PATH_NAME_DEBUG _LIB_FULL_PATH_NAME_RELEASE )# ARG4 _SUFFIX
	if ( WIN32 )
		set( _DllSuffix "${ARGV4}.dll" )
		find_dll_config( _DLL_FILE_DEBUG ${_LIB_FULL_PATH_NAME_DEBUG} ${_DllSuffix} )
		find_dll_config( _DLL_FILE_RELEASE ${_LIB_FULL_PATH_NAME_RELEASE} ${_DllSuffix} )
		find_dll_config( _DLL_FILE_RELWITHDEBINFO ${_LIB_FULL_PATH_NAME_RELEASE} ${_DllSuffix} )
		if ( _DLL_FILE_DEBUG AND _DLL_FILE_RELEASE AND _DLL_FILE_RELWITHDEBINFO )
			copy_and_install_ex( ${_TARGET} ${_COMPONENT} ${_DLL_FILE_DEBUG} ${_DLL_FILE_RELEASE} ${_DLL_FILE_RELWITHDEBINFO} )
		else ()
			message( "Could not find external DLL: ${_LIB_FULL_PATH_NAME_DEBUG} [${_DLL_FILE_DEBUG}]" )
			message( "Could not find external DLL: ${_LIB_FULL_PATH_NAME_RELEASE} [${_DLL_FILE_RELEASE}]" )
			message( "Could not find external DLL: ${_LIB_FULL_PATH_NAME_RELEASE} [${_DLL_FILE_RELWITHDEBINFO}]" )
		endif ()
	endif ()
endfunction()

function( find_install_dll_ex _TARGET _COMPONENT _LIB_FULL_PATH_NAME_DEBUG _LIB_FULL_PATH_NAME_RELEASE )# ARG4 _SUFFIX
	if ( WIN32 )
		set( _DllSuffix "${ARGV4}.dll" )
		find_dll_config( _DLL_FILE_DEBUG ${_LIB_FULL_PATH_NAME_DEBUG} ${_DllSuffix} )
		find_dll_config( _DLL_FILE_RELEASE ${_LIB_FULL_PATH_NAME_RELEASE} ${_DllSuffix} )
		find_dll_config( _DLL_FILE_RELWITHDEBINFO ${_LIB_FULL_PATH_NAME_RELEASE} ${_DllSuffix} )
		if ( _DLL_FILE_DEBUG AND _DLL_FILE_RELEASE AND _DLL_FILE_RELWITHDEBINFO )
			install_dll_ex( ${_TARGET} ${_COMPONENT} ${_DLL_FILE_DEBUG} ${_DLL_FILE_RELEASE} ${_DLL_FILE_RELWITHDEBINFO} )
		else ()
			message( "Could not find external DLL: ${_LIB_FULL_PATH_NAME_DEBUG} [${_DLL_FILE_DEBUG}]" )
			message( "Could not find external DLL: ${_LIB_FULL_PATH_NAME_RELEASE} [${_DLL_FILE_RELEASE}]" )
			message( "Could not find external DLL: ${_LIB_FULL_PATH_NAME_RELEASE} [${_DLL_FILE_RELWITHDEBINFO}]" )
		endif ()
	endif ()
endfunction()

function( copy_dll _TARGET _LIB_FULL_PATH_NAME_DEBUG _LIB_FULL_PATH_NAME_RELEASE )# ARG3 _SUFFIX
	copy_dll_ex( ${_TARGET}
		${_TARGET}
		${_LIB_FULL_PATH_NAME_DEBUG}
		${_LIB_FULL_PATH_NAME_RELEASE}
		${ARG3}
	)
endfunction()
