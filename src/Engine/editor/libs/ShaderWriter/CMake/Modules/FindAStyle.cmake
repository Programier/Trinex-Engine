###########################################################
#
# Find AStyle on client machine
#
## 1: Setup:
# The following variables are searched for defaults
#  ASTYLE_ROOT_DIR:            Base directory of AStyle tree to use.
#
## 2: Variable
# The following are set after configuration is done: 
#  
#  AStyle_FOUND
#  AStyle_BINARY
#
###########################################################

FIND_PACKAGE( PackageHandleStandardArgs )

FIND_PATH(AStyle_ROOT_DIR
  NAMES
  	bin/AStyle.exe
  	bin/astyle
  HINTS
	PATH_SUFFIXES AStyle
  PATHS
  	/usr
  	/usr/local
	C:/
	Z:/
)

SET( AStyle_BINARY_DIR ${AStyle_ROOT_DIR}/bin )

FIND_PROGRAM( AStyle_BINARY
  NAMES 
  	AStyle.exe
	astyle
  PATHS
  	${AStyle_BINARY_DIR}
)

MARK_AS_ADVANCED( AStyle_LIBRARY_DIR AStyle_BINARY )

if ( AStyle_BINARY )
	set( AStyle_FOUND ON )
else ()
	set( AStyle_FOUND OFF )
endif ()
