# - Try to find LibXml
# Once done this will define
#  LIBXML_FOUND - System has LibXml
#  LIBXML_INCLUDE_DIRS - The LibXml include directories
#  LIBXML_LIBRARIES - The libraries needed to use LibXml
#  LIBXML_DEFINITIONS - Compiler switches required for using LibXml

find_package(PkgConfig QUIET)
pkg_check_modules(PC_LIBXML QUIET libxml)
set(LIBXML_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})

find_path(LIBXML_INCLUDE_DIR libxml/xpath.h
          HINTS ${PC_LIBXML_INCLUDEDIR} ${PC_LIBXML_INCLUDE_DIRS}
          PATH_SUFFIXES libxml2 )

find_library(LIBXML_LIBRARY NAMES xml2 libxml2
             HINTS ${PC_LIBXML_LIBDIR} ${PC_LIBXML_LIBRARY_DIRS} )

set(LIBXML_LIBRARIES ${LIBXML_LIBRARY} )
set(LIBXML_INCLUDE_DIRS ${LIBXML_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBXML_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibXml  DEFAULT_MSG
                                  LIBXML_LIBRARY LIBXML_INCLUDE_DIR)

mark_as_advanced(LIBXML_INCLUDE_DIR LIBXML_LIBRARY )
