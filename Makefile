CC = g++
DEBUG = -g -v
FLAGS = -std=c++11

all: delta-tests proto-tests replicas

delta-tests: crdt-proto delta-crdts.cc delta-tests.cc
	$(CC) $(FLAGS) delta-tests.cc -o delta-tests
	./delta-tests

proto-tests: crdt-proto delta-crdts.cc proto-tests.cc
	$(CC) $(FLAGS) proto-tests.cc crdt.pb.o -o proto-tests `pkg-config --libs protobuf`
	./proto-tests

replicas: crdt-proto delta-crdts.cc two-replicas/replica.cc
	$(CC) $(FLAGS) two-replicas/replica.cc crdt.pb.o -o replica `pkg-config --libs protobuf`

crdt-proto: crdt.proto
	protoc --cpp_out=. crdt.proto
	$(CC) -c crdt.pb.cc `pkg-config --cflags protobuf`

clean:
	rm -f delta-tests proto-tests replica crdt.pb.*
