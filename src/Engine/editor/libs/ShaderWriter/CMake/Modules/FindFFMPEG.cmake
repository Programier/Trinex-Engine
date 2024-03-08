# FindFFMPEG
# ------------
#
# Locate ffmpeg libraries
#
# This module defines
#
# ::
#
# FFMPEG_FOUND - system has ffmpeg or libav
# FFMPEG_INCLUDE_DIR - the ffmpeg include directory
# FFMPEG_LIBRARIES - Link these to use ffmpeg
# FFMPEG_LIBAVCODEC
# FFMPEG_LIBAVFORMAT
# FFMPEG_LIBAVUTIL
# FFMPEG_LIBSWSCALE
# FFMPEG_LIBSWRESAMPLE
#

if (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
	# in cache already
	set(FFMPEG_FOUND TRUE)
else ()
	if ( NOT FFMPEG_FIND_COMPONENTS )
		set( FFMPEG_FIND_COMPONENTS avcodec avutil avformat swscale swresample )
	endif ()

	find_package( PkgConfig QUIET )

	if ( PKG_CONFIG_FOUND )
		pkg_check_modules( _FFMPEG_AVCODEC libavcodec )
	endif ( PKG_CONFIG_FOUND )

	find_path( FFMPEG_INCLUDE_DIR
		NAMES libavcodec/avcodec.h
		PATHS
			${_FFMPEG_AVCODEC_INCLUDE_DIRS}
			/usr/include
			/usr/local/include
			/opt/local/include
			/sw/include
			${FFMPEG_DIR}/include
		PATH_SUFFIXES ffmpeg libav
	)

	foreach ( FIND_COMPONENT ${FFMPEG_FIND_COMPONENTS} )
		string( TOUPPER ${FIND_COMPONENT} FIND_COMPONENT_UPPER )
		# use pkg-config to get the directories and then use these values
		# in the FIND_PATH() and FIND_LIBRARY() calls
		if ( PKG_CONFIG_FOUND )
			pkg_check_modules(_FFMPEG_${FIND_COMPONENT_UPPER} lib${FIND_COMPONENT})
		endif ()

		find_library( FFMPEG_LIB${FIND_COMPONENT_UPPER}
			NAMES
				${FIND_COMPONENT}
			PATHS
				${_FFMPEG_${FIND_COMPONENT_UPPER}_LIBRARY_DIRS}
				/usr/lib
				/usr/local/lib
				/opt/local/lib
				/sw/lib
				${FFMPEG_DIR}/lib
		)

		if ( FFMPEG_INCLUDE_DIR )
			if ( EXISTS ${FFMPEG_INCLUDE_DIR}/lib${FIND_COMPONENT}/version.h )
				file( STRINGS "${FFMPEG_INCLUDE_DIR}/lib${FIND_COMPONENT}/version.h" FFMPEG_VERSION_STR REGEX "^#[\t ]*define[\t ]+LIB${FIND_COMPONENT_UPPER}_VERSION_(MAJOR|MINOR|MICRO)[\t ]+[0-9]+$" )
				unset( FFMPEG_${FIND_COMPONENT_UPPER}_VERSION_STRING )
				foreach ( VPART MAJOR MINOR MICRO )
					foreach ( VLINE ${FFMPEG_VERSION_STR} )
						if ( VLINE MATCHES "^#[\t ]*define[\t ]+LIB${FIND_COMPONENT_UPPER}_VERSION_${VPART}" )
							string( REGEX REPLACE "^#[\t ]*define[\t ]+LIB${FIND_COMPONENT_UPPER}_VERSION_${VPART}[\t ]+([0-9]+)$" "\\1" FFMPEG_${FIND_COMPONENT_UPPER}_VERSION_${VPART} "${VLINE}" )
							if ( FFMPEG_${FIND_COMPONENT_UPPER}_VERSION_STRING )
								set( FFMPEG_${FIND_COMPONENT_UPPER}_VERSION_STRING "${FFMPEG_${FIND_COMPONENT_UPPER}_VERSION_STRING}.${FFMPEG_${FIND_COMPONENT_UPPER}_VERSION_${VPART}}" )
							else ()
								set( FFMPEG_${FIND_COMPONENT_UPPER}_VERSION_STRING "${FFMPEG_${FIND_COMPONENT_UPPER}_VERSION_${VPART}}" )
							endif ()
						endif ()
					endforeach ()
				endforeach ()
			endif ()

			if ( FFMPEG_LIB${FIND_COMPONENT_UPPER} )
				set( FFMPEG_LIBRARIES
					${FFMPEG_LIBRARIES}
					${FFMPEG_LIB${FIND_COMPONENT_UPPER}}
				)
			endif ()
		endif ()
	endforeach ()

	if ( FFMPEG_LIBAVCODEC AND FFMPEG_LIBAVFORMAT )
		set( FFMPEG_FOUND TRUE )
	endif ()

	if ( NOT FFMPEG_FOUND )
		unset( FFMPEG_LIBRARIES )
	endif ()

	if (FFMPEG_FOUND)
		if (NOT FFMPEG_FIND_QUIETLY)
			message(STATUS "Found FFMPEG or Libav:")
		endif (NOT FFMPEG_FIND_QUIETLY)
	else (FFMPEG_FOUND)
		if (FFMPEG_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find libavcodec or libavformat or libavutil or libswscale")
		endif (FFMPEG_FIND_REQUIRED)
	endif (FFMPEG_FOUND)
endif (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIR)
