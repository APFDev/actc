/**
 *  @file
 *  @copyright defined in actc.cdt/LICENSE.txt
 */

#include <array>
#include <deque>
#include <list>
#include <set>
#include <string>
#include <vector>

#include <actc/tester.hpp>
#include <actc/binary_extension.hpp>
#include <actc/crypto.hpp>
#include <actc/datastream.hpp>
#include <actc/ignore.hpp>
#include <actc/symbol.hpp>

using std::array;
using std::begin;
using std::deque;
using std::end;
using std::fill;
using std::list;
using std::map;
using std::optional;
using std::pair;
using std::set;
using std::string;
using std::tuple;
using std::variant;
using std::vector;

using actc::binary_extension;
using actc::datastream;
using actc::fixed_bytes;
using actc::ignore;
using actc::ignore_wrapper;
using actc::pack;
using actc::pack_size;
using actc::public_key;
using actc::signature;
using actc::symbol;
using actc::symbol_code;
using actc::unpack;

// This data structure (which cannot be defined within a test macro block) needs both a default and a
// user-defined constructor for a specific `binary extension` test
struct be_test {
   be_test() : val{42} {}
   be_test(int i) : val{i} {}
   int val;
   ACTCLIB_SERIALIZE( be_test, (val) )
};

// Definitions in `actc.cdt/libraries/actc/datastream.hpp`
ACTC_TEST_BEGIN(datastream_test)
   static constexpr uint16_t buffer_size{256};
   char datastream_buffer[buffer_size]{}; // Buffer for the datastream to point to
   char buffer[buffer_size]; // Buffer to compare `datastream_buffer` with

   unsigned char j{0};
   for(int i{0}; i < buffer_size; ++i) // Fill the char array `datastream_buffer` with all 256 ASCII characters
      datastream_buffer[i] = j++;

   /// datastream(T, size_t)
   datastream<char*> ds{datastream_buffer, buffer_size};

   // inline void skip(size_t)
   ds.skip(1);
   CHECK_EQUAL( ds.pos(), datastream_buffer+1 )
   ds.skip(-1);
   CHECK_EQUAL( ds.pos(), datastream_buffer )

   // inline bool read(char*, size_t)
   CHECK_EQUAL( ds.read(buffer, 256), true )
   CHECK_EQUAL( memcmp(buffer, datastream_buffer, 256), 0)

   CHECK_ASSERT( "read", ([&]() {ds.read(buffer, 1);}) )

   // T pos()const
   CHECK_EQUAL( ds.pos(), datastream_buffer+256 )

   // inline bool seekp(size_t)
   ds.seekp(0);
   CHECK_EQUAL( ds.pos(), datastream_buffer )

   // inline bool write(const char*, size_t)
   fill(begin(buffer), end(buffer), 1); // Fill `buffer` with a new set of values

   CHECK_EQUAL( ds.write(buffer, 256), true )
   CHECK_EQUAL( memcmp(buffer, datastream_buffer, 256), 0 )

   CHECK_ASSERT( "write", ([&]() {ds.write(buffer, 1);}) )

   // inline bool put(char)
   ds.seekp(0);
   CHECK_EQUAL( ds.put('c'), true )
   *buffer = 'c';
   CHECK_EQUAL( memcmp(buffer, datastream_buffer, 256), 0 )

   ds.seekp(256);
   CHECK_ASSERT( "put", ([&]() {ds.put('c');}) )

   // inline bool get(unsigned char&)
   unsigned char uch{};

   ds.seekp(0);
   CHECK_EQUAL( ds.get(uch), true )
   CHECK_EQUAL( uch, 'c' )

   // inline bool get(char&)
   char ch{};

   ds.seekp(0);
   CHECK_EQUAL( ds.get(ch), true )
   CHECK_EQUAL( ch, 'c' )

   // inline bool valid()const
   ds.seekp(256);
   CHECK_EQUAL( ds.valid(), true )

   ds.seekp(257);
   CHECK_EQUAL( ds.valid(), false )

   // inline size_t tellp()const
   ds.seekp(0);
   CHECK_EQUAL( ds.tellp(), 0 )
   ds.seekp(256);
   CHECK_EQUAL( ds.tellp(), 256 )
   ds.seekp(257);
   CHECK_EQUAL( ds.tellp(), 257 )

   //inline size_t remaining()const
   ds.seekp(0);
   CHECK_EQUAL( ds.remaining(), 256 )
   ds.seekp(256);
   CHECK_EQUAL( ds.remaining(), 0 )
   ds.seekp(257);
   CHECK_EQUAL( ds.remaining(), -1)
ACTC_TEST_END

// Definitions in `actc.cdt/libraries/actc/datastream.hpp`
ACTC_TEST_BEGIN(datastream_specialization_test)
   static constexpr uint16_t buffer_size{256};
   char datastream_buffer[buffer_size]{}; // Buffer for the datastream to point to
   char buffer[buffer_size]; // Buffer to compare `datastream_buffer` with

   unsigned char j{0};
   for(int i{0}; i < buffer_size; ++i) // Fill the char array `datastream_buffer` with all all 256 ASCII characters
      datastream_buffer[i] = j++;

   /// datastream(T, size_t)
   datastream<size_t> ds{buffer_size};

   // inline void skip(size_t)
   // inline size_t tellp()const
   CHECK_EQUAL( ds.skip(0), true)
   CHECK_EQUAL( ds.tellp(), 256)

   CHECK_EQUAL( ds.skip(1), true)
   CHECK_EQUAL( ds.tellp(), 257)

   CHECK_EQUAL( ds.skip(255), true)
   CHECK_EQUAL( ds.tellp(), 512)

   CHECK_EQUAL( ds.skip(1028), true)
   CHECK_EQUAL( ds.tellp(), 1540)

   // inline bool seekp(size_t)
   ds.seekp(0);
   CHECK_EQUAL( ds.tellp(), 0)

   // inline bool write(const char*,size_t)
   CHECK_EQUAL( ds.write(buffer, 256), true )
   CHECK_EQUAL( ds.tellp(), 256 )

   CHECK_EQUAL( ds.write(buffer, 1), true )
   CHECK_EQUAL( ds.tellp(), 257 )

   // inline bool put(char)
   char ch{'c'};

   ds.seekp(0);
   CHECK_EQUAL( ds.put(ch), true )
   CHECK_EQUAL( ds.tellp(), 1 )

   // inline bool valid()
   ds.seekp(0);
   CHECK_EQUAL( ds.valid(), true )

   ds.seekp(1);
   CHECK_EQUAL( ds.valid(), true )

   ds.seekp(256);
   CHECK_EQUAL( ds.valid(), true )

   ds.seekp(257);
   CHECK_EQUAL( ds.valid(), true )

   // inline size_t remaining()
   ds.seekp(0);
   CHECK_EQUAL( ds.remaining(), 0 )

   ds.seekp(1);
   CHECK_EQUAL( ds.remaining(), 0 )

   ds.seekp(256);
   CHECK_EQUAL( ds.remaining(), 0 )

   ds.seekp(257);
   CHECK_EQUAL( ds.remaining(), 0 )
ACTC_TEST_END

// Definitions in `actc.cdt/libraries/actc/datastream.hpp`
ACTC_TEST_BEGIN(datastream_stream_test)
   static constexpr uint16_t buffer_size{256};
   char datastream_buffer[buffer_size]; // Buffer for the datastream to point to

   datastream<const char*> ds{datastream_buffer, buffer_size};

   // -------------
   // T (primitive)
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static constexpr int cprim{10};
   int prim{};
   ds << cprim;
   ds.seekp(0);
   ds >> prim;
   CHECK_EQUAL( cprim, prim )

   // -----------------
   // T (non-primitive)
   struct non_prim_test {
      int val;
   };

   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static constexpr non_prim_test cnonprim{10};
   non_prim_test nonprim{};
   ds << cnonprim;
   ds.seekp(0);
   ds >> nonprim;
   CHECK_EQUAL( cnonprim.val, nonprim.val )

   // ---------------
   // T[] (primitive)
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static constexpr int cprim_array[10]{0,1,2,3,4,5,6,7,8,9};
   int prim_array[10]{};
   ds << cprim_array;
   CHECK_EQUAL( ds.tellp(), 41 )

   ds.seekp(0);
   ds >> prim_array;
   CHECK_EQUAL( memcmp(cprim_array, prim_array, 10), 0 )

   ds.seekp(1);
   fill(begin(prim_array), end(prim_array), 0);
   CHECK_ASSERT( "T[] size and unpacked size don't match", [&](){ds >> prim_array;} )

   // -------------------
   // T[] (non-primitive)
   struct non_prim_array_test {
      int val;
   };

   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static constexpr non_prim_array_test cnon_prim_array[10]{0,1,2,3,4,5,6,7,8,9};
   non_prim_array_test non_prim_array[10]{};
   ds << cnon_prim_array;
   CHECK_EQUAL( ds.tellp(), 41 )

   ds.seekp(0);
   ds >> non_prim_array;
   CHECK_EQUAL( memcmp(cnon_prim_array, non_prim_array, 10), 0 )

   ds.seekp(1);
   fill(begin(non_prim_array), end(non_prim_array), non_prim_array_test{});
   CHECK_ASSERT( "T[] size and unpacked size don't match", [&](){ds >> non_prim_array;} )

   // ----
   // bool
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static constexpr bool cboolean{true};
   bool boolean{};
   ds << cboolean;
   ds.seekp(0);
   ds >> boolean;
   CHECK_EQUAL( cboolean, boolean )

   // ----------
   // std::array
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static constexpr array<char, 9> carr{'a','b','c','d','e','f','g','h','i'};
   array<char, 9> arr{};
   ds << carr;
   ds.seekp(0);
   ds >> arr;
   CHECK_EQUAL( carr, arr )

   // ----------
   // std::deque
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   const deque<char> cd{'a','b','c','d','e','f','g','h','i' };
   deque<char> d{};
   ds << cd;
   ds.seekp(0);
   ds >> d;
   CHECK_EQUAL( cd, d )

   // ---------
   // std::list
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const list<char> cl{'a','b','c','d','e','f','g','h','i' };
   list<char> l{};
   ds << cl;
   ds.seekp(0);
   ds >> l;
   CHECK_EQUAL( cl, l )

   // --------
   // std::map
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const map<char,int> cm{{'a',97}, {'b',98}, {'c',99}, {'d',100}};
   map<char,int> m{};
   ds << cm;
   ds.seekp(0);
   ds >> m;
   CHECK_EQUAL( cm, m )

   // -------------
   // std::optional
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const optional<char> co{'c'};
   optional<char> o{};
   ds << co;
   ds.seekp(0);
   ds >> o;
   CHECK_EQUAL( co, o )

   // ---------
   // std::pair
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const pair<char, int> cp{'c', 42};
   pair<char, int> p{};
   ds << cp;
   ds.seekp(0);
   ds >> p;
   CHECK_EQUAL( cp, p )

   // --------
   // std::set
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const set<char> cs{'a','b','c','d','e','f','g','h','i'};
   set<char> s{};
   ds << cs;
   ds.seekp(0);
   ds >> s;
   CHECK_EQUAL( cs, s )

   // -----------
   // std::string
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const string cstr {"abcdefghi"};
   string str{};
   ds << cstr;
   ds.seekp(0);
   ds >> str;
   CHECK_EQUAL( cstr, str )

   // ----------
   // std::tuple
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const tuple<string,int,char> ctup{"abcefghi",42,'a'};
   tuple<string,int,char> tup{};
   ds << ctup;
   ds.seekp(0);
   ds >> tup;
   CHECK_EQUAL( ctup, tup )

   // ------------
   // std::variant
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const variant<int, char> cv{1024};
   variant<int, char> v{};
   ds << cv;
   ds.seekp(0);
   ds >> v;
   CHECK_EQUAL( cv, v )

   // -----------
   // std::vector
   struct vec_test {
      char val;
   };

   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const vector<vec_test> cvec{{'a'},{'b'},{'c'},{'d'},{'e'},{'f'},{'g'},{'h'},{'i'}};
   vector<vec_test> vec{};
   ds << cvec;
   ds.seekp(0);
   ds >> vec;

   for (int i{0}; i < cvec.size(); ++i) {
      CHECK_EQUAL( cvec[i].val, vec[i].val )
   }

   // -----------------
   // std::vector<char>
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const vector<char> cchar_vec{'a','b','c','d','e','f','g','h','i'};
   vector<char> char_vec{};
   ds << cchar_vec;
   ds.seekp(0);
   ds >> char_vec;
   CHECK_EQUAL( cchar_vec, char_vec )

   // -----------------------
   // actc::binary_extension
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const binary_extension<char> cbe_char{'c'};
   binary_extension<char> cb_char{};
   ds << cbe_char;
   ds.seekp(0);
   ds >> cb_char;
   CHECK_EQUAL( cbe_char.value(), cb_char.value() )

   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   binary_extension<int> be_int0{};
   binary_extension<int> be_int1{};
   ds << be_int0.value_or(42);
   ds.seekp(0);
   ds >> be_int1;
   CHECK_EQUAL( be_int1.value(), 42 )

   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const be_test be_default{}; // Default constructing object
   static const binary_extension<be_test> cbe_test{be_default};
   binary_extension<be_test> cb_test{};
   ds << cbe_test;
   ds.seekp(0);
   ds >> cb_test;
   CHECK_EQUAL( cbe_test.value().val, cb_test.value().val )

   ds.seekp(256);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   binary_extension<be_test> be_default_test{};
   ds >> be_default_test;
   CHECK_EQUAL( 42, be_default_test.value_or().val )

   // ------------------
   // actc::fixed_bytes
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const fixed_bytes<32> cfb{fixed_bytes<32>::make_from_word_sequence<uint64_t>(1ULL,2ULL,3ULL,4ULL)};
   fixed_bytes<32> fb{};
   ds << cfb;
   ds.seekp(0);
   ds >> fb;
   CHECK_EQUAL( cfb, fb )

   // -------------
   // actc::ignore
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const ignore<char> cig{};
   ignore<char> ig;
   ds << cig;
   CHECK_EQUAL( ds.tellp(), 0 )
   ds >> ig;
   CHECK_EQUAL( ds.tellp(), 0 )

   // ---------------------
   // actc::ignore_wrapper
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const ignore_wrapper<char> cigw{'c'};
   char igw;
   ds << cigw;
   ds.seekp(0);
   ds >> igw;
   CHECK_EQUAL( cigw.value, igw )

   // -----------------
   // actc::public_key
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const public_key cpubkey{{},'a','b','c','d','e','f','g','h','i'};
   public_key pubkey{};
   ds << cpubkey;
   ds.seekp(0);
   ds >> pubkey;
   CHECK_EQUAL( cpubkey, pubkey )

   // ----------------
   // actc::signature
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const signature csig{{},'a','b','c','d','e','f','g','h','i'};
   signature sig{};
   ds << csig;
   ds.seekp(0);
   ds >> sig;
   CHECK_EQUAL( csig, sig )

   // -------------
   // actc::symbol
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const symbol csym_no_prec{"SYMBOLL", 0};
   symbol sym{};
   ds << csym_no_prec;
   ds.seekp(0);
   ds >> sym;
   CHECK_EQUAL( csym_no_prec, sym )

   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const symbol csym_prec{"SYMBOLL", 255};
   ds << csym_prec;
   ds.seekp(0);
   ds >> sym;
   CHECK_EQUAL( csym_prec, sym )

   // ------------------
   // actc::symbol_code
   ds.seekp(0);
   fill(begin(datastream_buffer), end(datastream_buffer), 0);
   static const symbol_code csc{"SYMBOLL"};
   symbol_code sc{};
   ds << csc;
   ds.seekp(0);
   ds >> sc;
   CHECK_EQUAL( csc, sc )
ACTC_TEST_END

// Definitions in `actc.cdt/libraries/actc/datastream.hpp`
ACTC_TEST_BEGIN(misc_datastream_test)
   // ---------------------------
   // vector<char> pack(const T&)
   static const string pack_str{"abcdefghi"};
   vector<char> pack_res{};
   pack_res = pack(pack_str);
   CHECK_EQUAL( memcmp(pack_str.data(), pack_res.data()+1, pack_str.size()), 0 )

   // --------------------------
   // size_t pack_size(const T&)
   int pack_size_i{42};
   double pack_size_d{3.14};
   string pack_size_s{"abcdefghi"};;
   CHECK_EQUAL( pack_size(pack_size_i),  4 )
   CHECK_EQUAL( pack_size(pack_size_d),  8 )
   CHECK_EQUAL( pack_size(pack_size_s), 10 )

   // -----------------------------
   // T unpack(const char*, size_t)
   static const char unpack_source_buffer[9]{'a','b','c','d','e','f','g','h','i'};
   char unpack_ch{};
   for (uint8_t i = 0; i < 9; ++i) {
      unpack_ch = unpack<char>(unpack_source_buffer+i, 9);
      CHECK_EQUAL( unpack_source_buffer[i], unpack_ch )
   }

   // -----------------------------
   // T unpack(const vector<char>&)
   static const vector<char> unpack_source_vec{'a','b','c','d','e','f','g','h','i'};
   for (uint8_t i = 0; i < 9; ++i) {
      unpack_ch = unpack<char>(unpack_source_buffer+i, 9);
      CHECK_EQUAL( unpack_source_buffer[i], unpack_ch )
   }
ACTC_TEST_END

int main(int argc, char* argv[]) {
   bool verbose = false;
   if( argc >= 2 && std::strcmp( argv[1], "-v" ) == 0 ) {
      verbose = true;
   }
   silence_output(!verbose);

   ACTC_TEST(datastream_test);
   ACTC_TEST(datastream_specialization_test);
   ACTC_TEST(datastream_stream_test);
   ACTC_TEST(misc_datastream_test);
   return has_failed();
}
