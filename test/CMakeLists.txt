##
# @file CMakeLists.txt
# test目录的cmake文件
#

# gtest.cpp 测试框架

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

aux_source_directory(./ TEST_DIR)
add_executable(utest ${TEST_DIR})
target_link_libraries(utest gtest gtest_main)
target_link_libraries(utest FORTEST)