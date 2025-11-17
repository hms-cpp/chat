#cmake版本
cmake_minimum_required(VERSION 3.0)

#工程名
project(main)

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g -O2)

#头文件搜索路径
include_directories()

#库文件搜索路径
link_directories()

#设置需要编译的源文件列表
set(SRC_LIST test_muduo.cpp)

#设置可执行文件生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#生成可执行文件
add_executable(server ${SRC_LIST})

#链接的库文件
target_link_libraries(server muduo_net muduo_base pthread)