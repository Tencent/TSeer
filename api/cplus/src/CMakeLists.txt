#project(tseerapi)

#set(LIBRARY_OUTPUT_PATH ${TSEERAPI_ROOT_DIR}/lib)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/lib)

aux_source_directory(. DIR_SRCS)

add_library(tseerapi STATIC ${DIR_SRCS})

target_link_libraries(tseerapi rt)
