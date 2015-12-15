#include "delta-crdts.cc"
#include <google/protobuf/text_format.h>
#include "cat.h"
#include "crdt.pb.h"

void show_crdt_proto(const proto::crdt& crdt)
{
  cout << "byte size: " << crdt.ByteSize() << endl;
  string crdt_str;
  google::protobuf::TextFormat::PrintToString(crdt, &crdt_str);
  cout << crdt_str;
}

void test_gset()
{
  proto::crdt crdt_i;
  gset<int> gi;
  gi.add(2);
  gi.add(4);
  crdt_i << gi;
  show_crdt_proto(crdt_i);

  gset<int> gi_;
  crdt_i >> gi_;
  cout << gi_ << endl;
  cout << "----" << endl;

  proto::crdt crdt_s;
  gset<string> gs;
  gs.add("abc");
  gs.add("xyz");
  crdt_s << gs;
  show_crdt_proto(crdt_s);

  gset<string> gs_;
  crdt_s >> gs_;
  cout << gs_ << endl;
  cout << "----" << endl;

  proto::crdt crdt_c;
  gset<cat> gc;
  cat c0(0, "zero");
  cat c1(1, "one");
  gc.add(c0);
  gc.add(c1);
  crdt_c << gc;
  show_crdt_proto(crdt_c);

  gset<cat> gc_;
  crdt_c >> gc_;
  cout << gc_ << endl;
  cout << "----" << endl;
}

void test_twopset()
{
  proto::crdt crdt;
  twopset<string> ts;
  ts.add("hello");
  ts.add("world");
  ts.add("my");
  ts.rmv("my");
  crdt << ts;
  show_crdt_proto(crdt);

  twopset<string> ts_;
  crdt >> ts_;
  cout << ts_ << endl;
  cout << "----" << endl;
}

void test_gcounter()
{
  proto::crdt crdt;
  gcounter<> o1("idx");
  gcounter<> o2("idy");
  o1.inc();
  o1.join(o2.inc(4));
  crdt << o1;
  show_crdt_proto(crdt);

  gcounter<> o1_;
  crdt >> o1_;
  cout << o1_ << endl;
  cout << "----" << endl;
}

int main(int argc, char * argv[])
{
  test_gset();
  test_twopset();
  test_gcounter();
}
