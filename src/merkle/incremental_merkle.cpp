#include <iostream>

//#include <eosio/chain/incremental_merkle.hpp>
#include "incremental_merkle.hpp"

using namespace eosio::chain;
using namespace std;


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

incremental_merkle generate_inc_merkle( uint32_t amount ){
   // std::srand(std::time(nullptr));
   std::srand(0);
   vector<digest_type> digests;

   for (int n = 0; n < amount; n++) {
      string s;
      for (int i = 0; i < 10; i++) { s.append(to_string(std::rand())); }
      digests.push_back(digest_type{s});
      s = "";
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

   {
      incremental_merkle inc_merkle = generate_inc_merkle( 1667 );

      dump_inc_merkle(inc_merkle);

      auto res = get_branch_root( 1664, inc_merkle );

      cout << std::get<0>(res) << endl;
      cout << string( std::get<1>(res))  << endl;



   }







   return 0;
}
