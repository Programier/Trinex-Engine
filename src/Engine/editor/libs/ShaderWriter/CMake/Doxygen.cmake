find_package( Doxygen QUIET COMPONENTS doxygen dot dia )
option( PROJECTS_GENERATE_DOC "Generate Doxygen documentation" FALSE )

#--------------------------------------------------------------------------------------------------
#	Function :	target_add_doc
#	Generates doc for given target
#--------------------------------------------------------------------------------------------------
function( target_add_doc_ex TARGET_NAME LANGUAGE EXT_LIST LOOKUP_FOLDER )
	if ( DOXYGEN_FOUND AND PROJECTS_GENERATE_DOC )
		find_package( HTMLHelp )
		set( DOXYGEN_INPUT_FOLDER ${LOOKUP_FOLDER} )
		set( TARGET_VERSION_MAJOR ${${PROJECT_NAME}_VERSION_MAJOR} )
		set( TARGET_VERSION_MINOR ${${PROJECT_NAME}_VERSION_MINOR} )
		set( TARGET_VERSION_BUILD ${${PROJECT_NAME}_VERSION_BUILD} )
		if ( EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Doc/${TARGET_NAME}Doxygen.css )
			set( TARGET_STYLESHEET ${CMAKE_CURRENT_SOURCE_DIR}/Doc/${TARGET_NAME}Doxygen.css )
		endif ()
		set( FOLDER_LIST
			${CMAKE_CURRENT_SOURCE_DIR}/Doc/${TARGET_NAME}
			${CMAKE_CURRENT_SOURCE_DIR}/Doc
			${CMAKE_SOURCE_DIR}/Doc/${TARGET_NAME}
			${CMAKE_SOURCE_DIR}/Doc
		)
		if ( "${LANGUAGE}" STREQUAL "" )
			set( CHM_NAME ${TARGET_NAME}.chm )
			set( DOXYGEN_INPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.Doxyfile )
			set( DOXYGEN_OUTPUT_DIR ${PROJECTS_DOCUMENTATION_OUTPUT_DIR}/${TARGET_NAME}/v${${TARGET_NAME}_VERSION_MAJOR}.${${TARGET_NAME}_VERSION_MINOR}.${${TARGET_NAME}_VERSION_BUILD} )
			set( DOXYGEN_TARGET_NAME ${TARGET_NAME}_Doc )
			set( DOXYGEN_DOC_FILE ${PROJECTS_DOCUMENTATION_OUTPUT_DIR}/${TARGET_NAME}/${CHM_NAME} )
			set( DOXYGEN_INST_DIR share/doc/${TARGET_NAME} )
		else ()
			if ( LANGUAGE STREQUAL "English" )
				set( SHORT_LANGUAGE "EN" )
			elseif ( LANGUAGE STREQUAL "French" )
				set( SHORT_LANGUAGE "FR" )
			endif ()
			set( CHM_NAME ${TARGET_NAME}-${LANGUAGE}.chm )
			set( DOXYGEN_INPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_${LANGUAGE}.Doxyfile )
			set( DOXYGEN_OUTPUT_DIR ${PROJECTS_DOCUMENTATION_OUTPUT_DIR}/${TARGET_NAME}/${SHORT_LANGUAGE}/v${${TARGET_NAME}_VERSION_MAJOR}.${${TARGET_NAME}_VERSION_MINOR}.${${TARGET_NAME}_VERSION_BUILD} )
			set( DOXYGEN_TARGET_NAME ${TARGET_NAME}_${LANGUAGE}_Doc )
			set( DOXYGEN_DOC_FILE ${PROJECTS_DOCUMENTATION_OUTPUT_DIR}/${CHM_NAME} )
			set( DOXYGEN_INST_DIR share/doc/${LANGUAGE}/${TARGET_NAME} )
		endif ()
		set( _FOUND OFF )
		foreach( FOLDER ${FOLDER_LIST} )
			if ( NOT _FOUND )
				set( FILEPATH ${FOLDER}/${TARGET_NAME}.Doxyfile )
				if ( EXISTS ${FILEPATH} )
					file( COPY
						${FILEPATH}
						DESTINATION ${DOXYGEN_INPUT_FILE}
					)
					set( _FOUND ON )
				endif ()
				set( FILEPATH ${FOLDER}/${TARGET_NAME}.Doxyfile.in )
				if ( EXISTS ${FILEPATH} )
					configure_file(
						${FILEPATH}
						${DOXYGEN_INPUT_FILE}
						NEWLINE_STYLE LF
					)
					set( _FOUND ON )
				endif ()
			endif ()
		endforeach()
		if ( NOT ${_FOUND} )
			message( SEND_ERROR "Couldn't find a doxyfile or a template doxyfile" )
		endif ()
		file( MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR} )
		add_custom_target(
			${DOXYGEN_TARGET_NAME}
			COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_INPUT_FILE}
			COMMENT "Building Doxygen documentation for ${TARGET_NAME}"
			VERBATIM
		)
		set_property( TARGET ${DOXYGEN_TARGET_NAME} PROPERTY FOLDER "Documentation/${TARGET_NAME}" )
		if ( EXISTS ${DOXYGEN_DOC_FILE} )
			install(
				FILES ${DOXYGEN_DOC_FILE}
				DESTINATION ${DOXYGEN_INST_DIR}
				COMPONENT ${DOXYGEN_TARGET_NAME}
				CONFIGURATIONS Release
			)
		endif ()
	endif ()
endfunction( target_add_doc_ex )

#--------------------------------------------------------------------------------------------------
#	Macro :	add_doc
#	Generates doc for current project
#--------------------------------------------------------------------------------------------------
macro( target_add_doc TARGET_NAME LANGUAGE EXT_LIST )
	target_add_doc_ex( ${TARGET_NAME} "${LANGUAGE}" "${EXT_LIST}" ${CMAKE_CURRENT_SOURCE_DIR}/Src )
endmacro( target_add_doc )

macro( add_doc LANGUAGE EXT_LIST )
	target_add_doc( ${PROJECT_NAME} "${LANGUAGE}" "${EXT_LIST}" )
endmacro( add_doc )
