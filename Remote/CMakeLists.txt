cmake_minimum_required(VERSION 3.14)  # 确保 CMake 版本不低于 3.14

# 设置项目名称
project(QRemote)
# 查找 Qt5 或 Qt6
find_package(Qt5 COMPONENTS Core REQUIRED)
# 如果你使用的是 Qt6，可以改为：
# find_package(Qt6 COMPONENTS Core REQUIRED)

# 包含 Qt 的头文件目录
include_directories(${Qt5Core_INCLUDE_DIRS})

# 添加可执行文件
add_executable(QRemote 
*.cpp)

# 链接 Qt 库
target_link_libraries(QRemote Qt5::Core)
# 如果你使用的是 Qt6，可以改为：
# target_link_libraries(QRemote Qt6::Core)

# 生成 moc 文件
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)