FIND_PATH(ZZip_ROOT_DIR include/zzip/zzip.h
	HINTS
	PATH_SUFFIXES include zziplib
	PATHS
	/usr/local
	/usr
)

FIND_PATH(ZZip_INCLUDE_DIR zzip/zzip.h
	HINTS
	PATH_SUFFIXES include ZZip
	PATHS
	${ZZip_ROOT_DIR}
)

if(MSVC)
	FIND_PATH(ZZip_LIBRARY_DIR_DEBUG zziplib_d.lib
		HINTS
		PATH_SUFFIXES lib
		PATHS
		${ZZip_ROOT_DIR}
	)
	FIND_PATH(ZZip_LIBRARY_DIR_RELEASE zziplib.lib
		HINTS
		PATH_SUFFIXES lib
		PATHS
		${ZZip_ROOT_DIR}
	)
	FIND_LIBRARY(ZZip_LIBRARY_DEBUG
		NAMES zziplib_d.lib
		HINTS
		PATHS
		${ZZip_LIBRARY_DIR_DEBUG}
	)
	FIND_LIBRARY(ZZip_LIBRARY_RELEASE
		NAMES zziplib.lib
		HINTS
		PATHS
		${ZZip_LIBRARY_DIR_RELEASE}
	)
	SET( ZZip_LIBRARIES debug ${ZZip_LIBRARY_DEBUG} optimized ${ZZip_LIBRARY_RELEASE})
	MARK_AS_ADVANCED(${ZZip_LIBRARY_DEBUG} ${ZZip_LIBRARY_RELEASE} ${ZZip_LIBRARY_DIR_DEBUG} ${ZZip_LIBRARY_DIR_RELEASE})
else()
	FIND_PATH(ZZip_LIBRARY_DIR libZZip.so libzzip.so
		HINTS
		PATH_SUFFIXES lib64 lib
		PATHS
		${ZZip_ROOT_DIR}
	)
	FIND_LIBRARY(ZZip_LIBRARY
		NAMES libZZip.so libzzip.so
		HINTS
		PATHS
		${ZZip_LIBRARY_DIR}
	)
	SET( ZZip_LIBRARIES ${ZZip_LIBRARY})
	MARK_AS_ADVANCED(${ZZip_LIBRARY_DIR})
endif()

find_package_handle_standard_args( ZZip DEFAULT_MSG ZZip_LIBRARIES ZZip_INCLUDE_DIR )
