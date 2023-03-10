#include <baseio.h>
using namespace my;

BaseIO::BaseIO(int fd):fd(fd),block(true){}
BaseIO::~BaseIO(){}
int BaseIO::FD() const{
    return fd;
}
void BaseIO::FD(int _fd){
    fd = _fd;
}