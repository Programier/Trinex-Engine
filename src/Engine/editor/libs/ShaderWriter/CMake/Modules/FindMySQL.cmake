
# *******************************<+>***************************************
#
# File        : FindMySQL.cmake
#
# Description : Find the native MySQL Client and MySQL Connector C++ includes and library.
#               This modules defines the following variables:
#                   MySQL_ROOT_DIR                 : The MySQL root directory.
#                   MySQL_INCLUDE_DIR              : MySQL include directory.
#                   MySQL_LIBRARIIES               : Contains the libraries.
#                   MySQL_client_LIBRARY_DEBUG     : Contains the client debug library.
#                   MySQL_client_LIBRARY_RELEASE   : Contains the client release library.
#
#
# *************************************************************************
# Contributors :
#    Version 1.0: 29/01/13, Author: sdoremus, Original hack
# *********************************<+>*************************************

if ( MYSQL_STATIC )
	set( _LIBTYPE STATIC )
else ()
	set( _LIBTYPE DYNAMIC )
endif ()

set( SUPPORTED_MySQL_SERVERS "5.2" "5.3" "5.4" "5.5" "5.6" )
set( SUPPORTED_MySQL_C_CONNECTORS "5.2" "5.3" "5.4" "5.5" "5.6" "5.7" "5.8" "5.9" "6.0" "6.1" )
set( CONFIGURATION x86 )
set( MySQL_DIR "C:/Program Files (x86)/MySQL" )

if ( CMAKE_CL_64 )
	set( CONFIGURATION x64 )
	set( MySQL_DIR "C:/Program Files/MySQL" )
elseif ( CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set( CONFIGURATION x64 )
	set( MySQL_DIR "C:/Program Files/MySQL" )
endif ()

set( MySQL_SERVER_DIR "${MySQL_DIR}/MySQL Server" )
set( MySQL_C_CONNECTOR_DIR "${MySQL_DIR}/MySQL Connector C" )

if ( NOT MySQL_FIND_COMPONENTS )
	set( MySQL_FIND_COMPONENTS client cppconn )
endif ()

if ( WIN32 )
	foreach( SERVER ${SUPPORTED_MySQL_SERVERS} )
		if ( NOT ACTUAL_MySQL_DIR )
			set( DIR "${MySQL_SERVER_DIR} ${SERVER}" )
			if( EXISTS "${DIR}/" )
				set( ACTUAL_MySQL_DIR ${DIR} )
			endif()
		endif()
	endforeach()
	foreach( SERVER ${SUPPORTED_MySQL_C_CONNECTORS} )
		if ( NOT ACTUAL_MySQL_DIR )
			set( DIR "${MySQL_C_CONNECTOR_DIR} ${SERVER}" )
			if( EXISTS "${DIR}/" )
				set( ACTUAL_MySQL_DIR ${DIR} )
			endif()
		endif()
	endforeach()
endif ()

foreach( _COMPONENT ${MySQL_FIND_COMPONENTS} )
	if ( "cppconn" STREQUAL "${_COMPONENT}" )
		set( STATIC_LIB_SUFFIX "-static" )
		set( MySQL_${_COMPONENT}_ROOT_DIR
			"${MySQL_${_COMPONENT}_ROOT_DIR}"
			CACHE
			PATH
			"Path to search for MySQL Connector C++." )
		find_path( MySQL_${_COMPONENT}_ROOT_DIR 
			NAMES   include/mysql_connection.h
			DOC     "The MySQL Connector C++ include directory"
		)
		find_path( MySQL_${_COMPONENT}_INCLUDE_DIR 
			NAMES   mysql_connection.h
			HINTS   ${MySQL_${_COMPONENT}_ROOT_DIR}/include
			DOC     "The MySQL Connector C++ include directory"
		)
		
		if ( NOT MySQL_${_COMPONENT}_INCLUDE_DIR )
			message( STATUS "MySQL Connector C++ include directory not found!" )
		endif ()
	else ()
		if ( "client" STREQUAL "${_COMPONENT}" )
			set( ADDITIONAL_LIB_NAME libmysql )
			if ( MSVC10 )
				set( FOLDER_SUFFIX "vs10" )
			elseif ( MSVC11 )
				set( FOLDER_SUFFIX "vs11" )
			elseif ( MSVC12 )
				set( FOLDER_SUFFIX "vs12" )
			endif ()
		endif ()
		set( STATIC_LIB_SUFFIX "" )
		if ( WIN32 )
			set( MySQL_LINK_FLAGS "/NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libcmtd.lib" )
			if ( ACTUAL_MySQL_DIR )
				set( MySQL_${_COMPONENT}_ROOT_DIR
					"${ACTUAL_MySQL_DIR}"
					CACHE
					PATH
					"Path to search for MySQL Client."
					FORCE
				)
			endif ()
		else ()
			find_path( MySQL_${_COMPONENT}_ROOT_DIR include/mysql/mysql.h
				HINTS
				PATHS
					/usr/local
					/usr
			)
		endif ()

		find_path( MySQL_${_COMPONENT}_INCLUDE_DIR mysql.h
			HINTS
				${MySQL_${_COMPONENT}_ROOT_DIR}/include
				${MySQL_${_COMPONENT}_ROOT_DIR}/include/mysql
			DOC
				"The MySQL Client include directory"
		)

		if ( NOT MySQL_${_COMPONENT}_INCLUDE_DIR )
			message( STATUS "MySQL Client include directory not found!" )
		endif ()
	endif ()
	if ( ${_LIBTYPE} STREQUAL "DYNAMIC" )
		find_library( MySQL_${_COMPONENT}_LIBRARY_RELEASE
			NAMES
				${ADDITIONAL_LIB_NAME}
				mysql${_COMPONENT}
			PATHS
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib/opt
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib/${FOLDER_SUFFIX}
			DOC "The MySQL ${_COMPONENT} dynamic library release"
		)
		find_library( MySQL_${_COMPONENT}_LIBRARY_DEBUG
			NAME
				${ADDITIONAL_LIB_NAME}d
				mysql${_COMPONENT}d
			PATHS
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib/opt
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib/${FOLDER_SUFFIX}
			DOC "The MySQL ${_COMPONENT} dynamic library debug"
		)
	else ()
		find_library( MySQL_${_COMPONENT}_LIBRARY_RELEASE
			NAMES
				${ADDITIONAL_LIB_NAME}${STATIC_LIB_SUFFIX}
				mysql${_COMPONENT}${STATIC_LIB_SUFFIX}
			PATHS
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib/opt
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib/${FOLDER_SUFFIX}
			DOC "The MySQL ${_COMPONENT} static library release"
		)
		find_library( MySQL_${_COMPONENT}_LIBRARY_DEBUG
			NAMES
				${ADDITIONAL_LIB_NAME}${STATIC_LIB_SUFFIX}d
				${ADDITIONAL_LIB_NAME}d${STATIC_LIB_SUFFIX}
				mysql${_COMPONENT}${STATIC_LIB_SUFFIX}d
				mysql${_COMPONENT}d${STATIC_LIB_SUFFIX}
			PATHS
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib/opt
				${MySQL_${_COMPONENT}_ROOT_DIR}/lib/${FOLDER_SUFFIX}
			DOC "The MySQL ${_COMPONENT} static library debug"
		)
	endif ()
	mark_as_advanced(
		MySQL_${_COMPONENT}_LIBRARY_RELEASE
		MySQL_${_COMPONENT}_LIBRARY_DEBUG
	)
	if( MySQL_${_COMPONENT}_LIBRARY_RELEASE )
		if( MySQL_${_COMPONENT}_LIBRARY_DEBUG )
			set( MySQL_${_COMPONENT}_LIBRARIES optimized;${MySQL_${_COMPONENT}_LIBRARY_RELEASE};debug;${MySQL_${_COMPONENT}_LIBRARY_DEBUG} )
		else()
			set( MySQL_${_COMPONENT}_LIBRARIES ${MySQL_${_COMPONENT}_LIBRARY_RELEASE} )
		endif()
	endif()
	if ( MySQL_${_COMPONENT}_LIBRARIES )
		set( _MySQL_LIBRARIES
			${MySQL_LIBRARIES}
			${MySQL_${_COMPONENT}_LIBRARIES}
		)
	endif ()
	set( MySQL_INCLUDE_DIRS
		${MySQL_INCLUDE_DIRS}
		${MySQL_${_COMPONENT}_INCLUDE_DIR}
	)
endforeach()

if ( _MySQL_LIBRARIES )
	set( MySQL_LIBRARIES "${_MySQL_LIBRARIES}" CACHE STRING "" )
	mark_as_advanced(
		MySQL_LIBRARIES
	)
endif ()

find_program( MySQL_COMMAND mysql
	PATHS
		/usr/bin
		/usr/local/bin
		${ACTUAL_MySQL_DIR}/bin
)

# -----------------------------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set MySQL_FOUND to true
# if all listed variables are TRUE
# ---------------------------------------------------------------------
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( MYSQL DEFAULT_MSG MySQL_LIBRARIES MySQL_INCLUDE_DIRS )
