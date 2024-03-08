#--------------------------------------------------------------------------------------------------
#	Function :	DumpCompilerVersion
# 	Function which gives the GNU Compiler version, used to build name of project's libs
#--------------------------------------------------------------------------------------------------
function( DumpCompilerVersion OUTPUT_VERSION)
	exec_program( ${CMAKE_CXX_COMPILER}
		ARGS ${CMAKE_CXX_COMPILER_ARG1} -dumpversion 
		OUTPUT_VARIABLE COMPILER_VERSION
	)
	string( REGEX 
		REPLACE
		"([0-9])\\.([0-9])(\\.[0-9])?" "\\1\\2"
		COMPILER_VERSION_REPL
		${COMPILER_VERSION}
	)
	if ( ( ${COMPILER_VERSION_REPL} STREQUAL "" )
		OR ( ${COMPILER_VERSION_REPL} STREQUAL ${COMPILER_VERSION} ) )
		string( REGEX 
			REPLACE
			"([0-9])(\\.[0-9])?(\\.[0-9])?" "\\10"
			COMPILER_VERSION_REPL
			${COMPILER_VERSION}
		)
	endif ()
	set( ${OUTPUT_VERSION} ${COMPILER_VERSION_REPL} PARENT_SCOPE )
endfunction()

#--------------------------------------------------------------------------------------------------
#	MSVC specifics
#--------------------------------------------------------------------------------------------------
if( MSVC )
	# We remove the existing /Zm flag from CMAKE_CXX_FLAGS
	string( REGEX REPLACE ".*(/Zm[0-9]+).*" "\\1" ZmFlag ${CMAKE_CXX_FLAGS} )
	if( (NOT "${ZmFlag}" STREQUAL "") AND (NOT "${CMAKE_CXX_FLAGS}" STREQUAL "${ZmFlag}") )
		string( REPLACE ${ZmFlag} "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} )
	endif()
endif()
