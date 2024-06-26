aux_source_directory(. SRC_LIST)

add_library(mymuduo SHARED ${SRC_LIST})

target_link_libraries(mymuduo pthread)