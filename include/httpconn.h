#ifndef HTTP_CONN_H__
#define HTTP_CONN_H__
#include <socket.h>
#include <unordered_map>
#include <string>
#include <http.h>
#include <httpengine.h>
#include <resptype.h>
#include <httpfile.h>
#include <vector>
namespace my{
typedef unordered_map<std::string,std::string> HTTPHeader_t;
typedef unordered_map<std::string,std::string> HTTPCookie_t;
typedef unordered_map<std::string,std::string> HTTPQuery_t;
typedef unordered_map<std::string,std::string> HTTPParam_t;
typedef byte_t Body;

struct Request{
    string URL;
    HttpMethod_t method;
    HTTPQuery_t  query;
    HTTPParam_t  param;
    string http_version;
    bool keepalive;
    HTTPHeader_t headers;
    ssize_t ContentLength;
    ssize_t bodyLength;
    string ContentType;
    HTTPCookie_t cookies;
    Body*   bodys;
    std::vector<MultipartFile*> FromFile(const std::string & name);
};

struct Response{
    string http_version;
    HttpStatus_t status;
    HTTPHeader_t headers;
    ssize_t ContentLength;
    string ContentType;
    HTTPCookie_t cookies;
    Body*   bodys;
};
class HttpEngine;
class HttpConn:public TCPSocket{
    HttpEngine* engine;
    Request  _request;  
    Response _response;
    char* readbuf;
    ssize_t max_read_size;
    ssize_t read_idx;        //缓冲区读入的数据大小
    ssize_t  line_idx;       //已经从缓冲区读过的<也就是说 下次读要从这里开始
    ssize_t  line_start_idx; //本行开始的行
    vector<byte_t> writebuf;
    string router;

    HTTP_CHECK_STATE_t check_status;
    HTTP_LINE_STATUS_t line_status;
    HTTP_RESULT_t http_result;
    
    string static_filename;
    int file_fd;
    bool   file_req;
    byte_t* address;
    size_t  file_size;
    
    MultipartFiles m_files;
private:
    HttpConn(const HttpConn&) = delete;
    HttpConn& operator=(const HttpConn&)=delete;
public:
    HttpConn(TCPSocket socket,HttpEngine* engine);
    ~HttpConn();
    void InitConn();
public:
    string URL();
    void URL(string);
    HttpMethod_t Method();
    void Method(HttpMethod_t);
    string Router();
    void Router(string router);
public:
    bool KeepAlive();
    bool FileReq();
    void FileReq(bool isFileReq);
    size_t StaticFileSize();
    void   StaticFileSize(size_t filesize = 0);
    void StaticFileFD(int fd);
    int StaticFileFD();
    byte_t* Address();
    void Address(byte_t*);
    void StaticFileName(string filename);
    std::string StaticFileName(void);
    std::vector<MultipartFile*> PostFrom(const std::string &key);
    int SaveUploadFile(const MultipartFile* file,const std::string path);
public:
    int Write();
    // 解析出来的路由
    int WriteToJson(HttpStatus_t code,string json);
    int WriteToJson(HttpStatus_t code,JsonType* obj);
    int WriteToXML(HttpStatus code,string json);
    int WriteToText(HttpStatus_t code,string text);
    int WriteToHTML(HttpStatus code,string json);
    int WriteToFile();
    HTTP_RESULT_t ReadToRequest();
    string GetHeader(string key);
    void SetHeader(string key,string value);

    string GetCookie(string key);
    void SetCookie(string key,string value);

    string GetQuery(string key);
    string GetParam(string key); // key value

    int BindBody(void* body,string type);
    void BindJsonBody(JsonType* ret);
    string GetStaticFileName();
private:
    HTTP_RESULT_t ReadDataToBuff();
    HTTP_RESULT_t ParseRequest();
public:
    HTTP_RESULT_t ParseParam();
    void ReSetRequest();
private:
    string GetLine();
    HTTP_LINE_STATUS_t ParseLine();
    HTTP_RESULT_t ParseRequestLine(string text);
    HTTP_RESULT_t ParseHeaderLine(string text);
    HTTP_RESULT_t ParseContentLine(const char* text);
    void ParseCookie(string value);
    void RequestToWriteBuff();
}; 
};
#endif