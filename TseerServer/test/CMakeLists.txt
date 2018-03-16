aux_source_directory(. DIR_SRCS)

add_executable(Test ${DIR_SRCS})

target_link_libraries(Test tarsservant tarsparse tarsutil
                      pthread)