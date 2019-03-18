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






int main() {
   const int amount = 137;

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

   cout << endl << endl;


   // -- 1 --
   incremental_merkle inc_merkle;
   digest_type root;
   for (int n = 0; n < amount; n++) {
      root = inc_merkle.append(digests[n]);
//      cout << "digests[" << n << "] = " << string(digests[n]) << endl;
//      cout << "root = " << string(root) << endl << endl;
   }

   for( auto d: inc_merkle._active_nodes ){
      cout << "inc merkle digest: " << string(d) << endl;
   }

   cout << endl << endl;


   inc_merkle_verify( inc_merkle );



   return 0;
}




//      cout << "nodes : " << inc_merkle._node_count << endl;
//      for(auto& n: inc_merkle._active_nodes){
//         cout << string(n) << endl;
//      }
































