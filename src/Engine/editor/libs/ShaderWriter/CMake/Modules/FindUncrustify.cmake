###########################################################
#
# Find Uncrustify on client machine
#
## 1: Setup:
# The following variables are searched for defaults
#  UNCRUSTIFY_ROOT_DIR: Base directory of Uncrustify binary to use.
#
## 2: Variable
# The following are set after configuration is done: 
#  
#  UNCRUSTIFY_FOUND
#  UNCRUSTIFY_BINARY
#
###########################################################

FIND_PACKAGE( PackageHandleStandardArgs )

FIND_PATH(UNCRUSTIFY_ROOT_DIR
	NAMES
		uncrustify.exe
		bin/uncrustify.exe
		bin/uncrustify
	HINTS
	PATH_SUFFIXES Uncrustify
	PATHS
		/usr
		/usr/local
		C:/
		Z:/
)

SET( UNCRUSTIFY_BINARY_DIR ${UNCRUSTIFY_ROOT_DIR}/bin )

FIND_PROGRAM( UNCRUSTIFY_BINARY
	NAMES 
		uncrustify.exe
		uncrustify
	PATHS
		${UNCRUSTIFY_BINARY_DIR}
		${UNCRUSTIFY_ROOT_DIR}
)

MARK_AS_ADVANCED( UNCRUSTIFY_BINARY_DIR UNCRUSTIFY_BINARY )

find_package_handle_standard_args( UNCRUSTIFY DEFAULT_MSG UNCRUSTIFY_BINARY )
