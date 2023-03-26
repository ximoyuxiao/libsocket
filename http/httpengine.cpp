#include<httpengine.h>
#include<epoll.h>
#include<vector>
#include<util.h>
#include<worker.h>
#include<threadpool.h>
#include<cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
using namespace my;

EngineRouter::EngineRouter(string url):prefix_url(url){

}

EngineRouter::~EngineRouter(){
    for(auto router:routers){
        delete router.second;
    }
    for(auto o_router:reg_router){
        delete o_router.second;
    }

}

bool EngineRouter::Get(std::string router,CallBackFunc func){
    return Register(HttpMethod::GET,router,func);
}

bool EngineRouter::Post(std::string router,CallBackFunc func){
    return Register(HttpMethod::POST,router,func);
}

bool EngineRouter::PUT(std::string router,CallBackFunc func){
    return Register(HttpMethod::PUT,router,func);
}

bool EngineRouter::Delete(std::string router,CallBackFunc func){
    return Register(HttpMethod::DELETE,router,func);
}

EngineRouter* EngineRouter::CreateRouter(std::string router){
    int idx = router.find_first_of('/',0);
    auto cur_url= router.substr(0,idx);
    auto ret = new EngineRouter(cur_url);
    this->routers[cur_url] = ret;
    return ret;
}

// return xxx/xxx/xxxx/
std::string EngineRouter::FixRouter(std::string router){
    int start = 0;
    int end = router.size() - 1;
    while(router[start] =='/' || router[start] == '\\') start++;
    if(router[end] != '/' && router[end] != '\\'){
        end++;
        router.push_back('/');
    }
    for(int i = start;i<end;i++){
        if(router[i] == '\\') router[i] = '/';
    }
    return router.substr(start,end - start + 1);
}
/*
method：方法
router:路由  /user/info/{id}
func:回调函数  路由执行该函数
ret:返回注册是否成功，一般来说失败的情况是路由冲突了或者路由错误
*/
bool EngineRouter::Register(HttpMethod_t method,std::string router,CallBackFunc func){
    if(router.size() <= 0) return false;
    router = FixRouter(router);
    vector<string> routers;
    SplitString(router,"/",routers);
    auto curr = this;
    for(auto router:routers){
        EngineRouter* next_router = nullptr;
        if(router[0] == '{' && router.back() == '}'){
            if(!reg_router.count(method)){
                 reg_router[method] = CreateRouter(router);
            }
            next_router = reg_router[method];
            // 验证该路由没有被注册过
            if(router != next_router->prefix_url){
                return false;
            }
        }else{
            next_router = curr->routers[router];
        }
        // 没注册过的情况
        if(!next_router){
            next_router = curr->CreateRouter(router);
        }
        curr = next_router;
    }
    if(!curr){
        return false;
    }
    if(curr->method_handler.count(method)){
        std::cerr<<"You cannot register multiple handlers for the same route."<<std::endl;
        return false;
    }
    curr->method_handler[method] = func;
}

vector<std::string> EngineRouter::SpliteRouter(std::string router){
    vector<std::string> ret;
    SplitString(router,"/",ret);
    return ret;
}

// set xxx/xxx/   ---> FILE:
int EngineRouter::StaticFile(std::string router,std::string path){
    router = FixRouter(router);  // 修复 路由
    if(path.back() == '/' || path.back() == '\\'){
        path = path.substr(0,path.size() - 1);
    }
    staticRouter[router] = path;
    return 0;
}

void EngineRouter::StaticFileFunc(HttpConn* conn){
    std::string filename = conn->GetStaticFileName();
    struct stat m_file_stat;
    if ( stat( filename.c_str(), &m_file_stat ) < 0 ) {
        conn->WriteToJson(HttpStatus::StatusForbidden,"{\n\
                \"code\":404,\n\
                \"msg\":\"not file found\"\n\
            }");
        return ;
    }

    // 判断访问权限
    if ( ! ( m_file_stat.st_mode & S_IROTH ) ) {
        conn->WriteToJson(HttpStatus::StatusForbidden,"{\n\
                \"code\":403,\n\
                \"msg\":\"not access\"\n\
            }");
        return ;
    }

    // 判断是否是目录
    if ( S_ISDIR( m_file_stat.st_mode ) ) {
        conn->WriteToJson(HttpStatus::StatusInternalServerError,"{\n\
                \"code\":500,\n\
                \"msg\":\"server parse failed\"\n\
            }");
        return ;
    }
    
    conn->StaticFileSize(m_file_stat.st_size);
    
    auto fd = open(filename.c_str(),O_RDONLY);
    conn->StaticFileFD(fd);
    conn->WriteToFile();
}

class ClientFunc:public HandleEventOBJ,public worker{
    Epoll* ep;
    HttpEngine* server;
    HttpConn* conn;
public:
    ClientFunc(Epoll* ep,HttpEngine* server):ep(ep),server(server){}
    HTTP_RESULT_t BuildResponse(){
        // 解析请求
        auto ret = conn->ReadToRequest();
        if(ret != GET_REQUEST){
            server->HandlerErrorResult(conn,ret);
            return ret;
        }
        // 解析路由，并压入文件请求
        vector<CallBackFunc> handlers;
        conn->Router(server->VisiteURL(handlers,conn));
        // 404的情况下
        if(conn->Router() == ""){
            if(server->NoRouterFunc) server->NoRouterFunc(conn);  
            return ret;
        }
        ret = conn->ParseParam();
        if(ret != GET_REQUEST){
            server->HandlerErrorResult(conn,ret);
            return ret;
        }

        // 正常情况 先调用钩子函数 最后调用处理函数
        for(auto handler:handlers){
            handler(conn);
        }
        return GET_REQUEST;
    }
    void run(){
        auto ret = BuildResponse();
        if(ret == NO_REQUEST || ret == CLOSED_CONNECTION){
            return ;
        }
        conn->Write();
        conn->ReSetRequest();
        if(!conn->KeepAlive()){
            server->CloseClient(conn);
        }
        ep->ModifyEvent(conn,EPOLLIN|EPOLLRDHUP|EPOLLONESHOT);
        return ;
    }
    
    void handle(epoll_event event){
        conn = server->clients[event.data.fd];
        if(!conn) return;
        if(event.events & EPOLLIN){
            auto tpool = threadpool::getPool();
            tpool->excute(std::shared_ptr<worker>(new ClientFunc(*this)));
            return ;
        }

        if(event.events & (EPOLLRDHUP|EPOLLHUP|EPOLLERR)){
            if(server->_mode == HttpMode::M_Debug)
                cout<<"连接断开"<<endl;
            server->CloseClient(conn);
            return ;
        }
       /*if(event.events & EPOLLOUT){
            auto ret = conn->Write();
            if(!ret){
                ep->DelEvent(conn);
                conn->ShutDown(SHUT_RDWR);
                conn->Close();
                return ;
            }
            conn->ReSetRequest();
            ep->ModifyEvent(conn,EPOLLIN|EPOLLRDHUP|EPOLLONESHOT);
            return ;
        }*/
        return ;
    }
};

class ServerFunc:public HandleEventOBJ{
    Epoll* ep;
    HttpEngine* server;
    public:
        ServerFunc(Epoll* ep,HttpEngine* server):ep(ep),server(server){}
    public:
        void handle(epoll_event event){
            auto socket = server->Accept();
            if(server->max_conn  <= server->current_conn){
                // 执行拒绝连接函数
                socket.ShutDown(SHUT_RDWR);
                socket.Close();
                return ;
            }
            server->AddConn(socket);
            ep->AddEvenet(&socket,EPOLLIN|EPOLLRDHUP|EPOLLONESHOT,shared_ptr<HandleEventOBJ>(new ClientFunc(ep,server)));
        }
};


HttpEngine::HttpEngine(int port,int max_conn):TCPServer("0.0.0.0",port),\
NoRouterFunc(nullptr),_mode(HttpMode::M_Default),max_conn(max_conn),current_conn(0),live(true),\
EngineRouter("/"){}


HttpEngine::~HttpEngine(){
   for(auto router:routers){
        delete router.second;
    }
    for(auto router:reg_router)
        delete router.second;
    for(auto con:clients){
        CloseClient(con.second);
    }
}

void HttpEngine::SetMode(HttpMode_t _http_mode){
    this->_mode = _http_mode;
}

CallBackFunc HttpEngine::NoRouter(CallBackFunc func){
    auto ret = NoRouterFunc;
    NoRouterFunc = func;
    return ret;
}

void HttpEngine::Run(){
    if(Listen()<0){
        perror("Listen failed");
        return ;
    }
    my::Epoll ep;
    ep.EventInit();
    this->ep = &ep;
    auto func = shared_ptr<HandleEventOBJ>(new ServerFunc(&ep,this));
    ep.AddEvenet(this,EPOLLIN,func);
    ep.LoopWait();
}

void HttpEngine::AddConn(TCPSocket socket){
    HttpConn *con = new HttpConn(socket,this);
    socket.SetNonBlock();
    int reuse = 1;
    setsockopt(socket.FD(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );
    this->max_conn++;
    clients[socket.FD()] = con; 
    con->InitConn();
}

int HttpEngine::GetAllRouter(vector<RouterNode>& routerList){
    EngineRouter* router = (EngineRouter*)this;
    string url = this->prefix_url;
    VisiteAllRouter(router,url,routerList);
}

void HttpEngine::HandlerErrorResult(HttpConn* client,HTTP_RESULT_t result,CallBackFunc func){
    if(result == GET_REQUEST) return ;
    if(func) func(client);
    if(result == BAD_REQUEST){
        client->WriteToJson(HttpStatus::StatusBadRequest,"{\n\
            \"code\":401,\n\
            \"msg\":\"BAD REQUEST\"\n\
            }");
        return ;
    }
    if(result == NO_REQUEST){
          int ret = ep->ModifyEvent(client,EPOLLIN|EPOLLRDHUP|EPOLLONESHOT);
          if(ret == -1){
            cout<<client->FD()<<endl;
            perror("epool");
          }
          return ;
    }
    if(result == CLOSED_CONNECTION){
        CloseClient(client);
        return ;
    }
    if(result == INTERNAL_ERROR){
        client->WriteToJson(HttpStatus::StatusInternalServerError,"{\n\
    \"code\":505,\n\
    \"msg\":\"SERVER INTERNAL ERROR\"\n\
}");
        return ;
    }
}

int HttpEngine::VisiteAllRouter(EngineRouter* curr,string url,vector<RouterNode>& routerList){
    // 放入所有的叶子节点
    
    // dynamic router
    for(auto handler:curr->method_handler){
        routerList.push_back(RouterNode{
            url:url,
            method:handler.first,
        });
    }
    
    // static router
    for(auto path:curr->staticRouter){
        routerList.push_back(RouterNode{
            url:url + path.first + "/*",
            method:HttpMethod::GET,
        });
    }
    
    // 下一个路由
    for(auto router:curr->routers){
        auto newurl =  url + router.first + "/";
        VisiteAllRouter(router.second,newurl,routerList);
    }

    // 正则路由
    for(auto router:curr->reg_router){
        auto newurl =  url + router.first + "/";
        VisiteAllRouter(router.second,newurl,routerList);
    }
    
    return 0;
}

/*
return: /url/xxx/
*/
std::string HttpEngine::VisiteURL(vector<CallBackFunc>& handlers,HttpConn* conn){
    auto router = conn->URL();
    auto method = conn->Method();
    if(router.size() <= 0) return "";
    bool noRouter = false;
    string path = "/";
    EngineRouter* curr = this;
    router = FixRouter(router);
    auto routers = SpliteRouter(router);
    auto idx = 0; 
    for(auto router:routers){
        // 匹配静态路由
        string staticURL = "";
        for(int i = idx;i<routers.size();i++){
            staticURL += routers[idx] + "/";
            if(curr->staticRouter.count(staticURL)){
                path += staticURL;
                handlers.push_back(StaticFileFunc);
                auto FilePrefix = curr->staticRouter[staticURL];
                string file = "";
                for(int j = i+1;j<routers.size();j++){
                    file +="/" +  routers[j];
                }  
                conn->StaticFileName(FilePrefix + file);
                conn->FileReq(true);
                return path;
            }
        }
        idx++;
        // 中间回调函数
        for(auto handler:curr->handlerList){
            handlers.push_back(handler);
        }   
        
        // 匹配路由
        if(curr->routers.count(router)){
            curr = curr->routers[router];
            path +=  curr->prefix_url  + "/";
            continue;
        }
        
        // 匹配正则路由
        if(curr->reg_router.count(method)){
            curr = curr->reg_router[method];
            path +=  curr->prefix_url  + "/";
            continue;
        }
        noRouter = true;
    }
    if(!noRouter && curr->method_handler.count(method)){
        for(auto handler:curr->handlerList){
            handlers.push_back(handler);
        }
        handlers.push_back(curr->method_handler[method]);
        return path;
    }
    handlers = vector<CallBackFunc>();
    return "";
}

void HttpEngine::CloseClient(HttpConn* client){
    client->Close();
    ep->DelEvent(client);
    auto conn = clients[client->FD()];
    delete conn;
    clients.erase(client->FD());
}