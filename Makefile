CC=g++
SOKCETOBJ=src/baseio.o src/socket.o src/tcpsocket.o src/epoll.o\
http/http.o http/util.o http/httpcoon.o http/httpengine.o http/httpfile.o\
Tpool/cond.o Tpool/locker.o Tpool/sem.o Tpool/threadpool.o
TARGETDIR=./build
CXXFLAGS = -fPIC -I./include -std=c++11 -g
LDFLAGS=-L${TARGETDIR} -lsocket -pthread  -Wl,--rpath=.
all:init libsocket test

init:
	$(shell if [ ! -d $(TARGETDIR) ]; then mkdir -p $(TARGETDIR); fi)

libsocket:libsocket.a libsocket.so

libsocket.a:${SOKCETOBJ}
	ar -cr ${TARGETDIR}/$@ $^

libsocket.so:${SOKCETOBJ}
	${CC} -shared -fPIC -o ${TARGETDIR}/$@  $^

install:libsocket.so
	cp ${TARGETDIR}/libsocket.so /usr/lib/x86_64-linux-gnu/libsocket.so
	cp ${TARGETDIR}/libsocket.a /usr/lib/x86_64-linux-gnu/libsocket.a
	cp -rf ./include /usr/include/libsocket/

test:server client httpserver

server:example/server.o
	${CC}  $^ -o ${TARGETDIR}/$@ ${LDFLAGS}

client:example/client.o
	${CC}  $^ -o ${TARGETDIR}/$@ ${LDFLAGS}

httpserver:example/httpserver.o
	${CC} $^ -o ${TARGETDIR}/$@ ${LDFLAGS}

.PHONY : clean clean-test
clean:
	rm -rf ${TARGETDIR}
	rm src/*.o http/*.o Tpool/*.o example/*.o