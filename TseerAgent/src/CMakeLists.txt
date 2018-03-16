set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/bin)

aux_source_directory(. DIR_SRCS)
add_executable(${SERVER_NAME} ${DIR_SRCS})

target_link_libraries(${SERVER_NAME} tarsservant tarsparse tarsutil rt
                      pthread)