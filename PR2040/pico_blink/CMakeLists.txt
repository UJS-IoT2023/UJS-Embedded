cmake_minimum_required(VERSION 3.13)

# 1. 自动寻找 pico_sdk_import.cmake
# 优先找项目根目录下的，找不到再去环境变量里找
if (NOT PICO_SDK_PATH)
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(blink C CXX ASM) # 建议加上 CXX 和 ASM 支持
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICOTOOL_FETCH_FROM_GIT OFF)
# 2. 初始化 SDK
pico_sdk_init()

# 3. 添加你的源文件（如果是 C++ 请改为 blink.cpp）
add_executable(blink blink.c)

# 4. 链接标准库
target_link_libraries(blink pico_stdlib)

# 5. 开启/关闭 串口输出 (非常实用)
pico_enable_stdio_usb(blink 1)  # 开启 USB 串口打印
pico_enable_stdio_uart(blink 0) # 关闭 物理 UART 串口打印

# 6. 生成 .uf2 文件
pico_add_extra_outputs(blink)