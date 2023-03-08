#ifndef HTTP_ENGINE_H__
#define HTTP_ENGINE_H__
#include<tcpsocket.h>
#include<httpconn.h>
#include<http.h>
#include<string>
#include<unordered_map>
#include<list>
namespace my{
typedef void (*CallBackFunc)(HttpConn*);
typedef std::list<CallBackFunc> RouterList;
typedef unordered_map<int,HttpConn*> HttpConnMap; 
typedef struct RouterNode{
    string url;
    HttpMethod_t method;
};

class EngineRouter{
protected:
    std::string prefix_url;
    list<CallBackFunc> handlerList;
    unordered_map<HttpMethod_t,CallBackFunc> method_handler;  // URL 匹配好之后根据router去寻找
    unordered_map<string,EngineRouter*> routers;
    EngineRouter* reg_router; // {ID} 类的走这条路线
private:
    EngineRouter(const EngineRouter&)=delete;
    EngineRouter& operator=(const EngineRouter&)=delete;
public:
    EngineRouter(string url = "/");
    virtual ~EngineRouter();
public:
    bool Get(std::string router,CallBackFunc func);
    bool Post(std::string router,CallBackFunc func);
    bool PUT(std::string router,CallBackFunc func);
    bool Delete(std::string router,CallBackFunc func);
    bool Any(std::string router,CallBackFunc func);
    bool Register(HttpMethod_t method,std::string router,CallBackFunc func);
    std::string VisiteURL(vector<CallBackFunc>& handlers,std::string router,HttpMethod_t method);
    int StaticFile(std::string router,std::string path); // 待实现
protected:
    EngineRouter* FindRouter(std::string router);
    EngineRouter* CreateRouter(std::string router);
    std::string FixRouter(std::string router);
    vector<std::string> SpliteRouter(std::string router);
};

class HttpEngine:public TCPServer,public EngineRouter{
public:
    HttpEngine(int port = 80,int max_conn= 1024);
    virtual ~HttpEngine();
public:
    void SetMode(HttpMode_t _http_mode);
    CallBackFunc NoRouter(CallBackFunc func);
    void Run();
    void AddConn(TCPSocket socket);
    int GetAllRouter(vector<RouterNode>& routerList);
private:
    int VisiteAllRouter(EngineRouter* curr,string url,vector<RouterNode>& routerList);
public:
    CallBackFunc NoRouterFunc;
    HttpMode_t _mode;
    HttpConnMap conn;
    int max_conn;
    int current_conn;
    bool live;
};

};
#endif