add_executable (hellolittleb hello_littleb.c)
add_executable (esplittleb esp_littleb.c)

include_directories(${PROJECT_SOURCE_DIR}/api)

target_link_libraries (hellolittleb littleb ${CMAKE_THREAD_LIBS_INIT} ${SYSTEMD_LIBRARIES})
target_link_libraries (esplittleb littleb ${CMAKE_THREAD_LIBS_INIT} ${SYSTEMD_LIBRARIES})

install (DIRECTORY ${PROJECT_SOURCE_DIR}/examples/ DESTINATION ${CMAKE_INSTALL_DATADIR}/littleb/examples)
