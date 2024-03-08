FIND_PATH(SQLite_ROOT_DIR include/SQLite.h
  HINTS
  PATH_SUFFIXES SQLite
  PATHS
  /usr/local/include
  /usr/include
)

FIND_PATH(SQLite_INCLUDE_DIR SQLite.h 
  HINTS
  PATH_SUFFIXES Dist dist include
  PATHS
  ${SQLite_ROOT_DIR}
  /usr/local
  /usr
)

FIND_PATH(SQLite_LIBRARY_DIR libSQLite.so libSQLite.so SQLite.lib
  HINTS
  PATH_SUFFIXES Dist dist lib64 lib
  PATHS
  ${SQLite_ROOT_DIR}
  /usr/local
  /usr
)

FIND_LIBRARY(SQLite_LIBRARY
  NAMES libSQLite.so libSQLite.so SQLite.lib
  HINTS
  PATH_SUFFIXES lib64 lib
  PATHS
  ${SQLite_LIBRARY_DIR}
  /usr/local/X11R6
  /usr/local/X11
  /usr/X11
  /sw
  /usr/freeware
)

MARK_AS_ADVANCED( SQLite_LIBRARY_DIR )
MARK_AS_ADVANCED( SQLite_LIBRARY )

SET( SQLite_LIBRARY_DIRS ${SQLite_LIBRARY_DIR} )
SET( SQLite_LIBRARIES ${SQLite_LIBRARY} )

find_package_handle_standard_args( SQLite DEFAULT_MSG SQLite_LIBRARIES SQLite_INCLUDE_DIR )
