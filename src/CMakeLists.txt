##
# @file CMakeLists.txt
# src目录的cmake文件
#
include_directories(${CMAKE_SOURCE_DIR}/include)

set(LINK_DIR ${CMAKE_SOURCE_DIR}/lib)
link_directories(${LINK_DIR})


aux_source_directory(./ SRC_DIR)
aux_source_directory(./PELoader LOADER_SRC_DIR)
aux_source_directory(./AttackComponent ATTACK_SRC_DIR)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# 生成可执行文件
add_executable(${PROJECT_NAME}  ${LOADER_SRC_DIR} ${ATTACK_SRC_DIR} ${SRC_DIR} )
target_link_libraries(${PROJECT_NAME} ntdll user32)
# 生成用于测试的lib
add_library(FORTEST STATIC ${LOADER_SRC_DIR} ${ATTACK_SRC_DIR}  ${SRC_DIR} )

target_link_libraries(FORTEST ntdll)
