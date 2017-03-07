cmake_minimum_required (VERSION 2.8)
project (littleb)

find_package (Threads REQUIRED)

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -fno-omit-frame-pointer -std=c++11")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -std=c++11")

# Set CMAKE_INSTALL_LIBDIR if not defined
include(GNUInstallDirs)

# Older cmake might not pick CMAKE_INSTALL_LIBDIR right
if (CMAKE_INSTALL_LIBDIR)
  set (LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}" CACHE PATH "Installation path for libraries")
else ()
  set (LIB_INSTALL_DIR "lib" CACHE PATH "Installation path for libraries")
endif ()

# By default, build shared object libraries on linux
if (UNIX AND NOT APPLE)
  if (NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ON)
  endif()
endif()

# Appends the cmake/modules path to MAKE_MODULE_PATH variable.
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

# Make a version file containing the current version from git.
include (GetGitRevisionDescription)
git_describe (VERSION "--tags")
if ("x_${VERSION}" STREQUAL "x_GIT-NOTFOUND" OR "x_${VERSION}" STREQUAL "x_HEAD-HASH-NOTFOUND" OR "x_${VERSION}" STREQUAL "x_-128-NOTFOUND")
  message (WARNING " - Install git to compile a production liblittleb!")
  set (VERSION "v0.1.0")
endif ()

message (STATUS "INFO - liblittleb Version ${VERSION}")
message (STATUS "INFO - cmake Version ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}")

#parse the version information into pieces.
string (REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string (REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string (REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
string (REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+\\-([0-9]+).*" "\\1" VERSION_COMMIT "${VERSION}")
string (REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-[0-9]+\\-(.*)" "\\1" VERSION_SHA1 "${VERSION}")
set (VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

if ("${VERSION_COMMIT}" MATCHES "^v.*")
  set (VERSION_COMMIT "")
endif()

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/version.c.in
                ${CMAKE_CURRENT_BINARY_DIR}/src/version.c)

# this is the library version, independant of git revision
set (littleb_VERSION_MAJOR ${VERSION_MAJOR})
set (littleb_VERSION_MINOR ${VERSION_MINOR})
set (littleb_VERSION_PATCH ${VERSION_PATCH})
set (littleb_VERSION_STRING ${littleb_VERSION_MAJOR}.${littleb_VERSION_MINOR}.${littleb_VERSION_PATCH})

set (CMAKE_SWIG_FLAGS "")

find_path (SYSTEM_USR_DIR "stdlib.h")
include_directories (${SYSTEM_USR_DIR})

option (BUILDSWIG "Build swig modules." ON)
#currently disabling to avoid breaking the builc
option (BUILDSWIGPYTHON "Build swig python modules." OFF)
option (ENABLEEXAMPLES "Disable building of examples" ON)

if (BUILDSWIGPYTHON)
  include (cmake/modules/OpenCVDetectPython.cmake)
endif ()



add_subdirectory (src)

if (ENABLEEXAMPLES)
  add_subdirectory (examples)
endif ()
