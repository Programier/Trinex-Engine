# Locate DatabaseEngine library
# This module defines :
#  	Database_LIBRARIES :	The library to link against
#  	Database_FOUND :		If false, do not try to link to Database
#  	Database_INCLUDE_DIR :	Where to find headers.
#	Database_LIBRARY_DIR :	Where to find the library
# You just have to set Database_ROOT_DIR in your CMakeCache.txt if it is not found.
# If Database was built in a special folder, you can specify it in Database_LIBRARY_DIR

FIND_PATH(Database_ROOT_DIR include/Database/Database.h
  HINTS
  PATHS
  C: D: E: F: G: H: I: J: K: L: M: N: O: P: Q: R: S: T: U: V: W: X: Y: Z:
  /usr/local
  /usr
)

if (Database_ROOT_DIR)
	FIND_PATH(Database_INCLUDE_DIR Database/Database.h
	  HINTS
	  PATH_SUFFIXES include
	  PATHS
	  ${Database_ROOT_DIR}
	)

	set( Database_LIBRARY_NAME "Database")
	
	if ( WIN32 )
		if (NOT Database_LIBRARY_DIR)
			if (CMAKE_BUILD_TYPE)
				set( _Database_LIBRARY_DIR "${Database_ROOT_DIR}/lib/${CMAKE_BUILD_TYPE}")
			else ()
				set( _Database_LIBRARY_DIR "${Database_ROOT_DIR}/lib")
			endif ()
		endif ()
	
		if(NOT (MSVC OR BORLAND))
			set( Database_LIBRARY_NAME "lib${Database_LIBRARY_NAME}")
		endif()
	else ()
		if (NOT Database_LIBRARY_DIR)
			set( _Database_LIBRARY_DIR "${Database_ROOT_DIR}/lib")
		endif ()
	endif ()

	set( Database_LIBRARY_DIR ${_Database_LIBRARY_DIR} CACHE FILEPATH "Database library directory")

	find_library( Database_LIBRARY_DEBUG
		NAMES ${Database_LIBRARY_NAME}d
		PATHS
			${Database_LIBRARY_DIR}
	)
	
	if ( WIN32 OR Database_LIBRARY_DEBUG )
		find_library( Database_LIBRARY_RELEASE
			NAMES ${Database_LIBRARY_NAME}
			PATHS
				${Database_LIBRARY_DIR}
		)

		if ( Database_LIBRARY_DEBUG AND Database_LIBRARY_RELEASE )
			set( _Database_LIBRARIES optimized ${Database_LIBRARY_RELEASE} debug ${Database_LIBRARY_DEBUG} )
		elseif ( Database_LIBRARY_RELEASE )
			set( _Database_LIBRARIES ${Database_LIBRARY_RELEASE} )
		elseif ( Database_LIBRARY_DEBUG )
			set( _Database_LIBRARIES ${Database_LIBRARY_DEBUG} )
		endif ()
		set( Database_LIBRARIES ${_Database_LIBRARIES} CACHE STRING "Database libraries" FORCE )
		MARK_AS_ADVANCED( Database_LIBRARY_DEBUG Database_LIBRARY_RELEASE )
	else ()
		find_library( Database_LIBRARY
			NAMES ${Database_LIBRARY_NAME}
			PATHS
				${Database_LIBRARY_DIR}
		)

		if ( Database_LIBRARY )
			set( Database_LIBRARIES ${Database_LIBRARY} CACHE STRING "Database libraries" FORCE )
		endif ()
		MARK_AS_ADVANCED( Database_LIBRARY )
	endif ()
endif ()

FIND_PACKAGE_HANDLE_STANDARD_ARGS( Database DEFAULT_MSG Database_LIBRARIES Database_INCLUDE_DIR )
