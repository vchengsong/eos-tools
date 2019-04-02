#include <iostream>

#include <eosio/chain/incremental_merkle.hpp>
//#include "incremental_merkle.hpp"

using namespace eosio::chain;
using namespace std;


struct sim_block {
   uint32_t             block_num;
   digest_type          block_id;
   incremental_merkle   blockroot_merkle;
};

std::vector<sim_block> sim_chain;


///-------------------------------///

/** merkle tree
 *  layer, Increase upward from leaf node layer, starting from 1
 *  layer index, Increase from left to right in every layer, starting from 1
 *
 *                  * root            layer 5 depth 1
 *          *               *         layer 4 depth 2
 *      *       *       *       *     layer 3 depth 3
 *    *   *   *   *   *   *   *   *   layer 2 depth 4
 *   * * * * * * * * * * * * * * * *  layer 1 depth 5  leafs
 *
 */

bool inc_merkle_verify( const incremental_merkle& inc_mkl ){
   auto max_depth = eosio::chain::detail::calcluate_max_depth( inc_mkl._node_count );
   auto current_depth = max_depth;
   auto index = inc_mkl._node_count;
   auto active_iter = inc_mkl._active_nodes.begin();
   digest_type top;

   if ( inc_mkl._active_nodes.size() == 1 ){
      return true;
   }

   while (current_depth > 1) {
      if ((index & 0x1)) { // left

         if ( top == digest_type() ){
            const auto& left_value = *active_iter;
            ++active_iter;
            top = digest_type::hash(make_canonical_pair(left_value, left_value));
         } else {
            top = digest_type::hash(make_canonical_pair(top, top));
         }

      } else { // right
         if ( top != digest_type()){
            const auto& left_value = *active_iter;
            ++active_iter;

            top = digest_type::hash(make_canonical_pair(left_value, top));
         }
      }

      // move up a level in the tree
      current_depth--;
      index = (index + 1) >> 1;
   }
   return top == inc_mkl.get_root();
}

digest_type get_block_id_by_num( uint32_t block_num ){
   return sim_chain[ block_num - sim_chain.front().block_num ].block_id;
}

incremental_merkle get_inc_merkle_by_block_num( uint32_t block_num ){
   return sim_chain[ block_num - sim_chain.front().block_num ].blockroot_merkle;
}

digest_type get_inc_merkle_layer_left_node( const incremental_merkle& inc_mkl, const uint32_t& layer ){
   auto max_layers = eosio::chain::detail::calcluate_max_depth( inc_mkl._node_count );

   if ( inc_mkl._node_count == 0 ){ elog("inc_mkl._node_count == 0"); return digest_type(); }
   if ( layer >= max_layers ){ elog("layer >= max_layers"); return digest_type(); }

   auto current_layer = 1;
   auto index = inc_mkl._node_count;
   auto active_iter = inc_mkl._active_nodes.begin();

   if ( layer == 1 ){
      return inc_mkl._active_nodes.back();
   }

   digest_type current_layer_node;

   while ( current_layer < max_layers ) {

      if ((index & 0x1)) { // left
         current_layer_node = *active_iter;
         ++active_iter;
      } else {
         current_layer_node = digest_type();
      }

      if ( current_layer == layer ){
         return current_layer_node;
      }

      // move up a level in the tree
      current_layer++;
      index = index >> 1;
   }

   return digest_type();
}

std::tuple<uint32_t,uint32_t,digest_type> get_inc_merkle_full_branch_root_cover_from( const uint32_t& from_block_num, const incremental_merkle& inc_mkl ) {
   if ( from_block_num > inc_mkl._node_count ){
      elog("from_block_num > inc_mkl._node_count");
      return std::tuple<uint32_t,uint32_t,digest_type>();
   }

   auto max_layers = eosio::chain::detail::calcluate_max_depth( inc_mkl._node_count );
   auto current_layer = 1;
   auto index = inc_mkl._node_count;
   auto active_iter = inc_mkl._active_nodes.begin();

   digest_type current_layer_node = digest_type();

   while (current_layer <= max_layers ) {
      std::cout << "current_layer" << current_layer << std::endl;

      if (index & 0x1) { // left
         current_layer_node = *active_iter;
         ++active_iter;

         uint32_t first, last;
         uint32_t diff = current_layer - 1;
         last = index << diff;
         first = last - (1 << diff) + 1;

         std::cout << "first: " << first << " last: " << last << std::endl;

         if ( first <= from_block_num && from_block_num <= last ){
            return { current_layer, index, current_layer_node };
         }
      }

      // move up a level in the tree
      current_layer++;
      index = index >> 1;
   }

   elog("can not get_inc_merkle_full_branch_root_cover_from");
   return std::tuple<uint32_t,uint32_t,digest_type>();
}

// nodes in layer range [ 2, max_layers ]
std::vector<std::pair<uint32_t,uint32_t>> get_merkle_path_positions_to_layer_in_full_branch( uint32_t from_block_num, uint32_t to_layer ){
   std::vector<std::pair<uint32_t,uint32_t>> path;
   if ( to_layer < 2 ){ elog("to_layer < 2"); return path; }
   if ( to_layer > eosio::chain::detail::calcluate_max_depth(from_block_num) ){ elog("to_layer > max_depth"); return path; }

   auto index = from_block_num;
   auto current_layer = 2;
   while ( current_layer < to_layer ){
      index = ( index + 1 ) >> 1;
      if ( index & 0x1 ){
         path.emplace_back( current_layer, index + 1 );
      } else{
         path.emplace_back( current_layer, index - 1 );
      }
      current_layer++;
   }

   index = ( index + 1 ) >> 1;
   path.emplace_back( to_layer, index );
   return path;
}

digest_type get_merkle_node_value_in_full_sub_branch( const incremental_merkle& reference_inc_merkle, uint32_t layer, uint32_t index ){
   if ( layer < 2 ){ elog("to_layer < 2"); return digest_type(); }

   if ( index & 0x1 ){
      auto ret = get_inc_merkle_layer_left_node( reference_inc_merkle, layer ); // search in reference_inc_merkle first
      if ( ret != digest_type() ){
         return ret;
      } else {
         auto inc_merkle = get_inc_merkle_by_block_num( (index << (layer - 1)) + 1 );
         return inc_merkle._active_nodes.front();
      }
   } else {
      auto block_num = index << ( layer - 1 );
      auto inc_merkle = get_inc_merkle_by_block_num( block_num );
      auto active_iter = inc_merkle._active_nodes.begin();
      auto block_id = get_block_id_by_num( block_num );

      uint32_t current_layer = 1;
      digest_type top = block_id;
      while ( current_layer < layer ){
         const auto& left_value = *active_iter;
         ++active_iter;
         top = digest_type::hash(make_canonical_pair(left_value, top));
         current_layer++;
      }
      return top;
   }
}

std::vector<digest_type> get_block_id_merkle_path_to_anchor_block( uint32_t from_block_num, uint32_t anchor_block_num ){
   std::vector<digest_type> result;
   if ( from_block_num >= anchor_block_num ){ elog("from_block_num >= anchor_block_num"); return result; }

   auto inc_merkle = get_inc_merkle_by_block_num( anchor_block_num );
   uint32_t full_root_layer;
   uint32_t full_root_index;
   digest_type full_root_value;
   std::tie( full_root_layer, full_root_index, full_root_value ) = get_inc_merkle_full_branch_root_cover_from( from_block_num, inc_merkle );
   auto position_path = get_merkle_path_positions_to_layer_in_full_branch( from_block_num, full_root_layer );

   if ( position_path.back().first != full_root_layer || position_path.back().second != full_root_index ){
      elog("internal error! position_path.back() calculate failed");
      return result;
   }

   // add the first two elements to merkle path
   if ( from_block_num % 2 == 1 ){ // left side
      result.push_back( get_block_id_by_num( from_block_num ) );
      result.push_back( get_block_id_by_num( from_block_num + 1 ) );
   } else { // right side
      result.push_back( get_block_id_by_num( from_block_num - 1) );
      result.push_back( get_block_id_by_num( from_block_num ) );
   }

   position_path.erase( --position_path.end() );
   for( auto p : position_path ){
      auto value = get_merkle_node_value_in_full_sub_branch( inc_merkle, p.first, p.second );
      if ( p.second & 0x1 ){
         result.push_back( make_canonical_left(value) );
      } else {
         result.push_back( make_canonical_right(value) );
      }
   }

   return result;
}



///////////////////////////////

void print_merkle_tree(vector<digest_type> ids) {
   if( 0 == ids.size() ) { return; }
   int layer = 0;

   while( ids.size() > 1 ) {

      cout << "layer: " << layer++ << endl;
      for ( int i = 0; i < ids.size(); i++ ){ cout << "ids[" << i << "] = " << string(ids[i]) << endl; }

      if( ids.size() % 2 )
         ids.push_back(ids.back());

      for (int i = 0; i < ids.size() / 2; i++) {
         ids[i] = digest_type::hash(make_canonical_pair(ids[2 * i], ids[(2 * i) + 1]));
      }

      ids.resize(ids.size() / 2);
   }

   cout << "root: " << string(ids.front()) << endl;
}

digest_type generate_digest(){
   string s;
   for ( int i = 0; i < 10; i++ ) {
      s.append(to_string(std::rand()));
   }
   return digest_type{s};
}

incremental_merkle generate_inc_merkle( uint32_t amount ){
   vector<digest_type> digests;
   for (int n = 0; n < amount; n++) {
      digests.push_back( generate_digest() );
   }

   print_merkle_tree( digests );

   incremental_merkle inc_merkle;
   digest_type root;
   for (int n = 0; n < amount; n++) {
      root = inc_merkle.append(digests[n]);
      // cout << "digests[" << n << "] = " << string(digests[n]) << endl;
      // cout << "root = " << string(root) << endl << endl;
   }
   return inc_merkle;
}


void dump_inc_merkle( incremental_merkle inc_merkle ){
   for( auto d: inc_merkle._active_nodes ){
      cout << "inc merkle digest: " << string(d) << endl;
   }
}


int main() {
    std::srand(std::time(nullptr));
//   std::srand(0);


   cout << string(generate_digest()) << endl;
   cout << string(generate_digest()) << endl;

   // fill sim_chain

























//for ( int i = 1; i < 5000; i ++){
//   incremental_merkle inc_merkle = generate_inc_merkle( i );
//
//   if ( inc_merkle_verify( inc_merkle ) ){
//      std::cout << i << "-------yes--------" << std::endl;
//   } else {
//      std::cout << i << "-------no--------" << std::endl;
//   }
//}
   {
//   incremental_merkle inc_merkle = generate_inc_merkle( 32 );
//
//   dump_inc_merkle(inc_merkle);
//
//   cout << "layer 7: " << string(get_inc_mkl_layer_node( inc_merkle, 7 )) << endl;
//   cout << "layer 6: " << string(get_inc_mkl_layer_node( inc_merkle, 6 )) << endl;
//   cout << "layer 5: " << string(get_inc_mkl_layer_node( inc_merkle, 5 )) << endl;
//   cout << "layer 4: " << string(get_inc_mkl_layer_node( inc_merkle, 4 )) << endl;
//   cout << "layer 3: " << string(get_inc_mkl_layer_node( inc_merkle, 3 )) << endl;
//   cout << "layer 2: " << string(get_inc_mkl_layer_node( inc_merkle, 2 )) << endl;
//   cout << "layer 1: " << string(get_inc_mkl_layer_node( inc_merkle, 1 )) << endl;
   }

//   {
//      incremental_merkle inc_merkle = generate_inc_merkle( 1667 );
//
//      dump_inc_merkle(inc_merkle);
//
//      auto res = get_branch_root( 1664, inc_merkle );
//
//      cout << std::get<0>(res) << endl;
//      cout << string( std::get<1>(res))  << endl;
//
//
//
//   }
//






   return 0;
}
