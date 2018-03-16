cmake_minimum_required(VERSION 2.8)

project(TseerServer)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/protocol
                    ${TARS_DIR_INC}
                    ${RAPID_JSON_DIR}
                    ${TARS_DIR_INC}/servant)

link_directories(${TARS_DIR_LIB})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/bin)

add_subdirectory(protocol)
add_subdirectory(src)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.conf DESTINATION ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/conf)