cmake_minimum_required(VERSION 3.10) 
# set the project name 
project(Loader) 
message(STATUS "### Begin to configure project ###")

# C++标准   
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# 编码格式
# 设置 C 语言编译 flags,  输入代码编码格式为 utf-8
set(CMAKE_C_FLAGS /source-charset:utf-8)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
# 
message(STATUS "C/C++ standard: ${CMAKE_CXX_STANDARD}")
# 设置编译结果输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)

# 设置可执行程序输出路径
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

# 指定编译子目录
add_subdirectory(src)
# 强制使用与项目相同的运行时库设置
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(extern/googletest)

add_subdirectory(extern/simpleProgram)
add_subdirectory(extern/simpleProgram2)
add_subdirectory(extern/testDll)
add_subdirectory(extern/simpleProgram3)
add_subdirectory(extern/simpleWindows)
add_subdirectory(test)
message(STATUS "### Done ###")               