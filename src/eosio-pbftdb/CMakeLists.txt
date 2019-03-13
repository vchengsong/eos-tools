
add_executable(eosio-pbftlog pbftlog.cpp)
target_link_libraries(eosio-pbftlog ${LIBRARIES})

install( TARGETS eosio-pbftlog
        RUNTIME DESTINATION /usr/local/eosio/bin )