cmake_minimum_required (VERSION 3.18.4)

project(idle)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 静态链接
set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")

aux_source_directory(. DIR_SRCS)

add_subdirectory(core)
add_subdirectory(demo)


add_executable(idle ${DIR_SRCS})

target_link_libraries(idle core)
target_link_libraries(idle demo)

