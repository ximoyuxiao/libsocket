#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "socket.h"
static uint64_t htonll(uint64_t val){
    return (((uint64_t) htonl(val)) << 32) + htonl(val >> 32);
}
 
static uint64_t ntohll(uint64_t val){
    return (((uint64_t) ntohl(val)) << 32) + ntohl(val >> 32);
}

my::BaseSocket::BaseSocket(int domain,int type,int protocol)
:_domain(domain),_type(type),_protocol(protocol),BaseIO(){}

my::BaseSocket::~BaseSocket(){}

void my::BaseSocket::SetNonBlock(){
    if(!block) return ;
    block = false;
    int flags = fcntl(fd,F_GETFL,0);
    flags |=O_NONBLOCK;
    fcntl(fd,F_SETFL,flags);
    return ;
}

int my::BaseSocket::ReadBytes(byte_t *buff,size_t size){
    int len = 0;
    if(!block){
        while(len < size){
            int ret = read(fd,buff + len,size - len);
            if(ret < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    return len;
                }
                if(errno == EINTR){
                    continue;
                }
                return ret;
            }
            if(!ret) return len;
            len += ret;
        }
        return len;
    }
    return read(fd,buff,size);
}

int my::BaseSocket::WriteBytes(const byte_t* buff,size_t size){
    return write(fd,buff,size);
}

int my::BaseSocket::ReadString(char* buf,size_t max_size){
    return ReadBytes((byte_t*)buf,max_size);
}

int my::BaseSocket::WriteString(const char* buf,size_t max_size){
    return write(fd,buf,max_size);
}

int32_t my::BaseSocket::ReadInt32(){
    int32_t ret;
    read(fd,&ret,sizeof(ret));
    return ntohl(ret);
}
int my::BaseSocket::WriteInt32(int32_t buf){
    int w_data =htonl(buf);
    return write(fd,&w_data,sizeof(w_data));
}

int64_t my::BaseSocket::ReadInt64(){
    int64_t ret;
    read(fd,&ret,sizeof(ret));
    return ntohll(ret);
}
int my::BaseSocket::WriteInt64(int64_t buf){
    int w_data =htonll(buf);
    return write(fd,&w_data,sizeof(w_data));
}

int my::BaseSocket::ReadObj(void* buff,size_t size){
    return ReadBytes((byte_t*)buff,size);
}
int my::BaseSocket::WriteObj(void* buff,size_t size){
    return WriteBytes((byte_t*)buff,size);
}

int my::BaseSocket::Flush(){}

int my::BaseSocket::Close(){
    if(fd == -1){
        return 0;
    }
    int ret = close(fd);
    if(!ret)    fd = -1;
    return ret;
}

int my::BaseSocket::SetOption(int _level,int _optname,const void* _optval,socklen_t _optlen ){
    if(fd == -1){
        return EHOSTDOWN;
    }
    setsockopt(fd,_level,_optname,_optval,_optlen);
    return 0;
}

int my::BaseSocket::Socket(){
    if(fd == -1){
        fd =  socket(_domain,_type,_protocol);
    }
    return fd;
}


my::UDPSocket::UDPSocket(int port):my::BaseSocket(AF_INET,SOCK_DGRAM,0),_port(port){}

my::UDPSocket::~UDPSocket(){}

my::address_t my::UDPSocket::Address(){return _address;}

void my::UDPSocket::Address(my::address_t addr){_address = addr;}

void my::UDPSocket::Port(int port){_port = port;}

int  my::UDPSocket::Port(){return _port;}


my::TCPSocket::TCPSocket(my::address_t address,int port):my::BaseSocket(AF_INET,SOCK_STREAM,0),_port(port){
    _address = nullptr;
    if(address != nullptr){
        _address = (address_t)malloc(strlen(address) + 1);
        memcpy((void*)_address,address,strlen(address) + 1);
    }
}

my::TCPSocket::~TCPSocket(){
    if(_address != nullptr){
        free((void*)_address);
    }
    _address = nullptr;
}
my::TCPSocket::TCPSocket(const TCPSocket& other):BaseSocket(other._domain,other._type,other._protocol){
    this->fd = other.fd;
    this->_port = other._port;
    this->block = other.block;
    _address = nullptr;
    if(other._address != nullptr){
        _address = (address_t)malloc(strlen(other._address) + 1);
        memcpy((void*)_address,other._address,strlen(other._address) + 1);
    }
}
my::TCPSocket& my::TCPSocket::operator=(const TCPSocket &other){
    this->_domain = other._domain;
    this->_type = other._type;
    this->_protocol = other._protocol;
    this->fd = other.fd;
    this->_port = other._port;
    this->block = other.block;
    _address = nullptr;
    if(other._address != nullptr){
        _address = (address_t)malloc(strlen(other._address) + 1);
        memcpy((void*)_address,other._address,strlen(other._address) + 1);
    }
    return *this;
}
my::address_t my::TCPSocket::Address() const {return _address;}

void my::TCPSocket::Address(my::address_t addr){_address = addr;}

void my::TCPSocket::Port(int port){_port = port;}

int  my::TCPSocket::Port(){return _port;}

int my::TCPSocket::ShutDown(int how){
    int _socketfd = fd;
    if(_socketfd == -1){
        int ret = shutdown(_socketfd,how);
        if(!ret) return ret;
        fd = -1;
    }
    return 0;
}