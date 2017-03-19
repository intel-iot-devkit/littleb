cmake_minimum_required (VERSION 2.8)

# if (PYTHON2_LIBRARY)
	SET(PYTHON_INCLUDE_DIRS "/usr/include/python2.7")
	SET(PYTHON_LIBRARIES "/usr/lib/x86_64-linux-gnu/libpython2.7.so")
	set(CMAKE_CXX_STANDARD 11)
	FIND_PACKAGE(Systemd REQUIRED)
	FIND_PACKAGE(SWIG REQUIRED)
	INCLUDE(${SWIG_USE_FILE})
	# FIND_PACKAGE(PythonLibs REQUIRED)
	add_definitions(${SYSTEMD_DEFINITIONS})
	include_directories(${PYTHON_INCLUDE_DIRS})

	set (littleb_LIB_INCLUDE_DIRS
	 ${PROJECT_SOURCE_DIR}/api
	  ${PROJECT_SOURCE_DIR}/include
	  ${CMAKE_CURRENT_SOURCE_DIR}/..
	)

	include_directories(
	  ${littleb_LIB_INCLUDE_DIRS}
	  ${SYSTEMD_INCLUDE_DIRS}
	)

	set (littleb_LIB_SRCS
	  ${PROJECT_SOURCE_DIR}/src/littleb.c
	  ${PROJECT_SOURCE_DIR}/src/device.cpp
	  ${PROJECT_SOURCE_DIR}/src/devicemanager.cpp
	  ${PROJECT_SOURCE_DIR}/src/littlebtypes.cpp
	  ${CMAKE_CURRENT_SOURCE_DIR}/../littleb.i
	)

	add_definitions(-DBUILDING_WITH_SWIG)
	add_definitions(-DPYTHON2)
	SET(CMAKE_SWIG_FLAGS "")

	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/../littleb.i PROPERTIES CPLUSPLUS ON)
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/../littleb.i PROPERTIES CMAKE_CXX_STANDARD "-includeall")

	SWIG_ADD_MODULE(littleb_py2 python  ${littleb_LIB_SRCS} )
	SWIG_LINK_LIBRARIES(littleb_py2 ${PYTHON_LIBRARIES} ${SYSTEMD_LIBRARIES} )
	set_target_properties(_littleb_py2 PROPERTIES OUTPUT_NAME _littleb) 

# endif()

