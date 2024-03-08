# - Try to find GDKmm 3.0
# Once done, this will define
#
#  GDKmm_FOUND - system has GDKmm
#  GDKmm_INCLUDE_DIRS - the GDKmm include directories
#  GDKmm_LIBRARIES - link these to use GDKmm

include(LibFindMacros)

# Dependencies
libfind_package(GDKmm GDK)
libfind_package(GDKmm Glibmm)
libfind_package(GDKmm Pangomm)
libfind_package(GDKmm Cairomm)

set(GDKmm_Version 3.0)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(GDKmm_PKGCONF gdkmm-${GDKmm_Version})

# Main include dir
find_path(GDKmm_INCLUDE_DIR
  NAMES gdkmm.h
  PATHS ${GDKmm_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES gdkmm-${GDKmm_Version}
)

# Glib-related libraries also use a separate config header, which is in lib dir
find_path(GDKmmConfig_INCLUDE_DIR
  NAMES gdkmmconfig.h
  PATHS ${GDKmm_PKGCONF_INCLUDE_DIRS} /usr
  PATH_SUFFIXES lib/gdkmm-${GDKmm_Version}/include
)

libfind_library(GDKmm gdkmm ${GDKmm_Version})

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(GDKmm_PROCESS_INCLUDES GDKmm_INCLUDE_DIR GDKmmConfig_INCLUDE_DIR GDK_INCLUDE_DIRS Glibmm_INCLUDE_DIRS Pangomm_INCLUDE_DIRS Cairomm_INCLUDE_DIRS)
set(GDKmm_PROCESS_LIBS GDKmm_LIBRARY GDK_LIBRARIES Glibmm_LIBRARIES Pangomm_LIBRARIES Cairomm_LIBRARIES)
libfind_process(GDKmm)

