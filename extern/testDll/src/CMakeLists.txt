##
# @file CMakeLists.txt
# src目录的cmake文件
#
include_directories(${CMAKE_SOURCE_DIR}/include)

set(LINK_DIR ${CMAKE_SOURCE_DIR}/lib)
link_directories(${LINK_DIR})


aux_source_directory(./ SP_SRC_DIR)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# 生成可执行文件
add_library(testDll SHARED ${SP_SRC_DIR})
target_compile_definitions(testDll PRIVATE MYLIBRARY_EXPORTS)
