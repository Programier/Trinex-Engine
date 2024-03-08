#
# Coverage.cmake
# -----------
#
# Find a coverage tool (OpenCppCoverage or gcov) and defines functions for using it.
#
# Usage
# ^^^^^^^^^^^^^^^^
#
# 1. Call `coverage_add_target(myTarget)` for each target that should be run to generate coverage
# 2. At the end call `coverage_add_merge_target(target outputFile)` to create a target which will create the combined coverage report
#
# Functions defined
# ^^^^^^^^^^^^^^^^
# coverage_add_target(target WORKING_DIRECTORY dir SOURCES [src, ...] MODULES [module, ...] ARGS [arg, ...])
#   Register `target` to be run with coverage tool.
#   WORKING_DIRECTORY -- Working directory in which to execute, defaults to the current binary dir
#   SOURCES           -- RegExps to specify source files to include, defaults to PROJECT_SOURCE_DIR
#   MODULES           -- RegExps to specify modules (binaries) to include, defaults to PROJECT_BINARY_DIR
#   EXCLUDES          -- RegExps to specify files to exclude
#   ARGS              -- Arguments to pass to the executable
#
# coverage_add_merge_target(target outputFile [FORMAT <format>])
#  Create `target` which merges all output from the previous `OpenCppCoverage_add_target` calls into `outputFile`.
#  A relative `outputFile` will be treated relative to the current binary dir.
#  FORMAT -- Output format (e.g. "html"). Defaults to "cobertura"
include( CMakeParseArguments )

option( PROJECTS_COVERAGE "Activate code coverage or not" OFF )
option( PROJECTS_COVERAGE_HTML_RESULTS "Enable HTML output for coverage merge target" OFF )

if ( PROJECTS_COVERAGE )
	if( MSVC )
		find_program( COVERAGE_TOOL_BINARY OpenCppCoverage.exe )

		if ( COVERAGE_TOOL_BINARY )
			execute_process( COMMAND "${COVERAGE_TOOL_BINARY}" --help ERROR_VARIABLE _out OUTPUT_QUIET )
			if( _out MATCHES "OpenCppCoverage Version: ([.0-9]+)" )
				set(COVERAGE_TOOL_VERSION ${CMAKE_MATCH_1})
			endif ()
		endif ()

		include( FindPackageHandleStandardArgs )
		find_package_handle_standard_args( COVERAGE_TOOL
			REQUIRED_VARS COVERAGE_TOOL_BINARY
			VERSION_VAR COVERAGE_TOOL_VERSION
		)

		if ( COVERAGE_TOOL_FOUND )
			function( coverage_add_target target )
				cmake_parse_arguments( PARSE_ARGV 1 ARG "VERBOSE" "WORKING_DIRECTORY" "SOURCES;MODULES;EXCLUDES;ARGS" )
				if ( ARG_UNPARSED_ARGUMENTS )
					message(FATAL_ERROR "Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
				endif ()

				get_property( counter GLOBAL PROPERTY COVERAGE_COUNTER )
				if ( NOT counter )
					set( counter 1 )
				else ()
					math( EXPR counter "${counter} + 1" )
				endif ()
				set( outputFile ${CMAKE_CURRENT_BINARY_DIR}/Coverage/cov-${counter}-${target}.bin )
				set( configFile ${CMAKE_CURRENT_BINARY_DIR}/Coverage/cov-${counter}-${target}.cfg )
				set_property( GLOBAL PROPERTY COVERAGE_COUNTER "${counter}" )
				set_property( GLOBAL APPEND PROPERTY COVERAGE_SOURCES "${outputFile}" )
				set_property( GLOBAL APPEND PROPERTY COVERAGE_TARGETS "${target}" )

				if ( NOT ARG_WORKING_DIRECTORY )
					set( ARG_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
				endif ()
				if ( NOT ARG_SOURCES )
					set( ARG_SOURCES ${PROJECT_SOURCE_DIR} )
				endif ()
				if ( NOT ARG_MODULES )
					set( ARG_SOURCES ${PROJECT_BINARY_DIR} )
				endif ()

				file( WRITE ${configFile} "export_type=binary:${outputFile}\n" )
				file( APPEND ${configFile} "cover_children=true\n" )
				set( args --working_dir $<TARGET_FILE_DIR:${target}> )
				list( APPEND args --config_file ${configFile} )
				if ( ARG_VERBOSE )
					list( APPEND args --verbose )
				else ()
					list( APPEND args --quiet )
				endif ()
				foreach( el IN LISTS ARG_EXCLUDES )
					list( APPEND args --excluded_sources ${el} )
				endforeach ()
				foreach( el IN LISTS ARG_SOURCES )
					list( APPEND args --sources ${el} )
				endforeach ()
				foreach ( el IN LISTS ARG_MODULES )
					list( APPEND args --modules ${el} )
				endforeach ()
				file( TO_NATIVE_PATH "${args}" args )

				add_custom_command( TARGET ${target}
					POST_BUILD
					COMMENT "Creating coverage for ${target}"
					COMMAND ${COVERAGE_TOOL_BINARY} ${args} -- $<TARGET_FILE:${target}> ${ARG_ARGS}
					VERBATIM
				)
			endfunction()

			function( coverage_add_merge_target target outputFile )
				cmake_parse_arguments( PARSE_ARGV 2 ARG "VERBOSE" "" "" )
				if ( PROJECTS_COVERAGE_HTML_RESULTS )
					set( ARG_FORMAT html )
				else ()
					set( ARG_FORMAT cobertura )
				endif ()

				get_property( targets GLOBAL PROPERTY COVERAGE_TARGETS )
				set_property( GLOBAL PROPERTY COVERAGE_TARGETS "" )
				get_property( sources GLOBAL PROPERTY COVERAGE_SOURCES )
				set_property( GLOBAL PROPERTY COVERAGE_SOURCES "" )
				get_filename_component( outputFile ${outputFile} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR} )

				set( configFile ${CMAKE_BINARY_DIR}/${target}Config.cfg )
				if ( PROJECTS_COVERAGE_HTML_RESULTS )
					set( commandOutput ${outputFile}/index.html )
					file( WRITE ${configFile} "export_type=html:${outputFile}\n" )
				else ()
					set( commandOutput ${outputFile}.xml )
					file( WRITE ${configFile} "export_type=cobertura:${commandOutput}\n" )
				endif ()
				set( args --config_file ${configFile} )
				if ( ARG_VERBOSE )
					list( APPEND args --verbose )
				else ()
					list( APPEND args --quiet )
				endif ()
				foreach( source IN LISTS sources )
					file( APPEND ${configFile} "input_coverage=${source}\n" )
				endforeach ()
				add_custom_command( OUTPUT ${commandOutput}
					DEPENDS ${sources}
					DEPENDS ${targets}
					COMMENT "Merging coverage data"
					COMMAND ${COVERAGE_TOOL_BINARY} ${args}
					VERBATIM
				)
				add_custom_target( ${target}
					DEPENDS ${commandOutput}
				)
			endfunction()
		endif ()
	else ()
		find_program( COVERAGE_TOOL_BINARY gcovr )

		include( FindPackageHandleStandardArgs )
		find_package_handle_standard_args( COVERAGE_TOOL
			REQUIRED_VARS COVERAGE_TOOL_BINARY
		)

		if ( COVERAGE_TOOL_FOUND )
			function( coverage_add_target target )
				cmake_parse_arguments( PARSE_ARGV 1 ARG "VERBOSE" "WORKING_DIRECTORY" "SOURCES;MODULES;EXCLUDES;ARGS" )
				if ( ARG_UNPARSED_ARGUMENTS )
					message(FATAL_ERROR "Invalid argument(s): ${ARG_UNPARSED_ARGUMENTS}")
				endif ()

				get_property( counter GLOBAL PROPERTY COVERAGE_COUNTER )
				if ( NOT counter )
					set( counter 1 )
				else ()
					math( EXPR counter "${counter} + 1" )
				endif ()
				set( outputFile ${CMAKE_CURRENT_BINARY_DIR}/Coverage/cov-${counter}-${target}.json )
				set( configFile ${CMAKE_CURRENT_BINARY_DIR}/Coverage/cov-${counter}-${target}.cfg )
				set_property( GLOBAL PROPERTY COVERAGE_COUNTER "${counter}" )
				set_property( GLOBAL APPEND PROPERTY COVERAGE_SOURCES "${outputFile}" )
				set_property( GLOBAL APPEND PROPERTY COVERAGE_TARGETS "${target}" )

				if ( NOT ARG_WORKING_DIRECTORY )
					set( ARG_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )
				endif ()
				if ( NOT ARG_SOURCES )
					file( TO_NATIVE_PATH ${PROJECT_SOURCE_DIR} ARG_SOURCES )
				endif ()
				if ( NOT ARG_MODULES )
					file( TO_NATIVE_PATH ${PROJECT_BINARY_DIR} ARG_MODULES )
				endif ()

				set( GCOVR_EXCLUDES "" )
				foreach(EXCLUDE ${ARG_EXCLUDES} )
					get_filename_component( EXCLUDE ${EXCLUDE} ABSOLUTE BASE_DIR ${PROJECT_SOURCE_DIR} )
					list( APPEND GCOVR_EXCLUDES "${EXCLUDE}" )
				endforeach()
				list( REMOVE_DUPLICATES GCOVR_EXCLUDES )

				file( WRITE ${configFile} "root = ${PROJECT_SOURCE_DIR}\n" )
				file( APPEND ${configFile} "json = yes\n" )
				file( APPEND ${configFile} "output = ${outputFile}\n" )
				if ( ARG_VERBOSE )
					file( APPEND ${configFile} "verbose = yes\n" )
				endif ()
				set( args ${PROJECT_BINARY_DIR} --config ${configFile} )
				foreach( el IN LISTS GCOVR_EXCLUDES )
					list( APPEND args --exclude ${el} )
				endforeach ()
				foreach( el IN LISTS ARG_SOURCES )
					file( TO_NATIVE_PATH "${el}" el )
					list( APPEND args --filter ${el} )
				endforeach ()
				file( TO_NATIVE_PATH "${args}" args )
				add_custom_command( TARGET ${target}
					POST_BUILD
					COMMENT "Creating coverage for ${target}"
					COMMAND $<TARGET_FILE:${target}> ${ARG_ARGS}
					COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Coverage
					COMMAND ${COVERAGE_TOOL_BINARY} ${args}
					VERBATIM
				)
			endfunction()

			function( coverage_add_merge_target target outputFile )
				cmake_parse_arguments( PARSE_ARGV 2 ARG "VERBOSE" "" "" )
				get_property( targets GLOBAL PROPERTY COVERAGE_TARGETS )
				set_property( GLOBAL PROPERTY COVERAGE_TARGETS "" )
				get_property( sources GLOBAL PROPERTY COVERAGE_SOURCES )
				set_property( GLOBAL PROPERTY COVERAGE_SOURCES "" )
				set( configFile ${CMAKE_BINARY_DIR}/${target}Config.cfg )
				get_filename_component( outputFile ${outputFile} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR} )

				file( WRITE ${configFile} "root = ${PROJECT_SOURCE_DIR}\n" )
				if ( PROJECTS_COVERAGE_HTML_RESULTS )
					set( commandOutput ${outputFile}/index.html )
					file( APPEND ${configFile} "html-details = yes\n" )
				else ()
					set( commandOutput ${outputFile}.xml )
					file( APPEND ${configFile} "xml = yes\n" )
				endif ()
				file( APPEND ${configFile} "output = ${commandOutput}\n" )
				if ( ARG_VERBOSE )
					file( APPEND ${configFile} "verbose = yes\n" )
				endif ()
				foreach( source IN LISTS sources )
					file( APPEND ${configFile} "add-tracefile = ${source}\n" )
				endforeach ()
				add_custom_command( OUTPUT ${commandOutput}
					DEPENDS ${targets}
					COMMENT "Merging coverage data"
					COMMAND ${CMAKE_COMMAND} -E make_directory ${outputFile}
					COMMAND ${COVERAGE_TOOL_BINARY} --config ${configFile}
					VERBATIM
				)
				add_custom_target( ${target}
					DEPENDS ${commandOutput}
				)
			endfunction()
		endif ()
	endif ()
endif ()

function( target_add_coverage_flags target )
	if ( PROJECTS_COVERAGE )
		if( NOT MSVC )
			set( COVERAGE_COMPILER_FLAGS "-g --coverage -fprofile-abs-path" CACHE INTERNAL "" )
			separate_arguments( _flag_list NATIVE_COMMAND "${COVERAGE_COMPILER_FLAGS}" )
			target_compile_options( ${target} PRIVATE ${_flag_list} )
			target_link_options( ${target} PRIVATE --coverage )
		endif ()
	endif ()
endfunction()
