CC = g++
DEBUG = -g -v
FLAGS = -std=c++11
PB_FLAGS = `pkg-config --cflags protobuf`
PB_LIBS = `pkg-config --libs protobuf`

all: delta-tests proto-tests replicas chat

delta-tests: message-proto delta-tests.cc
	$(CC) $(FLAGS) delta-tests.cc -o delta-tests $(PB_LIBS)
	./delta-tests

proto-tests: message-proto proto-tests.cc
	$(CC) $(FLAGS) proto-tests.cc message.pb.o -o proto-tests $(PB_LIBS)
	./proto-tests

chat: message-proto unix-sockets/client.cc unix-sockets/server.cc
	$(CC) $(FLAGS) unix-sockets/client.cc message.pb.o -o client $(PB_LIBS)
	$(CC) $(FLAGS) unix-sockets/server.cc message.pb.o -o server $(PB_LIBS)

replicas: message-proto replicas/replica.cc
	$(CC) $(FLAGS) replicas/replica.cc message.pb.o -o replica $(PB_LIBS)

message-proto: message.proto
	protoc --cpp_out=. message.proto
	$(CC) -c message.pb.cc $(PB_FLAGS)

clean:
	rm -f message.pb.*

call:
	rm -f delta-tests proto-tests client server replica message.pb.*
