#ifndef UTIL_H__
#define UTIL_H__
#include<vector>
#include<string>
namespace my{
    int SplitString(const std::string str,const std::string tag,std::vector<std::string>& splite);
    std::string URLdecode(std::string url);
    std::string URLencode(std::string url);
};
#endif