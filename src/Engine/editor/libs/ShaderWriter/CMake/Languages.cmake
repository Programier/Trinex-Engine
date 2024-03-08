#--------------------------------------------------------------------------------------------------
#	Macro : copy_languages
#	Used to copy language files for TARGET_NAME from SRC_FOLDER to DST_FOLDER
#--------------------------------------------------------------------------------------------------
macro( copy_languages_ex TARGET_NAME COMPONENT_NAME SRC_FOLDER DST_FOLDER LANGUAGES )
	# Copy each language file into each destination folder
	foreach( LANGUAGE ${LANGUAGES} )
		set( _FILE ${SRC_FOLDER}/po/${LANGUAGE}/${TARGET_NAME}.mo )
		add_custom_command(
			TARGET ${TARGET_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECTS_BINARIES_OUTPUT_DIR}/$<CONFIGURATION>/share/${DST_FOLDER}/${LANGUAGE}
			COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_FILE} ${PROJECTS_BINARIES_OUTPUT_DIR}/$<CONFIGURATION>/share/${DST_FOLDER}/${LANGUAGE}
			COMMENT "Copying ${LANGUAGE} translation file"
		)
		install(
			FILES ${_FILE}
			COMPONENT ${COMPONENT_NAME}
			DESTINATION share/${DST_FOLDER}/${LANGUAGE}/
			CONFIGURATIONS Release
		)
	endforeach()
endmacro()

macro( copy_languages TARGET_NAME SRC_FOLDER DST_FOLDER LANGUAGES )
	copy_languages_ex( ${TARGET_NAME}
		${TARGET_NAME}
		${SRC_FOLDER}
		${DST_FOLDER}
		${LANGUAGES}
	)
endmacro()
