cmake_minimum_required(VERSION 2.8)

project(Tseer)

# 根目录
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# tars依赖目录
set(TARS_DIR "${ROOT_DIR}/thirdparty/tars")
set(TARS_DIR_INC "${TARS_DIR}/include")
set(TARS_DIR_LIB "${TARS_DIR}/lib")
set(TARS2CPP "${TARS_DIR}/tools/tars2cpp")

# 安装路径
if(NOT INSTALL_PATH)
    set(INSTALL_PATH "/usr/local")
endif(NOT INSTALL_PATH)
set(CMAKE_INSTALL_PREFIX ${INSTALL_PATH}/${PROJECT_NAME})

# 是否采用Mysql来存储
if(NOT USE_MYSQL)
    set(USE_MYSQL OFF)
endif(NOT USE_MYSQL)

# mysql依赖目录
if(NOT MYSQL_DIR)
    set(MYSQL_DIR "/usr/local/mysql")
endif(NOT MYSQL_DIR)
set(MYSQL_DIR_INC "${MYSQL_DIR}/include")
set(MYSQL_DIR_LIB "${MYSQL_DIR}/lib")

#编译依赖第三方框架
set(RAPID_JSON_DIR "${ROOT_DIR}/thirdparty")

#操作系统版本
set(OS_VERSION "CentOs-6.5-64")

# tars框架版本//todo
set(TARS_VERSION "1.0.1")

set(CMAKE_CXX_FLAGS "-g -O2 -Wall -Wno-deprecated")

#调用tars2cpp, 生成tars对应的文件
macro(complice_tars OUT_DEPENDS_LIST HEADER)
    set(DEPENDS_LIST)

    FILE(GLOB SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/*.tars")

    set(CLEAN_LIST)

    #设置tars生成后, cpp文件因为h文件的include路径
    set(INCLUDE)
    if (NOT ${HEADER} STREQUAL "")
        set(INCLUDE "--header=${HEADER}")
    endif ()

    foreach (FILE ${SRC_LIST})

        get_filename_component(NAME_WE ${FILE} NAME_WE)

        # 生成tars文件
        set(TARS_IN ${FILE})
        set(TARS_H ${NAME_WE}.h)

        add_custom_command(
                OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${TARS_H}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                DEPENDS ${INCLUDE} ${TARS_IN}
                COMMAND ${TARS2CPP} --with-tars ${INCLUDE} ${TARS_IN}
                COMMENT "${TARS2CPP} --with-tars ${INCLUDE} ${TARS_IN}")

        list(APPEND DEPENDS_LIST ${CMAKE_CURRENT_SOURCE_DIR}/${TARS_H})

        #设置需要清除的文件
        get_filename_component(PATH ${FILE} PATH)
        list(APPEND CLEAN_LIST ${PATH}/${NAME_WE}.h)
        if (EXISTS ${PATH}/${NAME_WE}.cpp)
            list(APPEND CLEAN_LIST ${PATH}/${NAME_WE}.cpp)
        endif (EXISTS ${PATH}/${NAME_WE}.cpp)

    endforeach (FILE)

    set(OUT_DEPENDS_LIST ${DEPENDS_LIST})

    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEAN_LIST}")

endmacro()

add_subdirectory(TseerServer)
add_subdirectory(TseerAgent)
add_subdirectory(api/cplus)
#add_subdirectory(examples)
