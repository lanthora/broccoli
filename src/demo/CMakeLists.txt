aux_source_directory(. DIR_LIB_SRCS)
add_library(demo ${DIR_LIB_SRCS})

target_link_libraries(core pthread)
