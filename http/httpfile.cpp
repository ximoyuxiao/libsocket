#include<httpfile.h>
#include<cstring>
using namespace my;

MYLIBSOCKET_DEFINE_CLASS(MultipartFile,std::string,name,std::string,filename,size_t,size,std::string,ContentType,DistpositionType,dispostionType)

MultipartFile::MultipartFile():name(""),filename(""),size(0),content(nullptr),ContentType(""),dispostionType("form-data"){}

MultipartFile::~MultipartFile(){
    if(content){
        delete content;
    }
}

MultipartFile::MultipartFile(const MultipartFile& other):name(other.name),filename(other.filename),size(other.size),\
                            ContentType(other.ContentType),dispostionType(other.dispostionType){
    if(other.content){
        content = new byte_t[size]();
        memcpy(content,other.content,size);
    }
}

MultipartFile& MultipartFile::operator=(const MultipartFile& other){
    name = other.name;
    filename = other.filename;
    size = other.size;
    ContentType = other.ContentType;
    dispostionType = other.dispostionType;
    if(content){
        delete []content;
    }
    if(other.content){
        content = new byte_t[size]();
        memcpy(content,other.content,size);
    }
    return *this;
}

void MultipartFile::SetContent(const byte_t* content,const size_t& size){
    this->content = new byte_t[size];
    memcpy(this->content,content,size);
    return ;
}
byte_t* MultipartFile::GetContent() const{
    return content;
}