CC = g++
DEBUG = -g -v
FLAGS = -std=c++11
PB_FLAGS = `pkg-config --cflags protobuf`
PB_LIBS = `pkg-config --libs protobuf`

all: delta-tests proto-tests replicas

delta-tests: crdt-proto delta-tests.cc
	$(CC) $(FLAGS) delta-tests.cc -o delta-tests
	./delta-tests

proto-tests: crdt-proto proto-tests.cc
	$(CC) $(FLAGS) proto-tests.cc crdt.pb.o -o proto-tests $(PB_LIBS)
	./proto-tests

chat: crdt-proto unix-sockets/client.cc unix-sockets/server.cc
	$(CC) $(FLAGS) unix-sockets/client.cc crdt.pb.o -o client $(PB_LIBS)
	$(CC) $(FLAGS) unix-sockets/server.cc crdt.pb.o -o server $(PB_LIBS)
	echo "use with care: empty messages bug is on!!!"

replicas: crdt-proto two-replicas/replica.cc
	$(CC) $(FLAGS) two-replicas/replica.cc crdt.pb.o -o replica $(PB_LIBS)

crdt-proto: crdt.proto
	protoc --cpp_out=. crdt.proto
	$(CC) -c crdt.pb.cc $(PB_FLAGS)

clean:
	rm -f crdt.pb.*

call:
	rm -f delta-tests proto-tests client server replica crdt.pb.*
