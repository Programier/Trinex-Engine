# FindFbxSDK
# ------------
#
# Locate FBX SDK library
#
# This module defines
#
# ::
#
#   FbxSDK_LIBRARIES, the libraries to link against
#   FbxSDK_FOUND, if false, do not try to link to FBX SDK
#   FbxSDK_INCLUDE_DIR, where to find headers.
# 

if ( NOT FbxSDK_ROOT_DIR )
	set( LOOKUP_PATHS
		C:/Program\ Files/Autodesk/FBX/FBX\ SDK/2016.1.2
	)
	FIND_PATH(FbxSDK_ROOT_DIR include/fbxsdk.h 
		HINTS
		PATHS
			${LOOKUP_PATHS}
			/usr/local
			/usr
	)
endif ()

FIND_PATH(FbxSDK_INCLUDE_DIR fbxsdk.h 
	HINTS
	PATH_SUFFIXES
		include
	PATHS
		${FbxSDK_ROOT_DIR}
		/usr/local/include
		/usr/include
)

if (CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES Win64)
	if( MSVC )
		FIND_PATH(FbxSDK_LIBRARY_RELEASE_DIR libfbxsdk-md.lib
			HINTS
			PATH_SUFFIXES
				lib/vs2015/x64/release
				lib/x64/release
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_PATH(FbxSDK_LIBRARY_DEBUG_DIR libfbxsdk-md.lib
			HINTS
			PATH_SUFFIXES
				lib/vs2015/x64/debug
				lib/x64/debug
				lib
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_RELEASE
			NAMES
				libfbxsdk-md.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_RELEASE_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_DEBUG
			NAMES
				libfbxsdk-md.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_DEBUG_DIR}
		)
	elseif ( CMAKE_COMPILER_IS_GNUCXX )
		FIND_PATH(FbxSDK_LIBRARY_RELEASE_DIR libfbxsdk.so libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib/gcc4/x64/debug
				lib/x64/debug
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_PATH(FbxSDK_LIBRARY_DEBUG_DIR libfbxsdk.so libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib/gcc4/x64/release
				lib/x64/release
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_RELEASE
			NAMES
				libfbxsdk.so
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_RELEASE_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_DEBUG
			NAMES
				libfbxsdk.so
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_DEBUG_DIR}
		)
	else()
		FIND_PATH(FbxSDK_LIBRARY_RELEASE_DIR libfbxsdk.so libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib64
				lib/x64/Release
				lib/Release/x64
				lib/x64
				lib
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_PATH(FbxSDK_LIBRARY_DEBUG_DIR libfbxsdk.so libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib64
				lib/x64/Debug
				lib/Debug/x64
				lib/x64
				lib
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_RELEASE
			NAMES
				libfbxsdk.so
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_RELEASE_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_DEBUG
			NAMES
				libfbxsdk.so
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_DEBUG_DIR}
		)
	endif()
else()
	if( MSVC )
		FIND_PATH(FbxSDK_LIBRARY_RELEASE_DIR libfbxsdk-md.lib
		HINTS
		PATH_SUFFIXES
				lib/vs2015/x86/release
				lib/x86/release
		PATHS
			${FbxSDK_ROOT_DIR}
		)

		FIND_PATH(FbxSDK_LIBRARY_DEBUG_DIR libfbxsdk-md.lib
		HINTS
		PATH_SUFFIXES
				lib/vs2015/x86/debug
				lib/x86/debug
		PATHS
			${FbxSDK_ROOT_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_RELEASE
			NAMES
				libfbxsdk-md.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_RELEASE_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_DEBUG
			NAMES
				libfbxsdk-md.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_DEBUG_DIR}
		)
	elseif( MINGW )
		FIND_PATH(FbxSDK_LIBRARY_RELEASE_DIR libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib/x86
				lib/x86/Release
				lib/Release/x86
				lib
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_PATH(FbxSDK_LIBRARY_DEBUG_DIR libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib/x86
				lib/x86/Debug
				lib/Debug/x86
				lib
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_RELEASE
			NAMES
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_RELEASE_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_DEBUG
			NAMES
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_DEBUG_DIR}
		)
	elseif ( CMAKE_COMPILER_IS_GNUCXX AND (${CMAKE_SIZEOF_VOID_P} EQUAL 8) )
		FIND_PATH(FbxSDK_LIBRARY_RELEASE_DIR libfbxsdk.so libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib/gcc4/x64/release
				lib/x64/release
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_PATH(FbxSDK_LIBRARY_DEBUG_DIR libfbxsdk.so libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib/gcc4/x64/debug
				lib/x64/debug
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_RELEASE
			NAMES
				libfbxsdk.so
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_RELEASE_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_DEBUG
			NAMES
				libfbxsdk.so
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_DEBUG_DIR}
		)
	elseif ( CMAKE_COMPILER_IS_GNUCXX )
		FIND_PATH(FbxSDK_LIBRARY_RELEASE_DIR libfbxsdk.so libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib/gcc4/x86/release
				lib/x86/release
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_PATH(FbxSDK_LIBRARY_DEBUG_DIR libfbxsdk.so libfbxsdk.lib
			HINTS
			PATH_SUFFIXES
				lib/gcc4/x86/debug
				lib/x86/debug
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_RELEASE
			NAMES
				libfbxsdk.so
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_RELEASE_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_DEBUG
			NAMES
				libfbxsdk.so
				libfbxsdk.lib
			HINTS
			PATHS
				${FbxSDK_LIBRARY_DEBUG_DIR}
		)
	else()
		FIND_PATH(FbxSDK_LIBRARY_RELEASE_DIR libfbxsdk.so
			HINTS
			PATH_SUFFIXES
				lib
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_PATH(FbxSDK_LIBRARY_DEBUG_DIR libfbxsdk.so
			HINTS
			PATH_SUFFIXES
				lib
			PATHS
				${FbxSDK_ROOT_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_RELEASE
			NAMES
				libfbxsdk.so
			HINTS
			PATHS
				${FbxSDK_LIBRARY_RELEASE_DIR}
		)

		FIND_LIBRARY(FbxSDK_LIBRARY_DEBUG
			NAMES
				libfbxsdk.so
			HINTS
			PATHS
				${FbxSDK_LIBRARY_DEBUG_DIR}
		)
	endif()
endif()

MARK_AS_ADVANCED( FbxSDK_LIBRARY_RELEASE_DIR )
MARK_AS_ADVANCED( FbxSDK_LIBRARY_DEBUG_DIR )
MARK_AS_ADVANCED( FbxSDK_LIBRARY_RELEASE )
MARK_AS_ADVANCED( FbxSDK_LIBRARY_DEBUG_DIR )
find_package_handle_standard_args( FbxSDK DEFAULT_MSG FbxSDK_LIBRARY_RELEASE FbxSDK_INCLUDE_DIR )

IF ( FbxSDK_FOUND )
	IF (MSVC)
		if ( FbxSDK_LIBRARY_DEBUG )
			SET(FbxSDK_LIBRARIES optimized ${FbxSDK_LIBRARY_RELEASE} debug ${FbxSDK_LIBRARY_DEBUG} CACHE STRING "FBX SDK libraries")
			SET(FbxSDK_LIBRARY_DIRS ${FbxSDK_LIBRARY_RELEASE_DIR} ${FbxSDK_LIBRARY_DEBUG_DIR})
		else()
			SET(FbxSDK_LIBRARIES ${FbxSDK_LIBRARY_RELEASE} CACHE STRING "FBX SDK libraries")
			SET(FbxSDK_LIBRARY_DIRS ${FbxSDK_LIBRARY_RELEASE_DIR})
		endif()
	ELSE ()
		if ( FbxSDK_LIBRARY_DEBUG )
			SET(FbxSDK_LIBRARIES optimized ${FbxSDK_LIBRARY_RELEASE} debug ${FbxSDK_LIBRARY_DEBUG} CACHE STRING "FBX SDK libraries")
			SET(FbxSDK_LIBRARY_DIRS ${FbxSDK_LIBRARY_RELEASE_DIR} ${FbxSDK_LIBRARY_DEBUG_DIR})
		else()
			SET(FbxSDK_LIBRARIES ${FbxSDK_LIBRARY_RELEASE} CACHE STRING "FBX SDK libraries")
			SET(FbxSDK_LIBRARY_DIRS ${FbxSDK_LIBRARY_RELEASE_DIR})
		endif()
	ENDIF ()
ENDIF ()