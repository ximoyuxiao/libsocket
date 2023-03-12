#include<httpconn.h>
#include<util.h>
#include<cstring>
#include<httpengine.h>
using namespace my;
void PrintRequest(Request req);
HttpConn::HttpConn(TCPSocket socket):TCPSocket(socket),max_read_size(1024){
}
HttpConn::~HttpConn(){
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

int HttpConn::Write(){
    // _response ---> 
    cout<<"Write"<<endl;
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
    if(_response.bodys){
        respLine += "\r\n";
        respLine += _response.bodys;
    }
    cout<<respLine<<endl;
    int ret =  WriteString(respLine.c_str(),respLine.size());
    if(ret == -1){
        perror("WriteBytes");
        return ret;
    }
    return ret;
}

int HttpConn::WriteToJson(HttpStatus_t code,string json){
    auto len = json.size();
    _response.ContentType = "application/json";
    _response.status = code;
    _response.ContentLength = json.size();
    _response.bodys = new byte_t[len + 1]();
    bzero(_response.bodys,len + 1);
    memcpy(_response.bodys,json.c_str(),len + 1);
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

HTTP_RESULT_t HttpConn::ReadToRequest(){
    auto ret = ReadDataToBuff();
    cout<<readbuf<<endl<<endl<<endl;
    if(ret != NO_REQUEST)
        return ret;
    ret = ParseRequest();
    //line_idx 之前的已经读过了,将读过的信息重置一下
    char buff[max_read_size];
    memcpy(buff,readbuf + line_idx,read_idx - line_idx);
    memset(readbuf,0,max_read_size);
    memcpy(readbuf,buff,read_idx - line_idx);
    read_idx = read_idx - line_idx;
    line_idx = 0;
    line_start_idx = 0;
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

HTTP_RESULT_t HttpConn::ReadDataToBuff(){
    ssize_t len = ReadString(readbuf + read_idx,1024 - read_idx);
    if(len < 0){
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
                if(_request.ContentLength){
                    _request.bodys = new byte_t[_request.ContentLength + 1];
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
    if(read_idx >= _request.ContentLength + line_start_idx ){
        memcpy(_request.bodys,text,_request.ContentLength);
        line_idx = _request.ContentLength + line_start_idx;
        return GET_REQUEST;
    } 
    memcpy(_request.bodys,text,read_idx - line_start_idx);
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

//服务端响应完毕之后才会做清理的动作
void HttpConn::ReSetRequest(){
    //clear request
    this->line_status = LINE_OPEN;
    this->check_status = CHECK_STATE_REQUESTLINE;
    auto req = &_request;
    req->URL = "";
    req->method = "";
    req->param.clear();
    req->headers.clear();
    req->ContentLength = 0;
    req->ContentType ="";
    req->cookies.clear();
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
}