#include <assert.h>
#include <string>
#include "delta-crdts.cc"
#include "cat/cat.h"
#include "message.pb.h"

using namespace std;

template<typename T>
void assert_equals(T t1, T t2, string s = "check")
{
  assert(t1 == t2);
  cout << s << endl;
}

template<typename T>
void assert_not_equals(T t1, T t2, string s = "check")
{
  assert(!(t1 == t2));
  cout << s << endl;
}

void test_gset()
{
  proto::message message_i;
  gset<int> gi;
  gi.add(2);
  gi.add(4);
  message_i << gi;

  gset<int> gi_;
  message_i >> gi_;
  assert_equals(gi, gi_);

  proto::message message_s;
  gset<string> gs;
  gs.add("abc");
  gs.add("xyz");
  message_s << gs;

  gset<string> gs_;
  message_s >> gs_;
  assert_equals(gs, gs_);

  gset<string> gs__;
  gs__.add("abc");
  assert_not_equals(gs, gs__);

  proto::message message_c;
  gset<cat> gc;
  cat c0(0, "zero");
  cat c1(1, "one");
  gc.add(c0);
  gc.add(c1);
  message_c << gc;

  gset<cat> gc_;
  message_c >> gc_;
  assert_equals(gc, gc_);
}

void test_twopset()
{
  proto::message message;
  twopset<string> ts;
  ts.add("hello");
  ts.add("world");
  ts.add("my");
  ts.rmv("my");
  message << ts;

  twopset<string> ts_;
  message >> ts_;
  assert_equals(ts, ts_);

  twopset<string> ts__;
  ts__.rmv("my");
  assert_not_equals(ts, ts__);
}

void test_gcounter()
{
  proto::message message;
  gcounter<> o1("idx");
  gcounter<> o2("idy");
  o1.inc();
  o1.join(o2.inc(4));
  message << o1;

  gcounter<> o1_;
  message >> o1_;
  assert_equals(o1, o1_);
  assert_not_equals(o1, o2);
}

int main(int argc, char * argv[])
{
  cout << "testing gset..." << endl;
  test_gset();
  cout << "testing twopset..." << endl;
  test_twopset();
  cout << "testing gcounter..." << endl;
  test_gcounter();
}

