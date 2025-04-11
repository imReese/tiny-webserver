cmake_minimum_required(VERSION 3.10)
project(tiny_webserver)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 查找pkg-config
find_package(PkgConfig REQUIRED)

# 使用pkg-config查找MySQL和JSON
pkg_check_modules(MYSQL REQUIRED mysqlclient)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# 添加头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/backend
    ${PROJECT_SOURCE_DIR}/backend/src
    ${PROJECT_SOURCE_DIR}/backend/src/config
    ${PROJECT_SOURCE_DIR}/backend/src/core
    ${PROJECT_SOURCE_DIR}/backend/src/core/http
    ${PROJECT_SOURCE_DIR}/backend/src/utils
    ${PROJECT_SOURCE_DIR}/backend/src/utils/block_queue
    ${PROJECT_SOURCE_DIR}/backend/src/utils/lock
    ${PROJECT_SOURCE_DIR}/backend/src/utils/log
    ${PROJECT_SOURCE_DIR}/backend/src/utils/threadpool
    ${PROJECT_SOURCE_DIR}/backend/src/utils/timer
    ${PROJECT_SOURCE_DIR}/backend/src/third_party
    ${MYSQL_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

# 添加源文件
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/config/*.cpp"
    "src/core/*.cpp"
    "src/core/http/*.cpp"
    "src/utils/*.cpp"
    "src/utils/block_queue/*.cpp"
    "src/utils/lock/*.cpp"
    "src/utils/log/*.cpp"
    "src/utils/threadpool/*.cpp"
    "src/utils/timer/*.cpp"
    "src/third_party/*.cpp"
)

# 打印源文件列表用于调试
message(STATUS "Source files: ${SOURCES}")

# 添加可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接MySQL和JSON库
target_link_libraries(${PROJECT_NAME} ${MYSQL_LIBRARIES} ${JSONCPP_LIBRARIES})

# 设置输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin) 