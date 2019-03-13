/**
 *  @file
 *  @copyright defined in eosio/LICENSE.txt
 */
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/block_log.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain/reversible_block_object.hpp>

#include <fc/io/json.hpp>
#include <fc/filesystem.hpp>
#include <fc/variant.hpp>
#include <fc/io/fstream.hpp>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>

#include "pbft_database.hpp"


using namespace eosio::chain;
namespace bfs = boost::filesystem;
namespace bpo = boost::program_options;
using bpo::options_description;
using bpo::variables_map;
optional<chainbase::database> dumy; //非常奇怪的错误，在有些MAC上编译必须要有此声明，不然报链接的错误；


struct blocklog {
   blocklog()
   {}

   void read_log();
   void set_program_options(options_description& cli);
   void initialize(const variables_map& options);

   bfs::path                        blocks_dir;
   bfs::path                        output_file;
   uint32_t                         first_block;
   uint32_t                         last_block;
   bool                             no_pretty_print;
   bool                             as_json_array;

   bool                             info;
   bool                             print_packed_header;
   bool                             print_packed_trx;

   uint32_t                         pack_headers_from;
   uint32_t                         pack_headers_interval;
   uint32_t                         pack_headers_times;
};

template <typename T>
void print_packed_data(std::ostream* out, const string& name, const T &v){
   bytes s = fc::raw::pack(v);
   *out << name << '=' << "'\"" << fc::to_hex(s.data(),s.size()) <<"\"'" << "\n";
};

void print_hex(std::ostream* out, const string& name, const char* str, const int size){
   *out << name << '=' << "'\"" << fc::to_hex(str,size)  <<"\"'" << "\n";
};

template <typename T>
void print_var(std::ostream* out, const string& name, const T &v){
   *out << name << '=' << "'"<<  fc::json::to_string(v)  <<"'" << "\n";
};

struct transaction_receipt_type : public transaction_receipt_header {
   packed_transaction trx;
};

FC_REFLECT_DERIVED(transaction_receipt_type, (eosio::chain::transaction_receipt_header), (trx) )

void blocklog::read_log() {

   std::ostream* out = &std::cout;
   fc::variant pretty_output;
   const fc::microseconds deadline = fc::seconds(10);
   auto print_info = [&](pbft_state& next) {
      abi_serializer::to_variant(next, pretty_output, []( account_name n ) { return optional<abi_serializer>(); }, deadline);
      const auto enhanced_object = fc::mutable_variant_object
         (pretty_output.get_object());
      fc::variant v(std::move(enhanced_object));
      if (no_pretty_print)
         fc::json::to_stream(*out, v, fc::json::stringify_large_ints_and_doubles);
      else
         *out << fc::json::to_pretty_string(v) << "\n";
   };

   fc::path pbft_db_dir = blocks_dir;
   auto pbft_db_dat = pbft_db_dir / "pbftdb.dat";
   if (fc::exists(pbft_db_dat)) {
      string content;
      fc::read_file_contents(pbft_db_dat, content);

      fc::datastream<const char *> ds(content.data(), content.size());

      // keep these unused variables.
      uint32_t current_view;
      fc::raw::unpack(ds, current_view);

      unsigned_int size;
      fc::raw::unpack(ds, size);
      for (uint32_t i = 0, n = size.value; i < n; ++i) {
         pbft_state s;
         fc::raw::unpack(ds, s);
         pbft_state_index.insert(std::make_shared<pbft_state>(move(s)));
      }
   }

   const auto &by_commit_and_num_index = pbft_state_index.get<by_commit_and_num>();
   for ( auto itr = by_commit_and_num_index.begin(); itr != by_commit_and_num_index.end(); itr++ ){
      pbft_state_ptr psp = *itr;
      print_info(*psp);
   }
}

void blocklog::set_program_options(options_description& cli)
{
   cli.add_options()
      ("blocks-dir,d", bpo::value<bfs::path>()->default_value("blocks"),
       "the location of the blocks directory (absolute path or relative to the current directory)")
      ("output-file,o", bpo::value<bfs::path>(),
       "the file to write the block log output to (absolute or relative path).  If not specified then output is to stdout.")
      ("first,f", bpo::value<uint32_t>(&first_block)->default_value(1),
       "the first block number to log")
      ("last,l", bpo::value<uint32_t>(&last_block)->default_value(std::numeric_limits<uint32_t>::max()),
       "the last block number (inclusive) to log")
      ("no-pretty-print", bpo::bool_switch(&no_pretty_print)->default_value(false),
       "Do not pretty print the output.  Useful if piping to jq to improve performance.")
      ("as-json-array", bpo::bool_switch(&as_json_array)->default_value(false),
       "Print out json blocks wrapped in json array (otherwise the output is free-standing json objects).")
      ("info,i", bpo::bool_switch(&info)->default_value(false),
       "Only print the first and last block number in forkdb of current chain.")
      ("print-packed-header", bpo::bool_switch(&print_packed_header)->default_value(false),
       "Print packed header.")
      ("print-packed-trx", bpo::bool_switch(&print_packed_trx)->default_value(false),
       "Print packed transaction.")
      ("pack-headers-from", bpo::value<uint32_t>(&pack_headers_from)->default_value(0),
       "Packed headers from.")
      ("pack-headers-interval", bpo::value<uint32_t>(&pack_headers_interval)->default_value(10),
       "Packed headers amount.")
      ("pack-headers-times", bpo::value<uint32_t>(&pack_headers_times)->default_value(1),
       "Print Packed headers times.")
      ("help,h", "Print this help message and exit.")
      ;

}

void blocklog::initialize(const variables_map& options) {
   try {
      auto bld = options.at( "blocks-dir" ).as<bfs::path>();
      if( bld.is_relative())
         blocks_dir = bfs::current_path() / bld;
      else
         blocks_dir = bld;

      if (options.count( "output-file" )) {
         bld = options.at( "output-file" ).as<bfs::path>();
         if( bld.is_relative())
            output_file = bfs::current_path() / bld;
         else
            output_file = bld;
      }
   } FC_LOG_AND_RETHROW()

}


int main(int argc, char** argv)
{
   std::ios::sync_with_stdio(false); // for potential performance boost for large block log files
   options_description cli ("eosio-blocklog command line options");
   try {
      blocklog blog;
      blog.set_program_options(cli);
      variables_map vmap;
      bpo::store(bpo::parse_command_line(argc, argv, cli), vmap);
      bpo::notify(vmap);
      if (vmap.count("help") > 0) {
         cli.print(std::cerr);
         return 0;
      }
      blog.initialize(vmap);
      blog.read_log();
   } catch( const fc::exception& e ) {
      elog( "${e}", ("e", e.to_detail_string()));
      return -1;
   } catch( const boost::exception& e ) {
      elog("${e}", ("e",boost::diagnostic_information(e)));
      return -1;
   } catch( const std::exception& e ) {
      elog("${e}", ("e",e.what()));
      return -1;
   } catch( ... ) {
      elog("unknown exception");
      return -1;
   }

   return 0;
}
