
# 定义了一个SRC_LIST变量，包含了该目录下的所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)
aux_source_directory(../net NET_LIST)

add_executable(Chatserver ${SRC_LIST} ${NET_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})
# target_link_libraries(Chatserver muduo_net muduo_base mysqlclient hiredis pthread)
target_link_libraries(Chatserver  mysqlclient hiredis pthread)
