CC=g++
SOKCETOBJ=src/baseio.o\
src/socket.o\
src/tcpsocket.o\
src/epoll.o\
http/http.o\
http/util.o\
http/httpcoon.o\
http/httpengine.o\
Tpool/cond.o\
Tpool/locker.o\
Tpool/sem.o\
Tpool/threadpool.o
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

httpserver:example/httpserver.o
	${CC} $^ -o $@ ${LDFLAGS}

.PHONY : clean clean-test
clean:
	rm src/*.o libsocket.* http/*.o Tpool/*.o
clean-test:
	rm example/*.o server client httpserver