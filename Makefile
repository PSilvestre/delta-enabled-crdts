CC = g++
DEBUG = -g -v
FLAGS = -std=c++11

all: delta-tests proto-tests

delta-tests: delta-crdts.cc delta-tests.cc
	$(CC) $(FLAGS) delta-crdts.cc delta-tests.cc -o delta-tests

proto-tests: crdt-proto proto-tests.cc
	$(CC) $(FLAGS) delta-crdts.cc proto-tests.cc crdt.pb.o -o proto-tests `pkg-config --libs protobuf`

crdt-proto: crdt.proto
	protoc --cpp_out=. crdt.proto
	$(CC) -c crdt.pb.cc `pkg-config --cflags protobuf`

clean:
	rm -f delta-tests proto-tests crdt.pb.*
