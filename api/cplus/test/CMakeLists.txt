include_directories(${TSEERAPI_ROOT_DIR}/src)

aux_source_directory(. DIR_SRCS)
add_executable(testapi ${DIR_SRCS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/test)

target_link_libraries(testapi tseerapi pthread)