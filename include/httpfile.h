#ifndef HTTP_FILE_H__
#define HTTP_FILE_H__
#include<string>
#include<socktype.h>
#include<macrogen.h>
#include<unordered_map>
#include<vector>
namespace my{
typedef std::string DistpositionType; 
typedef byte_t* byteptr_t;
//128M
const size_t MAXFILESIZE = 128<<20;
MYLIBSOCKET_DECL_CLASS_HEAD(MultipartFile,std::string,name,std::string,filename,\
size_t,size,std::string,ContentType,DistpositionType,dispostionType)
private: 
    byte_t* content;
public:
    void SetContent(const byte_t* content,const size_t& size);
    byte_t* GetContent() const;
public:
    MultipartFile();
    MultipartFile(const MultipartFile& other);
    MultipartFile& operator=(const MultipartFile& other);
    ~MultipartFile();
public:
MYLIBSOCKET_DECL_CLASS_END

typedef std::unordered_map<std::string,std::vector<MultipartFile*>> MultipartFiles;
};

#endif