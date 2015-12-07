protoc --cpp_out=. crdt.proto
g++ -std=c++11 -c crdt.pb.cc `pkg-config --cflags protobuf`
g++ -std=c++11 server.cc crdt.pb.o -o server `pkg-config --libs protobuf`
g++ -std=c++11 client.cc crdt.pb.o -o client `pkg-config --libs protobuf`
./server 2222 &
sleep 1
./client localhost 2222
rm crdt.pb.* client server
