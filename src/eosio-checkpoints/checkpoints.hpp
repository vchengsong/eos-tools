#pragma once

#include <eosio/chain/controller.hpp>
#include <eosio/chain/fork_database.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace eosio {
    namespace chain {
       using boost::multi_index_container;
       using namespace boost::multi_index;
       using namespace std;
       using boost::uuids::uuid;


       struct pbft_checkpoint {
          string uuid;
          block_num_type block_num;
          block_id_type block_id;
          public_key_type public_key;
          fc::sha256 chain_id;
          signature_type producer_signature;
          time_point timestamp;
       };

       struct pbft_checkpoint_state {
          block_id_type block_id;
          block_num_type block_num;
          vector<pbft_checkpoint> checkpoints;
          bool is_stable;
       };

       using pbft_checkpoint_state_ptr = std::shared_ptr<pbft_checkpoint_state>;

       struct by_block_id;
       struct by_num;
       typedef multi_index_container<
       pbft_checkpoint_state_ptr,
       indexed_by<
          hashed_unique<
          tag<by_block_id>,
       member<pbft_checkpoint_state, block_id_type, &pbft_checkpoint_state::block_id>,
       std::hash<block_id_type>
       >,
       ordered_non_unique<
       tag<by_num>,
       composite_key<
          pbft_checkpoint_state,
          member<pbft_checkpoint_state, uint32_t, &pbft_checkpoint_state::block_num>
       >,
       composite_key_compare<less<>>
       >
       >
       >
       pbft_checkpoint_state_multi_index_type;

       pbft_checkpoint_state_multi_index_type checkpoint_index;
    }
} /// namespace eosio::chain

FC_REFLECT(eosio::chain::pbft_checkpoint, (uuid)(block_num)(block_id)(public_key)(chain_id)(producer_signature)(timestamp))
FC_REFLECT(eosio::chain::pbft_checkpoint_state, (block_id)(block_num)(checkpoints)(is_stable))