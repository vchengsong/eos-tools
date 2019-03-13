
add_executable(eosio-checkpoints checkpoints.cpp)
target_link_libraries(eosio-checkpoints ${LIBRARIES})

install( TARGETS eosio-checkpoints
        RUNTIME DESTINATION /usr/local/eosio/bin )