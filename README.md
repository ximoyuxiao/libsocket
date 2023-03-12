# libsocket
这是一个基于epoll的实现的网络库，其采用了事件机制

#### 项目依赖下载
```c++
#include<iostream>
#include<http.h>
#include<httpengine.h>
using namespace my;
void InitRouter(HttpEngine* engine){
    // /ping的GET请求
    engine->Get("/ping",[](HttpConn* conn){
        conn->WriteToJson(HttpStatus::StatusOK,"{\n\
            \"code\":0,\n\
            \"msg\":\"pong\"\n\u
        }");
    });
    engine->StaticFile("/static","./static/"); // 参数一：路由，参数而：文件路径
    // 404的情况
    engine->NoRouter([](HttpConn* conn){
        conn->WriteToJson(HttpStatus::StatusNotFound,"{\n\
            \"errno\":404,\n\
            \"msg\":\"page note found\"\n\
        \n\
        }");
    });
}
int main(){
    HttpEngine engine(4212); // 启动一个HTTP引擎
    InitRouter(&engine); //初始化路由
    engine.SetMode(HttpMode::M_Debug); //设置启动的模式
    engine.Run(); // 启动服务器并监听端口
    return 0;
}
```