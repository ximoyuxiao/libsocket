#ifndef UTIL_H__
#define UTIL_H__
#include<vector>
#include<string>
#include<socktype.h>
namespace my{
    int SplitString(const std::string str,const std::string tag,std::vector<std::string>& splite);
    size_t FindBinary(const byte_t* src_data,const size_t offset,const size_t srcSize,const byte_t* des_data,size_t cmpsize);
    std::string URLdecode(std::string url);
    std::string URLencode(std::string url);
};
#endif