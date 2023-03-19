#include<iostream>
#include<http.h>
#include<httpengine.h>
using namespace my;
void InitRouter(HttpEngine* engine){
    engine->Get("/ping",[](HttpConn* conn){
        conn->WriteToJson(HttpStatus::StatusOK,"{\n\
            \"code\":0,\n\
            \"msg\":\"pong\"\n\
        }");
    });

    engine->StaticFile("/static/","../static/"); // 参数一：路由，参数而：文件路径
    engine->NoRouter([](HttpConn* conn){
        conn->WriteToJson(HttpStatus::StatusNotFound,"{\n\
            cpde:404,\n\
            msg:\"page note found\"\n\
        }");
    });
}

int main(){
    HttpEngine engine(4490);
    InitRouter(&engine); //初始化路由
    engine.SetMode(HttpMode::M_Debug);
    engine.Run(); // 监听端口
    return 0;
}
