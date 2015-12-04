#include "delta-crdts.cc"
#include <google/protobuf/text_format.h>
#include "cat.h"
#include "crdt.pb.h"

void show_crdt(const delta::crdt& crdt)
{
  string crdt_str;
  google::protobuf::TextFormat::PrintToString(crdt, &crdt_str);
  cout << crdt_str;
}

void test_dump_and_load()
{
  delta::crdt crdt_i;

  gset<int> gi;
  gi.add(2);
  gi.add(4);
  dump(crdt_i, gi);
  show_crdt(crdt_i);

  gset<int> gi_;
  load(crdt_i, gi_);
  cout << gi_ << endl;
  cout << "----" << endl;

  delta::crdt crdt_s;

  gset<string> gs;
  gs.add("abc");
  gs.add("xyz");
  dump(crdt_s, gs);
  show_crdt(crdt_s);

  gset<string> gs_;
  load(crdt_s, gs_);
  cout << gs_ << endl;
  cout << "----" << endl;
  
  delta::crdt crdt_c;

  gset<cat> gc;
  cat c0(0, "zero");
  cat c1(1, "one");
  gc.add(c0);
  gc.add(c1);
  dump(crdt_c, gc);
  show_crdt(crdt_c);

  gset<cat> gc_;
  load(crdt_c, gc_);
  cout << gc_ << endl;
  cout << "----" << endl;
}

int main(int argc, char * argv[])
{
  test_dump_and_load();
}
