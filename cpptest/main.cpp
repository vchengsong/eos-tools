#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include <iostream>
#include <vector>


#include <eosio/chain/types.hpp>

#include <eosio/chain/controller.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/block.hpp>
#include <eosio/chain/types.hpp>
#include <eosio/chain/contract_types.hpp>


#include <fc/network/message_buffer.hpp>
#include <fc/network/ip.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>
#include <fc/log/appender.hpp>
#include <fc/container/flat.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/crypto/rand.hpp>
#include <fc/exception/exception.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/intrusive/set.hpp>

using mvo = fc::mutable_variant_object;

using namespace boost::asio;
using namespace eosio::chain;

using std::vector;

using boost::asio::ip::tcp;
using boost::asio::ip::address_v4;
using boost::asio::ip::host_name;
using boost::intrusive::rbtree;
using boost::multi_index_container;

using fc::time_point;
using fc::time_point_sec;
using eosio::chain::transaction_id_type;
using eosio::chain::name;
using mvo = fc::mutable_variant_object;
namespace bip = boost::interprocess;


int main()
{

   return 0;
}














