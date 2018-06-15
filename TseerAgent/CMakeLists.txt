project(TseerAgent)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -O2 -Wall -Wno-deprecated")

add_definitions(-DTARS_VERSION="${TARS_VERSION}" -DOS_VERSION="${OS_VERSION}")

set(TSEERAGENT_SERVER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${TSEERAGENT_SERVER_ROOT_DIR}/protocol
                    ${TARS_DIR_INC}
                    ${TARS_DIR_INC}/servant)

link_directories(${TARS_DIR_LIB})

set(SERVER_NAME ${PROJECT_NAME})

add_subdirectory(protocol)
add_subdirectory(src)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.conf DESTINATION ${PROJECT_BINARY_DIR}/${PROJECT_NAME}/conf)