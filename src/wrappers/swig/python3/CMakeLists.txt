set (CMAKE_CXX_STANDARD 11)

add_definitions(-DPYTHON3)

set_source_files_properties (${CMAKE_CURRENT_SOURCE_DIR}/../littleb.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties (${CMAKE_CURRENT_SOURCE_DIR}/../littleb.i PROPERTIES CMAKE_CXX_STANDARD "-includeall")

swig_add_module (littleb_py3 python ${CMAKE_CURRENT_SOURCE_DIR}/../littleb.i)
swig_link_libraries (littleb_py3 ${PYTHON3_LIBRARIES} littleb)
target_include_directories(${SWIG_MODULE_littleb_py3_REAL_NAME}
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${PYTHON3_INCLUDE_DIR}"
)
set_target_properties (${SWIG_MODULE_littleb_py3} PROPERTIES OUTPUT_NAME littleb)
