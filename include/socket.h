#ifndef MY_SOCKET_H__
#define MY_SOCKET_H__
#include<iostream>
#include <arpa/inet.h>
#include "socktype.h"
#include "baseio.h"
using namespace std;
namespace my{

class BaseSocket:public BaseIO{
    protected:
        int _domain;
        int _type;
        int _protocol;
    public:
        BaseSocket(int domain,int type,int protocol);
        virtual ~BaseSocket();
    public:
        int ReadBytes(byte_t *buff,size_t size) ;
        int WriteBytes(const byte_t* buff,size_t size) ;

        int ReadString(char* buf,size_t max_size) ;
        int WriteString(const char* buf,size_t max_size) ;
    
        int32_t ReadInt32() ;
        int WriteInt32(int32_t buf) ;

        int64_t ReadInt64() ;
        int WriteInt64(int64_t buf) ;
    
        int ReadObj(void* buff,size_t size) ;
        int WriteObj(void* buff,size_t size) ;

        int Flush() ;
        int Close() ;
    public:
        int Socket();        
        int SetOption(int _level,int _optname,const void* _optval,socklen_t _optlen);
        void SetNonBlock();
};

class UDPSocket:public BaseSocket{
        int _port;
        address_t  _address;
    public:
        UDPSocket(int port);
        virtual ~UDPSocket();
    public:
        int Port();
        void Port(int _port);
        address_t Address();
        void Address(address_t addr);
};

class TCPSocket:public BaseSocket{
    private:
        address_t _address;
        int _port;
    public:
        TCPSocket(address_t address ="",int port = 0);
        TCPSocket(const TCPSocket& other);
        TCPSocket& operator=(const TCPSocket &other);
        virtual ~TCPSocket();
    public:
        int ShutDown(int how);
    public:
        int Port(); 
        void Port(int port);
        address_t Address() const; 
        void Address(address_t addr); 
};
};

#endif