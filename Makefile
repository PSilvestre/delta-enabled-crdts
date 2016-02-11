CC = g++
DEBUG = -g -v
FLAGS = -std=c++11
PB_FLAGS = `pkg-config --cflags protobuf`
PB_LIBS = `pkg-config --libs protobuf`
COMPILED_FILES = message.pb.o csocket.o csocketserver.o helpers.o

all: delta-tests proto-tests chat replicas

delta-tests: message-proto delta-tests.cc
	$(CC) $(FLAGS) delta-tests.cc -o delta-tests $(PB_LIBS)
	./delta-tests

proto-tests: message-proto proto-tests.cc
	$(CC) $(FLAGS) proto-tests.cc message.pb.o -o proto-tests $(PB_LIBS)
	./proto-tests

chat: csock chat_server chat_client

chat_server:
	$(CC) $(FLAGS) chat/server.cc $(COMPILED_FILES) -o server $(PB_LIBS)

chat_client:
	$(CC) $(FLAGS) chat/client.cc $(COMPILED_FILES) -o client $(PB_LIBS)

replicas: csock replica pnp

replica:
	$(CC) $(FLAGS) replicas/replica.cc $(COMPILED_FILES) -o replica $(PB_LIBS)

pnp:
	$(CC) $(FLAGS) replicas/pnp_replica.cc $(COMPILED_FILES) -o pnp $(PB_LIBS)

csock: message-proto help csock/csocket.cc csock/csocketserver.cc
	$(CC) $(FLAGS) -c csock/csocket.cc $(PB_LIBS)
	$(CC) $(FLAGS) -c csock/csocketserver.cc $(PB_LIBS)

help: helpers.cc
	$(CC) $(FLAGS) -c helpers.cc

message-proto: message.proto
	protoc --cpp_out=. message.proto
	$(CC) -c message.pb.cc $(PB_FLAGS)

clean:
	rm -f message.pb.* *.o

cleanall:
	rm -f delta-tests proto-tests client server replica pnp message.pb.* *.o

