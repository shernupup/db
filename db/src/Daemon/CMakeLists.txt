aux_source_directory(. SERVER_DAEMON_DIR)

add_library(DaemonLib STATIC ${SERVER_DAEMON_DIR})

target_link_libraries(DaemonLib CommonLib CommonUtil IOLib)