##
# @file CMakeLists.txt
# src目录的cmake文件
#
include_directories(${CMAKE_SOURCE_DIR}/include)

aux_source_directory(./ SP_SRC_DIR)
# 生成可执行文件
add_executable(InlineHook ${SP_SRC_DIR})
