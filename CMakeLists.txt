#工程名称
project(db)
#cmake版本要求检查
cmake_minimum_required(VERSION 3.2.0)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Require at least gcc 6
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6 AND NOT CMAKE_VERSION VERSION_LESS 2.8.9)
        message (FATAL_ERROR "GCC version must be at least 6! For example, if GCC 6 is available under gcc-6, g++-6 names, do the following: export CC=gcc-6 CXX=g++-6; rm -rf CMakeCache.txt CMakeFiles; and re run cmake or ./release.")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Require at least clang 3.8
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.8)
        message (FATAL_ERROR "Clang version must be at least 3.8! Recommended 4+")
    endif ()
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4)
        message (WARNING "Compilation has only been tested with Clang 4+")
    endif ()
else ()
    message (WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang 4+ and GCC 6+.")
endif ()

# 设置根目录
SET(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}")
#是否不启用系统Poco，由于我们直接依赖源码包，因此设置为1
option(NOT_UNBUNDLED "not to use system poco library" 1)

#开启c++11,不直接使用add_definitions(-std=c++11)的原因是：
#编译c代码时会有警告:
#cc1: warning: command line option ‘-std=c++11’ is valid for C++/ObjC++ but not for C
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
message(STATUS "optional:-std=c++11")   

#打印当前根目录路径（绝对路径）
message("project root path is : " ${PROJECT_ROOT_PATH} )

#执行集成poco的相关cmake命令
include(cmake/find_poco.cmake)

#添加子目录
add_subdirectory(contrib)
add_subdirectory(util)
add_subdirectory(db)
