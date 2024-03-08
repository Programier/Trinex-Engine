function( copy_target_files_ex _TARGET _COMPONENT _DESTINATION )# ARGN: The files
	if ( NOT "${_DESTINATION}" STREQUAL "" )
		set( _DESTINATION ${_DESTINATION}/ )
	endif ()
	foreach ( _FILE ${ARGN} )
		get_filename_component( _FILE ${_FILE} REALPATH )
		get_filename_component( _FILE_NAME ${_FILE} NAME )
		add_custom_command(
			TARGET ${_TARGET}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECTS_BINARIES_OUTPUT_DIR}/$<CONFIGURATION>/share/${_TARGET}/${_DESTINATION}
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_FILE} ${PROJECTS_BINARIES_OUTPUT_DIR}/$<CONFIGURATION>/share/${_TARGET}/${_DESTINATION}${_FILE_NAME}
		)
	endforeach ()
	install(
		FILES ${ARGN}
		COMPONENT ${_COMPONENT}
		DESTINATION share/${_TARGET}
		CONFIGURATIONS Release
	)
endfunction()

function( copy_target_files _TARGET _DESTINATION )# ARGN: The files
	copy_target_files_ex( ${_TARGET} ${_TARGET} ${_DESTINATION} ${ARGN} )
endfunction()

function( copy_target_directory_ex _TARGET _COMPONENT _SOURCE ) #ARGV3: _DESTINATION
	set( _DESTINATION "${ARGV3}" )
	add_custom_command(
		TARGET ${_TARGET}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECTS_BINARIES_OUTPUT_DIR}/$<CONFIGURATION>/share/${_TARGET}/${_DESTINATION}
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${_SOURCE} ${PROJECTS_BINARIES_OUTPUT_DIR}/$<CONFIGURATION>/share/${_TARGET}/${_DESTINATION}
	)
	install(
		DIRECTORY ${_SOURCE}
		COMPONENT ${_COMPONENT}
		DESTINATION share/${_TARGET}
		CONFIGURATIONS Release
	)
endfunction()

function( copy_target_directory _TARGET _SOURCE ) #ARGV2: _DESTINATION
	copy_target_directory_ex( ${_TARGET} ${_TARGET} ${_SOURCE} ${ARGV2} )
endfunction()
