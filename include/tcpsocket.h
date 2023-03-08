#ifndef TCP_SERVER_H__
#define TCP_SERVER_H__
#include "socket.h"
namespace my{
class TCPServer:public TCPSocket
{
private:
    
public:
    TCPServer(address_t addr,int port);
    virtual ~TCPServer();
public:
    int Listen();
    TCPSocket Accept();
};

class TCPClient:public TCPSocket
{
public:
    TCPClient(address_t addr,int port);
    virtual ~TCPClient();
public:
    int Connection();
};
};
#endif