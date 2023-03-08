#include<cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <cstring>
#include"socket.h"
#include "tcpsocket.h"
#include "epool.h"
using namespace my;
#define PORT "4212"
#define SERVER "0.0.0.0"
#define BUFSIZE 1024
struct serverarg{
    TCPServer server;
    Epool ep;
};

class ClientFunc:public HandleEventOBJ{
    Epool* ep;
    my::TCPSocket socket;
public:
    ClientFunc(Epool* ep,my::TCPSocket socket):ep(ep),socket(socket){}
    void handle(epoll_event event){
        if(event.events & EPOLLIN){
            char buf[BUFSIZ];
            int ret = socket.ReadString(buf,BUFSIZE);
            if(ret < 0){
                perror("ReadString");
                return ;
            }
            if(!ret){
                ep->DelEvent(&socket);
                socket.ShutDown(SHUT_RDWR);
                socket.Close();
                return ;
            }
            socket.WriteString("hello world\n",strlen("hello world\n"));
            printf("%d:%s\n",socket.FD(),buf);
            ep->ModifyEvent(&socket,EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT);
            return ;
        }
        if(event.events & (EPOLLRDHUP)){
            printf("有连接断开\n");
            ep->DelEvent(&socket);
            socket.ShutDown(SHUT_RDWR);
            socket.Close();
        }
        return ;
    }
};

class ServerFunc:public HandleEventOBJ{
    Epool* ep;
    my::TCPServer* server;
    public:
        ServerFunc(Epool* ep,my::TCPServer* server):ep(ep),server(server){}
    public:
        void handle(epoll_event event){
            struct sockaddr_in raddr;
            socklen_t len = sizeof(raddr);
            auto socket = server->Accept();
            printf("有链接加入：%d\n",socket.FD());
            ep->AddEvenet(&socket,EPOLLIN|EPOLLRDHUP|EPOLLET|EPOLLONESHOT,shared_ptr<HandleEventOBJ>(new ClientFunc(ep,socket)));
        }
};

int main(){
    my::TCPServer server(SERVER,atoi(PORT));
    if(server.Listen() < 0){
        perror("Listen:");
        exit(1);
    }
    my::Epool ep;
    ep.EventInit();
    auto func = shared_ptr<HandleEventOBJ>(new ServerFunc(&ep,&server));
    ep.AddEvenet(&server,EPOLLIN|EPOLLET,func);
    ep.LoopWait();
}