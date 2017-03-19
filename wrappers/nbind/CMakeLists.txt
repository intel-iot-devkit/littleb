cmake_minimum_required (VERSION 2.8)

execute_process(COMMAND node-gyp configure build WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND node-gyp build WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#add_custom_command(OUTPUT: PRE_LINK COMMAND node-gyp configure build WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#add_custom_command(OUTPUT: PRE_LINK COMMAND node-gyp build WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})  
