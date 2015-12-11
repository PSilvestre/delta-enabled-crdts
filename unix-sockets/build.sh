protoc --cpp_out=. crdt.proto
g++ -std=c++11 -c crdt.pb.cc `pkg-config --cflags protobuf`
g++ -std=c++11 server.cc crdt.pb.o -o server `pkg-config --libs protobuf`
g++ -std=c++11 client.cc crdt.pb.o -o client `pkg-config --libs protobuf`
rm crdt.pb.*
echo "use carefully: empty messages bug is on"
