#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <util.h>
#include <cstdio>
#include <socktype.h>
#include <cstring>
#include <limits>
namespace my{

std::string toHex(unsigned char c) {
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    return ss.str();
}

int SplitString(const std::string str,const std::string tag,std::vector<std::string>& splite){
        splite.clear(); // 清空 splite 向量

        if (str.empty()) {
            return 0;
        }

        size_t pos = 0;
        size_t len = str.length();
        size_t tagLen = tag.length();
        if(str.substr(0,tagLen) == tag){
            pos = tagLen;
        }
        while (pos < len) {
            size_t found = str.find(tag, pos);

            if (found == std::string::npos) {
                // 如果没找到分隔符，说明当前是最后一个子串
                splite.push_back(str.substr(pos, len - pos));
                break;
            }

            // 如果找到了分隔符，说明当前子串是从 pos 到 found 的子串
            splite.push_back(str.substr(pos, found - pos));

            // 跳过分隔符
            pos = found + tagLen;
        }

        return splite.size();
    }

size_t FindBinary(const byte_t* src_data, const size_t offset, const size_t srcSize, const byte_t* des_data, size_t cmpsize) {
    if (src_data == nullptr || des_data == nullptr || cmpsize > srcSize - offset) {
        return std::numeric_limits<size_t>::max();// 返回错误标识
    }

    const byte_t* start = src_data + offset;
    const byte_t* end = src_data + srcSize - cmpsize + 1;

    for (const byte_t* p = start; p != end; ++p) {
        if (std::memcmp(p, des_data, cmpsize) == 0) {
            return p - src_data; // 找到了，返回下标
        }
    }

    return std::numeric_limits<size_t>::max(); // 没有找到，返回错误标识
}

std::string URLdecode(std::string url){
    std::string result;
    char c;
    int i, len = url.length();
    for (i = 0; i < len; i++) {
        switch (url[i]) {
            case '+':
                result += ' ';
                break;
            case '%':
                if (i + 2 < len && isxdigit(url[i + 1]) && isxdigit(url[i + 2])) {
                    sscanf(url.substr(i + 1, 2).c_str(), "%x", (unsigned int*)&c);
                    result += c;
                    i += 2;
                }
                else {
                    result += '%';
                }
                break;
            default:
                result += url[i];
                break;
        }
    }
    return result;
}

std::string URLencode(std::string url){
    std::string result;
    int i, len = url.length();
    for (i = 0; i < len; i++) {
        switch (url[i]) {
            case ' ':
                result += '+';
                break;
            case '-':
            case '_':
            case '.':
            case '!':
            case '~':
            case '*':
            case '\'':
            case '(':
            case ')':
                result += url[i];
                break;
            default:
                if (isalnum(url[i])) {
                    result += url[i];
                }
                else {
                    result += '%';
                    for (auto c : toHex(url[i]))
                        result += c;
                }
                break;
        }
    }
    return result;
}
};


