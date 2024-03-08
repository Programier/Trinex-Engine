# **********************************************************************
#
# File        : FindODBC.cmake
#
# Description : Find the ODBC includes and libraries.
#
# **********************************************************************

if( CMAKE_CL_64 OR ( CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SIZEOF_VOID_P EQUAL 8 ) )
    set( ODBC_ARCH "x64" )
else()
    set( ODBC_ARCH "x86" )
endif()

# -----------------------------------------------------------------------
# Find ODBC root directory
# -----------------------------------------------------------------------
find_path( ODBC_ROOT_DIR 
	NAMES
    	Include/um/sql.h
    	include/sql.h
	PATHS
    	"$ENV{PROGRAMFILES}/Windows Kits/8.0"
		"$ENV{ProgramW6432}/Windows Kits/8.0"
	DOC
		"The ODBC root directory"
)

if( NOT ODBC_ROOT_DIR )
	message( STATUS "ODBC root directory not found!" )
endif( NOT ODBC_ROOT_DIR )

# -----------------------------------------------------------------------
# Find ODBC include directory
# -----------------------------------------------------------------------
find_path( ODBC_INCLUDE_DIR sql.h
	PATHS
		${ODBC_ROOT_DIR}/Include/um
	DOC
		"The ODBC include directory"
)

if( NOT ODBC_INCLUDE_DIR )
    message( STATUS "ODBC include directory not found!" )
endif( NOT ODBC_INCLUDE_DIR )

# -----------------------------------------------------------------------
# Find ODBC libraries
# -----------------------------------------------------------------------
find_library( ODBC_LIBRARIES
	NAMES
		odbc32
		odbc
	PATHS
		${ODBC_ROOT_DIR}/Lib/win8/um/${ODBC_ARCH}
	DOC
		"The ODBC libraries."
)

if( NOT ODBC_LIBRARIES )
    message( STATUS "ODBC librairies not found!" )
endif( NOT ODBC_LIBRARIES )
    
# -----------------------------------------------------------------------
# handle the QUIETLY and REQUIRED arguments and set ODBC_FOUND to true
# if all listed variables are TRUE
# ---------------------------------------------------------------------
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( ODBC 
    DEFAULT_MSG 
    ODBC_INCLUDE_DIR
    ODBC_LIBRARIES
)

mark_as_advanced(
    ODBC_INCLUDE_DIR
    ODBC_LIBRARIES
)
