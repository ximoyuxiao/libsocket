#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <util.h>
#include <cstring>
#include <unistd.h>

#include <httpconn.h>
#include <nlohmann/json.hpp>

using namespace my;
void PrintRequest(Request req);
vector<size_t>  GetBoundarys(byte_t* bodys,size_t length,std::string boundary){
    vector<size_t> boundarys;
    size_t ReadBodySize = 0;
    auto startBoundary = "--" + boundary;
    auto endBoundary = startBoundary + "--";
    // 2.获得最后的终点 
    auto lastBoundaryIdx = FindBinary(bodys,ReadBodySize,length,endBoundary.data(),endBoundary.size());
    if(lastBoundaryIdx == std::numeric_limits<size_t>::max()) return boundarys;
    while(ReadBodySize < length){
        auto read = FindBinary(bodys,ReadBodySize,length,startBoundary.data(),startBoundary.size());
        // 需要检查正确性
        boundarys.push_back(read);
        if(read == lastBoundaryIdx){
            break;
        }
        ReadBodySize =  read + startBoundary.size();
    }
    return boundarys;
}
size_t GetLine(byte_t* body,int len){
    int s = 0;
    while(s < len){
        if(body[s] == '\r' ){
            if(s + 1 == len){
                return 0;
            }
            if(body[s + 1] == '\n'){
                body[s++] = '\0';
                body[s++] = '\0';
            }
            break;
        }
        s++;
    }
    return s;
}
MultipartFile* ReadAndCreateMultipartFile(byte_t* body,int s,int e){
    MultipartFile* ret = new MultipartFile();
    size_t ContentDisIdx,ContentTypeIdx;
    body = body + s;
    int len = e - s;
    auto idx = GetLine(body,len);
    body += idx;
    len -= idx;
    idx = GetLine(body,len);
    vector<string> splite;
    SplitString(body,";",splite);
    // Content-Disposition: 
    ret->SetdispostionType(splite[0].substr(21,splite[0].size() - 21));
    // name=""
    ret->Setname(splite[1].substr(7,splite[1].size() - 8));
    // filename=""
    ret->Setfilename(splite[2].substr(11,splite[2].size() - 12));

    body += idx;
    len -= idx;
    idx = GetLine(body,len);
    string data = string(body);
    ret->SetContentType(data.substr(13,data.size() - 13));
    
    body += idx;
    len -=idx;
    ret->Setsize(len - 4);
    ret->SetContent(body + 2,len - 4);
    return ret;
}
std::vector<MultipartFile*> Request::FromFile(const std::string & name){
    std::vector<MultipartFile*> ret;
    if(ContentType.find("multipart/form-data",0) != ContentType.npos){
        std::string prev = "multipart/form-data; boundary=";
        auto ContentTypeLen = ContentType.size();
        auto boundary = ContentType.substr(prev.size(),ContentTypeLen - prev.size());
        // 1.取出所有的 Data分割数据起点 --boundary
        vector<size_t> boundarys = GetBoundarys(bodys,ContentLength,boundary);
        
        for(int i = 0;i<boundarys.size() - 1;i++){
           MultipartFile* file =  ReadAndCreateMultipartFile(bodys,boundarys[i],boundarys[i+1]);
           ret.push_back(file);
        }
        return ret;
    }

    return ret;
}

HttpConn::HttpConn(TCPSocket socket,HttpEngine* engine):TCPSocket(socket),engine(engine),max_read_size(1024){
}

HttpConn::~HttpConn(){
    ReSetRequest();
    if(readbuf){
        delete readbuf;
    }
}

void HttpConn::InitConn(){
    readbuf = new char[max_read_size];
    read_idx = 0;
    line_idx = 0;
    line_start_idx = 0;
    router = "";
    _request.bodys = nullptr;
    _response.bodys = nullptr;
    for(auto hashfile:m_files){
        for(auto && file:hashfile.second){
            delete file;
            file =nullptr;
        }
    }
    m_files.clear();
    ReSetRequest();
}

string HttpConn::URL(){
    return _request.URL;
}

void HttpConn::URL(string url){
    _request.URL = url;
}

HttpMethod_t HttpConn::Method(){
    return _request.method;
}

void HttpConn::Method(HttpMethod_t method){
    _request.method = method;
}

string HttpConn::Router(){
    return router;
}

void HttpConn::Router(string router){
    this->router = router;
}
size_t HttpConn::StaticFileSize(){
    return this->file_size;
}
void  HttpConn::StaticFileSize(size_t filesize){
    this->file_size = filesize;
}

void HttpConn::StaticFileFD(int fd){
    this->file_fd = fd;
}
int HttpConn::StaticFileFD(){
    return file_fd;
}

bool HttpConn::FileReq(){
    return file_req;
}

void HttpConn::FileReq(bool isFileReq){
    this->file_req = isFileReq;
}

byte_t*  HttpConn::Address(){
    return address;
}

void  HttpConn::Address(byte_t* _address){
    this->address = _address;
}

void  HttpConn::StaticFileName(string filename){
    this->static_filename = filename;
}

std::string  HttpConn::StaticFileName(){
    return static_filename;
}

std::vector<MultipartFile*> HttpConn::PostFrom(const std::string &name){
    if(!m_files.size()){
        auto ret = _request.FromFile(name);
        for(auto file:ret){
            m_files[file->Getname()].push_back(file);
        }
    }
    return m_files[name];
}

int HttpConn::SaveUploadFile(const MultipartFile* file,const std::string path){
    int fd = open(path.c_str(),O_CREAT|O_RDWR,0666);
    if(fd < 0){
        perror("open:");
        return fd;
    }
    lseek(fd,file->Getsize() -1,SEEK_CUR);
    write(fd,"\0",1);
    auto addr =  mmap(0,file->Getsize(),PROT_WRITE,MAP_SHARED,fd,0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    memcpy(addr,file->GetContent(),file->Getsize());
    int ret = munmap(addr,file->Getsize());
    if(ret == -1){
        perror("munmap:");
        return -1;
    }
    msync(addr, file->Getsize(), MS_SYNC);
    close(fd);
    return 0;
}

int HttpConn::Write(){
    // _response ---> 
    auto respLine = _request.http_version + " " + to_string(_response.status) + " " + statusStr[_response.status] + "\r\n";
    SetHeader("Content-Length",to_string(_response.ContentLength));
    SetHeader("Content-Type",_response.ContentType);
    if(_response.cookies.size()){
        // 生成cookie
        SetHeader("Set-Cookie","1=1");
    }
    // header
    for(auto header:_response.headers){
        respLine += header.first +": " + header.second + "\r\n";
    }
    if(_response.bodys || file_req){
        respLine += "\r\n";
    }
    if(_response.bodys){
        respLine += _response.bodys;
    }
    int ret =  WriteString(respLine.c_str(),respLine.size());
    if(ret == -1){
        perror("WriteBytes");
        return ret;
    }
    if(file_req && fd >= 0){
        ret = sendfile(FD(),file_fd,0,file_size);
        if(ret == -1){
            perror("SendFile");
        }
    }
    return ret;
}

int HttpConn::WriteToJson(HttpStatus_t code,string json){
    auto len = json.size();
    _response.ContentType = "application/json";
    _response.status = code;
    _response.ContentLength = len;
    _response.bodys = new byte_t[len + 1]();
    bzero(_response.bodys,len + 1);
    memcpy(_response.bodys,json.c_str(),len + 1);
    return len + 1;
}

int HttpConn::WriteToJson(HttpStatus_t code,JsonType* obj){
    nlohmann::json j;
    obj->ToJson(j);
    auto str = j.dump();
    return WriteToJson(code,str);
}

int HttpConn::WriteToXML(HttpStatus code,string json){
    
}

int HttpConn::WriteToText(HttpStatus_t code,string text){
    auto len = text.size();
    _response.ContentType = "text/plain";
    _response.status = code;
    _response.ContentLength = text.size();
    _response.bodys = new byte_t[len + 1]();
    bzero(_response.bodys,len + 1);
    memcpy(_response.bodys,text.c_str(),len + 1);
    return 0;
}

int HttpConn::WriteToHTML(HttpStatus code,string json){}

std::string ParseFileType(string filename){
    vector<string> suffixes;
    SplitString(filename,".",suffixes);
    auto suffix = suffixes.back();
    unordered_map<string,string> suffixMap{
        {"html", "text/html"},
        {"css", "text/css"},
        {"js","application/javascript"},
        {"json", "application/json"},
        {"xml", "application/xml"},
        {"jpg", "image/jpeg"},
        {"jpeg", "image/jpeg"},
        {"png", "image/png"},
        {"gif","image/gif"},
        {"svg", "image/svg+xml"},
        {"pdf", "application/pdf"},
        {"txt", "text/plain"},
        {"csv", "text/csv"},
        {"zip", "application/zip"},
        {"tar", "application/x-tar"},
    } ;
    if(suffixMap.count(suffix)){
        return suffixMap[suffix];
    }
    return "text/plain";
}

int HttpConn::WriteToFile(){
    if(fd >= 0){
        _response.ContentType = ParseFileType(static_filename);  //文件类型
        _response.status = HttpStatus::StatusOK;
        _response.ContentLength = file_size;
    }
    return 0;
}

HTTP_RESULT_t HttpConn::ReadToRequest(){
    auto ret = ReadDataToBuff();
#ifdef MY_DEBUG
    cout<<readbuf<<endl<<endl<<endl;
#endif
    if(ret != NO_REQUEST)
        return ret;
    ret = ParseRequest();
    //line_idx 之前的已经读过了,将读过的信息重置一下
    char* buff = new char[max_read_size]();
    memcpy(buff,readbuf + line_idx,read_idx - line_idx);
    memset(readbuf,0,max_read_size);
    memcpy(readbuf,buff,read_idx - line_idx);
    read_idx = read_idx - line_idx;
    line_idx = 0;
    line_start_idx = 0;
    delete [] buff;
    return ret;
}

string HttpConn::GetHeader(string key){
    if(!_request.headers.count(key))
        return "";
    return _request.headers[key];
}

void HttpConn::SetHeader(string key,string value){
    _response.headers[key] = value;
}

string HttpConn::GetCookie(string key){
    if(!_request.cookies.count(key))
        return "";
    return _request.cookies[key];
}

void HttpConn::SetCookie(string key,string value){
    _response.cookies[key] = value;
}

string HttpConn::GetQuery(string key){
    return _request.query[key];
}

string HttpConn::GetParam(string key){
    return _request.param[key];
} // key value

int HttpConn::BindBody(void* body,string type){}

void HttpConn::BindJsonBody(JsonType* ret){
    string json = _request.bodys;
    nlohmann::json j = nlohmann::json::parse(json);
    ret->FromJson(j);
    return ;
}

string HttpConn::GetStaticFileName(){
    if(static_filename[0] != '/' && static_filename[0] != '\\'){
        return "./" + static_filename;
    }
    return static_filename;
}

HTTP_RESULT_t HttpConn::ReadDataToBuff(){
    ssize_t len = ReadString(readbuf + read_idx,1024 - read_idx);
    if(len < 0){
        perror("read");
        if(errno == EBADF){
            return CLOSED_CONNECTION;
        }
        return INTERNAL_ERROR;
    }
    if(len == 0){
        return CLOSED_CONNECTION;
    }
    read_idx += len;
    return NO_REQUEST;
}

HTTP_RESULT_t HttpConn::ParseRequest(){
    line_status = LINE_OK;
    HTTP_RESULT_t ret = NO_REQUEST;
    while(((check_status == CHECK_STATE_CONTENT) && (line_status == LINE_OK)) 
            ||((check_status != CHECK_STATE_CONTENT && ret == NO_REQUEST) && (line_status = ParseLine()) == LINE_OK)
        ){
        auto text = GetLine();
        line_start_idx = line_idx;
        switch (check_status)
        {
            case CHECK_STATE_REQUESTLINE:{
                ret = ParseRequestLine(text);
                break;
            }
            case CHECK_STATE_HEADER:{
                ret = ParseHeaderLine(text);
                if(_request.ContentLength && !_request.bodys){
                    _request.bodys = new byte_t[_request.ContentLength + 1]();
                    bzero(_request.bodys,_request.ContentLength + 1);
                }
                break;
            }
            case CHECK_STATE_CONTENT:{
                ret = ParseContentLine(readbuf + line_start_idx);
                line_status = LINE_OPEN;
                break;
            }
            default:
                return BAD_REQUEST;
        }
    }
    if(line_status==LINE_BAD) return BAD_REQUEST;
    return ret;
}

HTTP_RESULT_t  HttpConn::ParseParam(){
    vector<string> router_splite;
    vector<string> url_splite;
    SplitString(router,"/",router_splite);
    SplitString(_request.URL,"/",url_splite);
    for(ssize_t i = 0;i<router.size();i++){
        if(router[0] == '{' && router.back() == '}'){
            auto key = router.substr(1,router.size() - 2);
            if(i >= url_splite.size()){
                return INTERNAL_ERROR;
            }
            _request.param[key] = url_splite[i];
        }
    }
    auto req = _request;
    PrintRequest(req);
    return GET_REQUEST;
}


string HttpConn::GetLine(){
    return readbuf  + line_start_idx;
}

HTTP_LINE_STATUS_t HttpConn::ParseLine(){
    while(line_idx < read_idx){
        if(readbuf[line_idx] == '\r'){
            if ( line_idx + 1 == read_idx ) {
                return LINE_OPEN;
            }
            if ( readbuf[ line_idx + 1 ] == '\n' ) {
                readbuf[line_idx++] = '\0';
                readbuf[line_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        if(readbuf[line_idx] == '\n'){
            if(line_idx > 1 && readbuf[line_idx - 1] == '\r'){
                readbuf[ line_idx-1] = '\0';
                readbuf[ line_idx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        line_idx++;
    }
    return LINE_OPEN; // 没读完
}

HTTP_RESULT_t HttpConn::ParseRequestLine(string text){
    // GET /index.html?idx=1& http/1.1
    auto len = text.size();
    auto url_idx = text.find_first_of(' ',0);
    if(url_idx == -1){
        return BAD_REQUEST;
    }
    // 确定请求方法
    _request.method = methods[text.substr(0,url_idx)]; // GET
    if(_request.method == ""){
        return BAD_REQUEST;
    }
    auto v_idx = text.find_first_of(' ',url_idx + 1);  // 
    if(v_idx == -1){
        return BAD_REQUEST;
    }
    _request.http_version = text.substr(v_idx + 1,len - v_idx - 1);  // http/1.1

    // 处理查询参数 以及请求路径
    auto query_idx = text.find('?',url_idx + 1);
    if(query_idx != -1){
        _request.URL = text.substr(url_idx + 1,query_idx - url_idx - 1); // /index.html
        auto querys = text.substr(query_idx + 1,v_idx - query_idx -1);

        vector<std::string> query_arr;
        SplitString(querys,"&",query_arr);
        for(auto query:query_arr){
            auto idx = query.find_first_of('=',0);
            auto key = query.substr(0,idx);
            auto value = query.substr(idx + 1, query.size() - idx - 1); 
            key = URLdecode(key);
            value = URLdecode(value);
            _request.query[key] = value;
        }
    }else{
        _request.URL = text.substr(url_idx +1,v_idx - url_idx -1);
    }
    // 修复URL
    if(_request.URL.substr(0,7) == "http://"){
        _request.URL.substr(6,_request.URL.size() - 6);
    }
    if(_request.URL.substr(0,8) == "https://"){
        _request.URL.substr(7,_request.URL.size() - 7);
    }
    check_status = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HTTP_RESULT_t HttpConn::ParseHeaderLine(string text){
    if(text[0] == '\0'){
        if ( _request.ContentLength != 0 ) {
            check_status = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    int len = text.size();
    auto idx = text.find_first_of(":",0);
    if(idx == -1){
        return BAD_REQUEST;
    }
    auto key = text.substr(0,idx); // len  长度为idx;
    auto value = text.substr(idx + 1,len - idx - 1);
    if(value[0] == ' '){
        value = value.substr(1,value.size() - 1);
    }
    _request.headers[key] = value;
    if(key == "Connection" && value == "close"){
        _request.keepalive = false;
    }
    if(key == "Content-Type"){
        
        _request.ContentType = value;
    }
    if(key == "Content-Length"){
        _request.ContentLength = atoi(value.c_str());
    }
    if(key == "Cookie"){
        ParseCookie(value);
    }
    return NO_REQUEST;
}

HTTP_RESULT_t HttpConn::ParseContentLine(const char* text){
    if(read_idx >= _request.ContentLength -_request.bodyLength + line_start_idx ){
        memcpy(_request.bodys + _request.bodyLength,text,_request.ContentLength -_request.bodyLength);
        _request.bodyLength = _request.ContentLength;
        line_idx = _request.ContentLength -_request.bodyLength + line_start_idx;
        return GET_REQUEST;
    } 
    memcpy(_request.bodys + _request.bodyLength,text,read_idx - line_start_idx);
    line_idx = read_idx;
    _request.bodyLength += read_idx - line_start_idx;
    return NO_REQUEST;
}

void HttpConn::ParseCookie(string value){
    vector<string> cookies;
    SplitString(value,"; ",cookies);
    for(auto cookie:cookies){
        auto pos = cookie.find_first_of('=',0);
        auto len = cookie.length();
        string key = cookie.substr(0,pos);
        string value = cookie.substr(pos+1,len - pos - 1);
        _request.cookies[key] = value;
    }
    
}

bool HttpConn::KeepAlive(){
    return _request.keepalive;
}

//服务端响应完毕之后才会做清理的动作
void HttpConn::ReSetRequest(){
    //clear request
    this->line_status = LINE_OPEN;
    this->check_status = CHECK_STATE_REQUESTLINE;
    if(file_req){
        static_filename = "";
        file_req = false;
        if(file_fd >= 0){
            close(file_fd);
            file_fd = -1;
        }
        file_size = 0;
    }
    auto req = &_request;
    req->URL = "";
    req->method = "";
    req->param.clear();
    req->headers.clear();
    req->ContentLength = 0;
    req->ContentType ="";
    req->cookies.clear();
    req->bodyLength = 0;
    if(req->bodys){
        delete []req->bodys;
        req->bodys = nullptr;
    }
     
    auto resp = &_response;
    resp->status = 0;
    resp->headers.clear();
    resp->ContentLength = 0;
    resp->ContentType = "";
    resp->cookies.clear();
    if(resp->bodys){
        delete []resp->bodys;
        resp->bodys = nullptr;
    }
}

void HttpConn::RequestToWriteBuff(){

}

void PrintRequest(Request req){
#ifdef MY_DEBUG
    cout<<"req:\n"<<req.method<<" "<<req.URL<<" "<<req.http_version<<endl;
    cout<<"query\n";
    for(auto query:req.query){
        cout<<query.first<<" "<<query.second<<endl;
    }

    cout<<"param\n";
    for(auto query:req.param){
        cout<<query.first<<" "<<query.second<<endl;
    }
    cout<<"header:\n";
    for(auto header:req.headers){
        cout<<header.first<<" "<<header.second<<endl;
    }
    cout<<"body"<<endl;
    cout<<req.bodys<<endl;
    cout<<endl;
    cout<<req.ContentLength<<endl<<req.ContentType<<endl;
#endif
}
