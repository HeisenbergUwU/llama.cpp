cmake_minimum_required(VERSION 3.10)

project(my_code)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
aux_source_directory(src SRC_LIST) # 将远吗文件保存在 SRC_LIST 中
# 编译选项（所有目标）
add_compile_options(-Wall -Wextra)
message(STATUS "SOURCES: ${SRC_LIST}")

foreach(src IN LISTS SRC_LIST)
    get_filename_component(target_name "${src}" NAME_WE)
    add_executable(${target_name} "${src}")
    
    # 给单个目标加编译选项
    target_compile_options(${target_name} PRIVATE -O3)
    
    # 加链接库
    if(target_name STREQUAL "atomic_signal")
        target_link_libraries(${target_name} PRIVATE pthread)
    endif()
endforeach()
