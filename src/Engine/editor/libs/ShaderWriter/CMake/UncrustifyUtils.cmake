find_package( Uncrustify )
option( PROJECTS_USE_PRETTY_PRINTING "Enable Uncrustify" TRUE )

if ( UNCRUSTIFY_FOUND )
	message( STATUS "+ Found Uncrustify" )
endif()

function( add_target_format TARGET_NAME EXT_LIST )
	if ( UNCRUSTIFY_FOUND AND PROJECTS_USE_PRETTY_PRINTING )
		# Retrieve project source files
		get_property( SOURCE_FILES
			TARGET ${TARGET_NAME}
			PROPERTY SOURCES
		)
		set( PROJECT_FILES )
		foreach( SOURCE ${SOURCE_FILES} )
			get_source_file_property( SOURCE_LOC "${SOURCE}" LOCATION )
			get_filename_component( SOURCE_EXT ${SOURCE_LOC} EXT )
			# Add only files with wanted extension
			foreach( EXTENSION ${EXT_LIST} )
				if ( ${SOURCE_EXT} STREQUAL ${EXTENSION} )
					set( PROJECT_FILES "${PROJECT_FILES}\n${SOURCE_LOC}" )
					break()
				endif()
			endforeach()
		endforeach()
		configure_file(
			${CMAKE_TEMPLATES_DIR}/UncrustifyFiles.in
			${CMAKE_CURRENT_BINARY_DIR}/UncrustifyFiles
			NEWLINE_STYLE LF
		)
		# Add test
		add_test(
			NAME ${TARGET_NAME}_Uncrustify
			COMMAND ${UNCRUSTIFY_BINARY} -c ${CMAKE_TEMPLATES_DIR}/format.cfg -F ${CMAKE_CURRENT_BINARY_DIR}/UncrustifyFiles --replace --no-backup -l CPP
		)
	endif()
endfunction( add_target_format )
