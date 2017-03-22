set (CMAKE_CXX_STANDARD 11)

add_definitions(-DPYTHON2)

set_source_files_properties (${CMAKE_CURRENT_SOURCE_DIR}/../littleb.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties (${CMAKE_CURRENT_SOURCE_DIR}/../littleb.i PROPERTIES CMAKE_CXX_STANDARD "-includeall")

swig_add_module (littleb_py2 python ${CMAKE_CURRENT_SOURCE_DIR}/../littleb.i)
swig_link_libraries (littleb_py2 ${PYTHON2_LIBRARIES} littleb)
target_include_directories(${SWIG_MODULE_littleb_py2_REAL_NAME}
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${PYTHON2_INCLUDE_DIR}"
)
set_target_properties (${SWIG_MODULE_littleb_py2} PROPERTIES OUTPUT_NAME littleb)
