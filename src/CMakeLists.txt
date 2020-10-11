cmake_minimum_required (VERSION 3.18.4)


project(idle)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_EXE_LINKER_FLAGS "-static")

aux_source_directory(. DIR_SRCS)

add_subdirectory(core)
add_subdirectory(consumer)
add_subdirectory(producer)
add_subdirectory(handler)


add_executable(idle ${DIR_SRCS})

target_link_libraries(idle pthread)
target_link_libraries(idle consumer)
target_link_libraries(idle core)
target_link_libraries(idle handler)
target_link_libraries(idle producer)
