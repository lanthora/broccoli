cmake_minimum_required (VERSION 3.18.4)

project(broccoli)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive")

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")

aux_source_directory(. DIR_SRCS)

add_subdirectory(core)
add_subdirectory(util)
add_subdirectory(log)
add_subdirectory(frp)
add_subdirectory(locale)
add_subdirectory(remote)

add_executable(broccoli ${DIR_SRCS})

target_link_libraries(broccoli core)
target_link_libraries(broccoli util)
target_link_libraries(broccoli log)
target_link_libraries(broccoli frp)
target_link_libraries(broccoli locale)
target_link_libraries(broccoli remote)

target_link_libraries(broccoli cryptopp)
target_link_libraries(broccoli pthread)


