CC=g++
SOKCETOBJ=src/baseio.o\
src/socket.o\
src/tcpsocket.o\
src/epool.o
CXXFLAGS = -fPIC -I./include -std=c++11 -g
LDFLAGS=-L. -lsocket -pthread  -Wl,--rpath=./
all:libsocket.a libsocket.so

libsocket.a:${SOKCETOBJ}
	ar -cr $@ $^

libsocket.so:${SOKCETOBJ}
	${CC} -shared -fPIC -o $@  $^

install:libsocket.so
	cp libsocket.so /usr/lib/libsocket.so

test:server client httpserver

server:example/server.o
	${CC}  $^ -o $@ ${LDFLAGS}

client:example/client.o
	${CC}  $^ -o $@ ${LDFLAGS}

httpserver:http/httpserver.o
	${CC} $^ -o $@ ${LDFLAGS}

.PHONY : clean clean-test
clean:
	rm src/*.o libsocket.*
clean-test:
	rm example/*.o server client httpserver