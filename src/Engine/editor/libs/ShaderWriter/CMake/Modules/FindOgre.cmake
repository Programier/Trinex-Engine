# FindOgre
# ------------
#
# Locate Ogre library and plugins
#
# This module defines
#
# ::
#
#   Ogre_LIBRARIES, the libraries to link against
#   Ogre_FOUND, if false, do not try to link to Ogre
#   Ogre_INCLUDE_DIR, where to find headers.
#   Ogre_VERSION, contains the found Ogre version number
#

if ( NOT Ogre_FIND_COMPONENTS )
	set( Ogre_FIND_COMPONENTS Main Overlay RenderSystem_GL )
endif ()

find_path(Ogre_ROOT_DIR include/OGRE/Ogre.h include/Ogre.h
	HINTS
	PATH_SUFFIXES include Ogre/OgreMain
	PATHS
	/usr/local/include
	/usr/include
)

find_path( Ogre_INCLUDE_DIR Ogre.h
	HINTS
	PATH_SUFFIXES include/OGRE include
	PATHS
	${Ogre_ROOT_DIR}
)

if ( Ogre_INCLUDE_DIR )
	set( _Ogre_INCLUDE_DIRS ${Ogre_INCLUDE_DIR} )
	file( STRINGS ${Ogre_INCLUDE_DIR}/OgrePrerequisites.h _OGRE_VERSION_MAJOR REGEX "[^#]*define OGRE_VERSION_MAJOR ([0-9]*)" )
	file( STRINGS ${Ogre_INCLUDE_DIR}/OgrePrerequisites.h _OGRE_VERSION_MINOR REGEX "[^#]*define OGRE_VERSION_MINOR ([0-9]*)" )
	file( STRINGS ${Ogre_INCLUDE_DIR}/OgrePrerequisites.h _OGRE_VERSION_PATCH REGEX "[^#]*define OGRE_VERSION_PATCH ([0-9]*)" )
	string(REGEX REPLACE "[^0-9]" "" Ogre_VERSION_MAJOR ${_OGRE_VERSION_MAJOR} )
	string(REGEX REPLACE "[^0-9]" "" Ogre_VERSION_MINOR ${_OGRE_VERSION_MINOR} )
	string(REGEX REPLACE "[^0-9]" "" Ogre_VERSION_PATCH ${_OGRE_VERSION_PATCH} )
	set( Ogre_VERSION "${Ogre_VERSION_MAJOR}.${Ogre_VERSION_MINOR}.${Ogre_VERSION_PATCH}" )

	foreach( COMPONENT ${Ogre_FIND_COMPONENTS} )
		find_path( Ogre_${COMPONENT}_INCLUDE_DIR Ogre${COMPONENT}.h
			HINTS
			PATH_SUFFIXES
				${COMPONENT}
			PATHS
			${Ogre_INCLUDE_DIR}
		)
		if ( Ogre_${COMPONENT}_INCLUDE_DIR )
			set( _Ogre_INCLUDE_DIRS
				${_Ogre_INCLUDE_DIRS}
				${Ogre_${COMPONENT}_INCLUDE_DIR}
			)
		endif ()
	endforeach ()

	if ( Ogre_FIND_VERSION )
		if ( ${Ogre_FIND_VERSION} VERSION_GREATER ${Ogre_VERSION} )
			message( SEND_ERROR "Found version for Ogre (${Ogre_VERSION}) is less than required (${Ogre_FIND_VERSION})" )
		endif ()
	endif ()

	set( Ogre_INCLUDE_DIRS ${_Ogre_INCLUDE_DIRS} CACHE STRING "Ogre include directories" )
	mark_as_advanced( Ogre_INCLUDE_DIRS )
endif ()

if ( MSVC )
	foreach( _COMPONENT ${Ogre_FIND_COMPONENTS} )
		find_path( Ogre_${_COMPONENT}_LIBRARY_DIR_DEBUG Ogre${_COMPONENT}_d.lib
			HINTS
			PATH_SUFFIXES lib/Debug lib
			PATHS
				${Ogre_ROOT_DIR}
				${Ogre_LIBRARY_DIR}
				${Ogre_PLUGINS_DIR}
		)
		find_library( Ogre_${_COMPONENT}_LIBRARY_DEBUG
			NAMES
				Ogre${_COMPONENT}_d.lib
			HINTS
			PATHS
				${Ogre_${_COMPONENT}_LIBRARY_DIR_DEBUG}
		)
		find_path( Ogre_${_COMPONENT}_LIBRARY_DIR_RELEASE Ogre${_COMPONENT}.lib
			HINTS
			PATH_SUFFIXES lib/Release lib
			PATHS
				${Ogre_ROOT_DIR}
				${Ogre_LIBRARY_DIR}
				${Ogre_PLUGINS_DIR}
		)
		find_library( Ogre_${_COMPONENT}_LIBRARY_RELEASE
			NAMES
				Ogre${_COMPONENT}.lib
			HINTS
			PATHS
				${Ogre_${_COMPONENT}_LIBRARY_DIR_RELEASE}
		)

		if ( Ogre_${_COMPONENT}_LIBRARY_RELEASE AND Ogre_${_COMPONENT}_LIBRARY_DEBUG )
			set( Ogre_${_COMPONENT}_LIBRARIES optimized ${Ogre_${_COMPONENT}_LIBRARY_RELEASE} debug ${Ogre_${_COMPONENT}_LIBRARY_DEBUG} )
		elseif ( Ogre_${_COMPONENT}_LIBRARY_RELEASE )
			set( Ogre_${_COMPONENT}_LIBRARIES ${Ogre_${_COMPONENT}_LIBRARY_RELEASE} )
		elseif ( Ogre_${_COMPONENT}_LIBRARY_DEBUG )
			set( Ogre_${_COMPONENT}_LIBRARIES ${Ogre_${_COMPONENT}_LIBRARY_DEBUG} )
		endif ()

		if ( Ogre_${_COMPONENT}_LIBRARIES )
			set( Ogre_LIBRARIES
				${Ogre_LIBRARIES}
				${Ogre_${_COMPONENT}_LIBRARIES}
			)
		endif ()

		mark_as_advanced(
			${Ogre_${_COMPONENT}_LIBRARY_DEBUG}
			${Ogre_${_COMPONENT}_LIBRARY_RELEASE}
			${Ogre_${_COMPONENT}_LIBRARY_DIR_DEBUG}
			${Ogre_${_COMPONENT}_LIBRARY_DIR_RELEASE}
			${Ogre_${_COMPONENT}_LIBRARIES}
		)
	endforeach ()
else()
	find_path( Ogre_LIBRARY_DIR libOgreMain.so OgreMain.lib
		HINTS
		PATH_SUFFIXES lib64 lib x86_64-linux-gnu lib/x86_64-linux-gnu
		PATHS
		${Ogre_ROOT_DIR}
	)
	find_path( Ogre_PLUGINS_DIR libRenderSystem_GL.so RenderSystem_GL.lib RenderSystem_GL.so
		HINTS
		PATH_SUFFIXES lib64 lib x86_64-linux-gnu lib/x86_64-linux-gnu
		PATHS
		${Ogre_LIBRARY_DIR}
		${Ogre_LIBRARY_DIR}/OGRE
		${Ogre_LIBRARY_DIR}/OGRE-${Ogre_VERSION}
	)

	foreach( _COMPONENT ${Ogre_FIND_COMPONENTS} )
		find_library(Ogre_${_COMPONENT}_LIBRARY
			NAMES
				lib${_COMPONENT}.so
				${_COMPONENT}.so
				${_COMPONENT}.lib
			HINTS
			PATHS
				${Ogre_LIBRARY_DIR}
				${Ogre_PLUGINS_DIR}
		)
		if ( Ogre_${_COMPONENT}_LIBRARY )
			set( Ogre_LIBRARIES
				${Ogre_LIBRARIES}
				${Ogre_${_COMPONENT}_LIBRARY}
			)
		endif ()
	endforeach ()
	mark_as_advanced( ${Ogre_LIBRARY_DIR} )
endif()

find_package_handle_standard_args( OGRE DEFAULT_MSG Ogre_LIBRARIES Ogre_INCLUDE_DIR )
