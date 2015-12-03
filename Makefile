CC = g++
DEBUG = -g -v
FLAGS = -std=c++11

all: delta-tests

delta-tests: crdt-proto delta-crdts.cc delta-tests.cc
	$(CC) $(FLAGS) delta-crdts.cc delta-tests.cc crdt.pb.o -o delta-tests `pkg-config --libs protobuf`

crdt-proto: crdt.proto
	protoc --cpp_out=. crdt.proto
	$(CC) -c crdt.pb.cc `pkg-config --cflags protobuf`

clean:
	rm delta-tests crdt.pb.*
