#ifndef _BASE_IO_H__
#define _BASE_IO_H__
#include<iostream>
#include<vector>
#include<sys/time.h>
#include<socktype.h>
namespace my{
class BaseIO{
public:
    BaseIO(int fd = -1);
    virtual ~BaseIO();
public:
    int FD() const;
    void FD(int fd);
public:
    virtual int ReadBytes(byte_t *buff,size_t size) = 0;
    virtual int WriteBytes(const byte_t* buff,size_t size) = 0;

    virtual int ReadString(char* buf,size_t max_size) = 0;
    virtual int WriteString(const char* buf,size_t max_size) = 0;
    
    virtual int32_t ReadInt32() = 0;
    virtual int WriteInt32(int32_t buf) = 0;

    virtual int64_t ReadInt64() =0;
    virtual int WriteInt64(int64_t buf) = 0;
    
    virtual int ReadObj(void* buff,size_t size) = 0;
    virtual int WriteObj(void* buff,size_t size) = 0;

    virtual int Flush() = 0;
    virtual int Close() = 0;
    virtual void SetNonBlock()  = 0;
protected:
    int fd;
    bool block;
};
}

#endif