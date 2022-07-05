#include <doctest/doctest.h>
#include <greeter/cereal.h>
#include <stdint.h>
#include <vector>
#include <algorithm>
#include <tuple>
#include <fmt/core.h>
#include <fmt/format.h>


using namespace fmt;

struct header {
    int8_t   a[4];
    int32_t  size;
    int8_t   c[4];
};

struct data {
  
    int32_t  a;
    int32_t  b;
    uint32_t c;
};

struct example {
  header h;
  data d;
};


template <> struct fmt::formatter<data>{
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()){
    auto it = ctx.begin(), end = ctx.end();
    
    if( it != end && *it != '}') throw format_error("invalidformat");
    
    return it;
  }
  
  template <typename FormatContext>
  auto format(const data &p, FormatContext &ctx){
    return format_to(ctx.out(), "\n {} {} {} \n",p.a, p.b, p.c);
  }
};

template <> struct fmt::formatter<header>{
  constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()){
    auto it = ctx.begin(), end = ctx.end();
    
    if( it != end && *it != '}') throw format_error("invalidformat");
    
    return it;
  }
  
  template <typename FormatContext>
  auto format(const header &p, FormatContext &ctx){
    return format_to(ctx.out(), "\n {} {} {} \n",p.a, p.size, p.c);
  }
};

auto tie(data const &lhs){
  return std::tie(lhs.a,lhs.b, lhs.c);
}

auto compare(data const &lhs, data const &rhs){
    return tie(lhs) == tie(rhs);
}

bool compare(header const &lhs, header const &rhs){
     return std::equal(std::begin(lhs.a),std::end(lhs.a), std::begin(rhs.a)) &&
            lhs.size == rhs.size &&
            std::equal(std::begin(lhs.c),std::end(lhs.c), std::begin(rhs.c));
}

bool compare(example const &lhs, example const &rhs){
  return compare(lhs.h, rhs.h) &&
         tie(lhs.d) == tie(rhs.d);
}

bool operator==(const data &l, const data &r){
  return tie(l) == tie(r);
}

bool operator<(const data &l, const data &r){
  return tie(l) < tie(r);
}

template <typename T>
static bool compare(std::vector<T> l, std::vector<T> r){
  if(l.size() != r.size()) return false;
  
  std::sort(l.begin(), l.end());
  std::sort(r.begin(), r.end());
  return (l == r);
}


TEST_CASE("Read simple data structure"){
  data e{1,2,3};
  char buffer[sizeof(e)];
  cereal c{buffer,sizeof(e)};
   c.store(e.a)
    .store(e.b)
    .store(e.c).flush();

   data r{};
   c.read(r.a)
    .read(r.b)
    .read(r.c).flush();
   INFO("example struct:\n ", e.a,e.b,e.c);
   INFO("read into struct:\n ",r.a,e.b,r.c);
   CHECK(compare(e,r));
  
    
   cereal cer{sizeof(e)};
   data t{};
   cer.store(e)
      .flush();
  
   cer.read(t).flush();
   CHECK(compare(e,t));
   CHECK(compare(r,t));
 
}

TEST_CASE("Read simple header structure"){
  header e{{'H','E','A','D'}, 10, {'T','O','P','S'}};

  cereal c{sizeof(e)};
  c.store(e.a)
   .store(e.size)
   .store(e.c).flush();
  
  header r{};
  c.read(r.a)
  .read(r.size)
  .read(r.c).flush();
  INFO("example struct:\n ", e.a[0],e.a[1],e.a[2],e.a[3],e.size,e.c[0],e.c[1],e.c[2],e.c[3]);
  INFO("example struct:\n ", r.a[0],r.a[1],r.a[2],r.a[3],r.size,r.c[0],r.c[1],r.c[2],r.c[3]);  
  CHECK(compare(e,r));
  
  header t{};
  c.read(t).flush();

  CHECK(compare(e,t));
  CHECK(compare(r,t));
}


TEST_CASE("Read example structure"){
 
 data d{1,2,3};
 header e{{'H','E','A','D'}, sizeof(d), {'T','O','P','S'}};
 
 example x = {e,d};

 char buffer[sizeof(x)];
 cereal c{buffer,sizeof(x)};
 
 c.store(x.h)
  .store(x.d).flush();
 
 example t = {};

 c.read(t.h)
 .read(t.d).flush();

 CHECK(compare(x,t));
 CHECK(compare(x.h,t.h));
 CHECK(compare(x.d,t.d));
  
 example q{};
 c.read(q).flush();
 
 CHECK(compare(x,q));
}

struct data_2 {
  header h;
  std::vector<data> d;
};


bool compare(data_2 const &lhs, data_2 const &rhs){
  if(!compare(lhs.h, rhs.h)) return false;
  if(!compare(lhs.d,rhs.d))  return false;
  
  return true;
}

TEST_CASE("dynamic structure"){
  data_2 m{};
  m.d.push_back({1,2,3});
  m.d.push_back({4,5,6});
  
  const int header_size =   m.d.size() *sizeof(data);  
  m.h = {{'H','E','A','D'}, header_size, {'T','O','P','S'}};

  const int size = sizeof(m.h) + header_size;
  std::vector<char> buffer;
  buffer.resize(size);
  cereal c{buffer};

  c.store(m.h)
   .store(m.d)
   .flush();
  
  data_2 t{};
  c.read(t.h)
   .read(t.d,t.h.size)
   .flush();
  
  CHECK(compare(m,t));
}


struct example1 {
  header h;
  data_2  d;  
};

bool compare(example1 const &l, example1 const &r){
  return compare(l.h,r.h) && compare(l.d,r.d);
}

TEST_CASE("Peek for header"){
  data_2 m{};
  m.d.push_back({1,2,3});
  m.d.push_back({4,5,6});
  
  const int data_size = m.d.size() * sizeof(data);
  m.h = {{'I','N','N','E'}, data_size, {'T','O','P','S'}}; 
  
  cereal c{sizeof(m)};
  c.store(m).flush();
  int8_t id[4];
  c.peek(id).consume(4).consume(4);
  CHECK(std::equal(std::begin(id), std::end(id), std::begin(m.h.a)));
  c.peek(id);
  CHECK(std::equal(std::begin(id), std::end(id), std::begin(m.h.c)));
}

TEST_CASE("Nested Structures"){
  
  data_2 m{};
  m.d.push_back({1,2,3});
  m.d.push_back({4,5,6});
  
  const int data_size = m.d.size() * sizeof(data);
  m.h = {{'I','N','N','E'}, data_size, {'T','O','P','S'}};
  
  example1 e{};
  e.d = m;
  const int ex_size = sizeof(m.h) + data_size;
  e.h = {{'H','E','A','D'}, ex_size, {'T','O','P','S'}};
  
  const int vector_size = sizeof(e);
  std::vector<char> buffer;
  buffer.resize(vector_size);
  
  cereal c{buffer};
   c.store(e.h)
    .store(e.d)
    .flush();
   
   example1 t{};
   c.read(e.h)
    .read(e.d.h);
    e.d.d = c.read<data>(e.d.h.size);
   c.flush();

  for(auto i: m.d){
    fmt::print("{}", i);
  }
  fmt::print("{}", m.d.size());
  fmt::print("\nnew: \n");
  for(auto i: e.d.d){
    fmt::print("{}",i);
  }
  fmt::print("{}",e.d.d.size());
  CHECK(compare(e,t));
}