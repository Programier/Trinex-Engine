#
# Taken from https://github.com/iboB/cmake-pch
#
# Sets a precompiled header for a given target
# Args:
# TARGET_NAME - Name of the target. Only valid after add_library or add_executable
# PRECOMPILED_HEADER - Header file to precompile
# PRECOMPILED_SOURCE - MSVC specific source to do the actual precompilation. Ignored on other platforms
#
# Example Usage
# add_executable(myproj
#   src/myproj.pch.h
#   src/myproj.pch.cpp
#   src/main.cpp
#   ...
#   src/z.cpp
#   )
# add_precompiled_header(myproj src/myproj.pch.h src/myproj.pch.cpp)
#
macro( target_add_precompiled_header_modern TARGET_NAME PRECOMPILED_HEADER )
	target_precompile_headers( ${TARGET_NAME}
		PRIVATE
			${PRECOMPILED_HEADER}
	)
endmacro( target_add_precompiled_header_modern )

macro( target_add_precompiled_header_legacy TARGET_NAME PRECOMPILED_HEADER PRECOMPILED_SOURCE )
	get_filename_component( PRECOMPILED_HEADER_NAME ${PRECOMPILED_HEADER} NAME )

	if ( MSVC )
		get_filename_component( PRECOMPILED_SOURCE_REALPATH ${PRECOMPILED_SOURCE} REALPATH )

		get_filename_component( PRECOMPILED_HEADER_PATH ${PRECOMPILED_HEADER} DIRECTORY )
		target_include_directories( ${TARGET_NAME} PRIVATE ${PRECOMPILED_HEADER_PATH} ) # fixes occasional IntelliSense glitches

		get_filename_component( PRECOMPILED_HEADER_WE ${PRECOMPILED_HEADER} NAME_WE )
		set( PRECOMPILED_BINARY "$(IntDir)/${PRECOMPILED_HEADER_WE}.pch" )

		get_target_property( SOURCE_FILES ${TARGET_NAME} SOURCES )
		set( SOURCE_FILE_FOUND FALSE )

		foreach ( SOURCE_FILE ${SOURCE_FILES} )
			if ( SOURCE_FILE MATCHES \\.\(cc|cxx|cpp\)$ )
				get_filename_component( SOURCE_REAL_PATH ${SOURCE_FILE} REALPATH )

				if ( ${PRECOMPILED_SOURCE_REALPATH} STREQUAL ${SOURCE_REAL_PATH} )
					# Set source file to generate header
					set_source_files_properties( ${SOURCE_FILE}
						PROPERTIES
						COMPILE_FLAGS "/Yc\"${PRECOMPILED_HEADER_NAME}\" /Fp\"${PRECOMPILED_BINARY}\""
						OBJECT_OUTPUTS "${PRECOMPILED_BINARY}"
					)
					set( SOURCE_FILE_FOUND TRUE )
				else ()
					# Set and automatically include precompiled header
					set_source_files_properties( ${SOURCE_FILE}
						PROPERTIES
						COMPILE_FLAGS "/Yu\"${PRECOMPILED_HEADER_NAME}\" /Fp\"${PRECOMPILED_BINARY}\" /FI\"${PRECOMPILED_HEADER_NAME}\""
						OBJECT_DEPENDS "${PRECOMPILED_BINARY}"
					)
				endif ()
			endif ()
		endforeach ()

		if ( NOT SOURCE_FILE_FOUND )
			target_sources( ${TARGET_NAME}
				PRIVATE
					${PRECOMPILED_HEADER}
					${PRECOMPILED_SOURCE} )
			set_source_files_properties( ${PRECOMPILED_SOURCE}
				PROPERTIES
				COMPILE_FLAGS "/Yc\"${PRECOMPILED_HEADER_NAME}\" /Fp\"${PRECOMPILED_BINARY}\""
				OBJECT_OUTPUTS "${PRECOMPILED_BINARY}"
			)
		endif ( NOT SOURCE_FILE_FOUND )
	elseif ( CMAKE_GENERATOR STREQUAL Xcode )
		set_target_properties( ${TARGET_NAME}
			PROPERTIES
			XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${PRECOMPILED_HEADER}"
			XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
		)
	elseif ( CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
		# Create and set output directory.
		set( OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PRECOMPILED_HEADER_NAME}.gch" )
		make_directory( ${OUTPUT_DIR} )
		set( OUTPUT_NAME "${OUTPUT_DIR}/${PRECOMPILED_HEADER_NAME}.gch" )

		# Export compiler flags via a generator to a response file
		set( PCH_FLAGS_FILE "${OUTPUT_DIR}/${PRECOMPILED_HEADER_NAME}.rsp")
		set( _include_directories "$<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>" )
		set( _compile_definitions "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_DEFINITIONS>" )
		set( _compile_flags "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_FLAGS>" )
		set( _compile_options "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_OPTIONS>" )
		set( _include_directories "$<$<BOOL:${_include_directories}>:-I$<JOIN:${_include_directories},\n-I>\n>" )
		set( _compile_definitions "$<$<BOOL:${_compile_definitions}>:-D$<JOIN:${_compile_definitions},\n-D>\n>" )
		set( _compile_flags "$<$<BOOL:${_compile_flags}>:$<JOIN:${_compile_flags},\n>\n>" )
		set( _compile_options "$<$<BOOL:${_compile_options}>:$<JOIN:${_compile_options},\n>\n>" )
		file( GENERATE OUTPUT "${PCH_FLAGS_FILE}" CONTENT "${_compile_definitions}${_include_directories}${_compile_flags}${_compile_options}\n" )

		# Gather global compiler options, definitions, etc.
		string( TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" CXX_FLAGS )
		set( COMPILER_FLAGS "${${CXX_FLAGS}} ${CMAKE_CXX_FLAGS}" )
		separate_arguments( COMPILER_FLAGS )

		# Add a custom target for building the precompiled header.
		# HACK: Add explicit -std=${CXX_STD} to work around an ugly issue for CMake 3.2+
		# which prevents us from actually scraping the -std=??? flag set by target_compile_features
		set( CXX_STD c++17 )
		add_custom_command(
			OUTPUT ${OUTPUT_NAME}
			COMMAND ${CMAKE_CXX_COMPILER} @${PCH_FLAGS_FILE} ${COMPILER_FLAGS} -x c++-header -std=${CXX_STD} -o ${OUTPUT_NAME} ${PRECOMPILED_HEADER}
			DEPENDS ${PRECOMPILED_HEADER})
		add_custom_target( ${TARGET_NAME}_gch DEPENDS ${OUTPUT_NAME} )
		add_dependencies( ${TARGET_NAME} ${TARGET_NAME}_gch )

		# set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-include ${PRECOMPILED_HEADER_NAME} -Winvalid-pch")
		get_target_property( SOURCE_FILES ${TARGET_NAME} SOURCES )
		get_target_property( asdf ${TARGET_NAME} COMPILE_FLAGS )

		foreach( SOURCE_FILE ${SOURCE_FILES} )
			if ( SOURCE_FILE MATCHES \\.\(c|cc|cxx|cpp\)$ )
				set_source_files_properties(${SOURCE_FILE} PROPERTIES
					COMPILE_FLAGS "-include ${OUTPUT_DIR}/${PRECOMPILED_HEADER_NAME} -Winvalid-pch"
				)
			endif ()
		endforeach ()
	else ()
		message( FATAL_ERROR "Unknown generator for add_precompiled_header." )
	endif ()
endmacro( target_add_precompiled_header_legacy )

if ( MSVC OR ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0" )
	option( PROJECTS_USE_PRECOMPILED_HEADERS "Use precompiled headers" ON )
else ()
	option( PROJECTS_USE_PRECOMPILED_HEADERS "Use precompiled headers" OFF )
endif ()

macro( target_add_precompiled_header TARGET_NAME PRECOMPILED_HEADER PRECOMPILED_SOURCE )
	if ( PROJECTS_USE_PRECOMPILED_HEADERS )
		if ( ${CMAKE_VERSION} VERSION_LESS "3.16.0" )
			target_add_precompiled_header_legacy( ${TARGET_NAME}
				${PRECOMPILED_HEADER}
				${PRECOMPILED_SOURCE}
			)
		else ()
			target_add_precompiled_header_modern( ${TARGET_NAME}
				${PRECOMPILED_HEADER}
			)
		endif ()
	endif ()
endmacro( target_add_precompiled_header )
