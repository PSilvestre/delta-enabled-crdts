#include "delta-crdts.cc"
#include <google/protobuf/text_format.h>
#include "cat.h"
#include "message.pb.h"

void show_message_proto(const proto::message& message)
{
  cout << "byte size: " << message.ByteSize() << endl;
  string message_str;
  google::protobuf::TextFormat::PrintToString(message, &message_str);
  cout << message_str;
}

void test_gset()
{
  proto::message message_i;
  gset<int> gi;
  gi.add(2);
  gi.add(4);
  message_i << gi;
  show_message_proto(message_i);

  gset<int> gi_;
  message_i >> gi_;
  cout << gi_ << endl;
  cout << "----" << endl;

  proto::message message_s;
  gset<string> gs;
  gs.add("abc");
  gs.add("xyz");
  message_s << gs;
  show_message_proto(message_s);

  gset<string> gs_;
  message_s >> gs_;
  cout << gs_ << endl;
  cout << "----" << endl;

  proto::message message_c;
  gset<cat> gc;
  cat c0(0, "zero");
  cat c1(1, "one");
  gc.add(c0);
  gc.add(c1);
  message_c << gc;
  show_message_proto(message_c);

  gset<cat> gc_;
  message_c >> gc_;
  cout << gc_ << endl;
  cout << "----" << endl;
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
  show_message_proto(message);

  twopset<string> ts_;
  message >> ts_;
  cout << ts_ << endl;
  cout << "----" << endl;
}

void test_gcounter()
{
  proto::message message;
  gcounter<> o1("idx");
  gcounter<> o2("idy");
  o1.inc();
  o1.join(o2.inc(4));
  message << o1;
  show_message_proto(message);

  gcounter<> o1_;
  message >> o1_;
  cout << o1_ << endl;
  cout << "----" << endl;
}

int main(int argc, char * argv[])
{
  test_gset();
  test_twopset();
  test_gcounter();
}
