find_package (SWIG REQUIRED)
include (${SWIG_USE_FILE})

if (PYTHON2_LIBRARIES)
  add_subdirectory (python2)
endif ()
if (PYTHON3_LIBRARIES)
  add_subdirectory (python3)
endif ()
