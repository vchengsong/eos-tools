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

       struct pbft_prepare {
          string uuid;
          uint32_t view;
          block_num_type block_num;
          block_id_type block_id;
          public_key_type public_key;
          fc::sha256 chain_id;
          signature_type producer_signature;
          time_point timestamp;
       };

       struct pbft_commit {
          string uuid;
          uint32_t view;
          block_num_type block_num;
          block_id_type block_id;
          public_key_type public_key;
          fc::sha256 chain_id;
          signature_type producer_signature;
          time_point timestamp;
       };

       struct pbft_state {
          block_id_type block_id;
          block_num_type block_num;
          vector<pbft_prepare> prepares;
          bool should_prepared;
          vector<pbft_commit> commits;
          bool should_committed;
       };

       using pbft_state_ptr = std::shared_ptr<pbft_state>;

       struct by_block_id;
       struct by_num;
       struct by_prepare_and_num;
       struct by_commit_and_num;
       typedef multi_index_container<
          pbft_state_ptr,
          indexed_by<
             hashed_unique<
             tag<by_block_id>,
             member<pbft_state, block_id_type, &pbft_state::block_id>,
             std::hash<block_id_type>
          >,
          ordered_non_unique<
             tag<by_num>,
             composite_key<
                pbft_state,
                member<pbft_state, uint32_t, &pbft_state::block_num>
             >,
             composite_key_compare<less<>>
          >,
          ordered_non_unique<
             tag<by_prepare_and_num>,
             composite_key<
                pbft_state,
                member<pbft_state, bool, &pbft_state::should_prepared>,
                member<pbft_state, uint32_t, &pbft_state::block_num>
             >,
             composite_key_compare<greater<>, greater<>>
          >,
          ordered_non_unique<
             tag<by_commit_and_num>,
             composite_key<
                pbft_state,
                member<pbft_state, bool, &pbft_state::should_committed>,
                member<pbft_state, uint32_t, &pbft_state::block_num>
             >,
             composite_key_compare<greater<>, greater<>>
          >
       >
       >
       pbft_state_multi_index_type;

       pbft_state_multi_index_type pbft_state_index;






    }
} /// namespace eosio::chain

FC_REFLECT(eosio::chain::pbft_prepare, (uuid)(view)(block_num)(block_id)(public_key)(chain_id)(producer_signature)(timestamp))
FC_REFLECT(eosio::chain::pbft_commit,  (uuid)(view)(block_num)(block_id)(public_key)(chain_id)(producer_signature)(timestamp))
FC_REFLECT(eosio::chain::pbft_state,   (block_id)(block_num)(prepares)(should_prepared)(commits)(should_committed))
