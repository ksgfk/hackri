cmake_minimum_required (VERSION 3.8)

add_executable(hackri
    "main.cpp"
    "color.cpp"
    "buffer.cpp"
    "image.cpp"
    "renderer.cpp"
    "model.cpp")

target_include_directories(hackri PUBLIC ${HACKRI_INCLUDE})

if(MSVC)
    target_compile_definitions(hackri PUBLIC _CRT_SECURE_NO_WARNINGS) #CRT安全检查
    target_compile_definitions(hackri PUBLIC UNICODE                  #unicode字符宏
                                             _UNICODE)                #unicode字符宏
    target_compile_options(hackri PUBLIC /std:c++17                   #强制启用C++17
                                         /std:c11                     #强制启用C11
                                         /MP                          #多线程编译
                                         /utf-8)                      #强制utf8                         
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(hackri PUBLIC /Ob2                     #最高内联级别
                                             /Oi                      #内部函数
                                             /Ot                      #代码速度优先
                                             /Oy                      #省略帧指针
                                             /GF                      #启用字符串池
                                             /arch:AVX2)              #启用AVX2指令集
    endif()
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(hackri PUBLIC /GT)                     #纤程优化
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(hackri PUBLIC -stdlib=libc++)              #其他平台用clang时强制链接libc++
endif()