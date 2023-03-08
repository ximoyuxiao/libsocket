#include "tcpsocket.h"

my::TCPServer::TCPServer(address_t addr,int port):TCPSocket(addr,port)
{
}

int my::TCPServer::Listen(){
    fd  = Socket();
    if(fd  <= 0){
        return -1;
    }
    int flag = 1;
    SetOption(SOL_SOCKET,SO_REUSEADDR,&flag,sizeof(flag));
    sockaddr_in laddr;
    laddr.sin_family = AF_INET; 
    laddr.sin_port = htons(Port());
    inet_pton(AF_INET,Address(),&laddr.sin_addr);
    bind(fd,(sockaddr*)&laddr,sizeof(laddr));
    return listen(fd,200);
}

my::TCPSocket my::TCPServer::Accept(){
    struct sockaddr_in _addr; 
    socklen_t _addr_len = sizeof(_addr);
    int sock;
    while(true)
    {
        sock = accept(fd,(sockaddr*)&_addr,&_addr_len);
        if(sock < 0){
            if(errno == EAGAIN || errno == EINTR){
                continue;
            }
            perror("accept:");
            return TCPSocket();
        }
        break;
    }
    char buff[20] ={0};
    inet_ntop(AF_INET,&_addr.sin_addr,buff,sizeof(_addr.sin_addr));
    TCPSocket socket(buff,_addr.sin_port);
    socket.FD(sock);
    return socket;
}

my::TCPServer::~TCPServer()
{
    
}

my::TCPClient::TCPClient(address_t addr,int port):TCPSocket(addr,port){}
my::TCPClient::~TCPClient(){}

int my::TCPClient::Connection(){
    fd = Socket();
    sockaddr_in _addr;
    _addr.sin_family = AF_INET;
    _addr.sin_port  = htons(Port());
    inet_pton(AF_INET,Address(),&_addr.sin_addr);
    return connect(fd,(sockaddr*)(&_addr),sizeof(_addr));
}