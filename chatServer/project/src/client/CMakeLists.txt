
aux_source_directory(. SRC_LIST)


add_executable(ChatClient ${SRC_LIST})


#依赖的文件
target_link_libraries(ChatClient pthread)