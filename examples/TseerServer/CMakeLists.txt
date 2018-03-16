cmake_minimum_required(VERSION 2.8)
project(TestSeer)

add_subdirectory(protocol)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/protocol
                    ${TARS_DIR_INC}
                    ${RAPID_JSON_DIR}
                    ${TARS_DIR_INC}/servant)
					
link_directories(${TARS_DIR_LIB})

aux_source_directory(. DIR_SRCS)

add_executable(${PROJECT_NAME} ${DIR_SRCS})

add_dependencies(${PROJECT_NAME} TESTSEERSERVER_TARS_HEADER)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/bin)


target_link_libraries(${PROJECT_NAME} tarsservant tarsparse tarsutil
                      pthread)