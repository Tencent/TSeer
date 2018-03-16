cmake_minimum_required(VERSION 2.8)
project(TseerServer)

if(USE_MYSQL)
    include_directories(${MYSQL_DIR_INC})
    add_definitions(-DUSE_MYSQL)
endif(USE_MYSQL)

add_definitions(-DTARS_VERSION="${TARS_VERSION}" -DOS_VERSION="${OS_VERSION}")

aux_source_directory(. DIR_SRCS)

add_executable(${PROJECT_NAME} ${DIR_SRCS})

#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/bin)


if(USE_MYSQL)
    target_link_libraries(${PROJECT_NAME} tarsservant tarsparse tarsutil ${MYSQL_DIR_LIB}/libmysqlclient.a
							pthread z dl rt)
else(USE_MYSQL)
    target_link_libraries(${PROJECT_NAME} tarsservant tarsparse tarsutil
                      pthread)
endif(USE_MYSQL)
