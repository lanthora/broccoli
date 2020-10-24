aux_source_directory(. DIR_LIB_SRCS)
add_library(remote ${DIR_LIB_SRCS})

target_link_libraries(remote frp)
target_link_libraries(remote locale)
