#include<iostream>
#include<http.h>
#include<httpengine.h>
#include<resptype.h>
using namespace my;

MYLIBSOCKET_DECL_JSON_CLASS_HEAD(Resp,int,code,string,msg)
MYLIBSOCKET_DECL_CLASS_END

MYLIBSOCKET_DEFINE_CLASS(Resp,int,code,string,msg)
MYLIBSOCKET_DEFINE_JSON_SERIALIZATION(Resp,code,msg)

//声明一个类
MYLIBSOCKET_DECL_JSON_CLASS_HEAD(LoginReq,string,username,string,password,string,salt,bool,login)
public:
    bool CheckLogin();
MYLIBSOCKET_DECL_CLASS_END

// 定义一个类 主要是实现GET和SET方法
MYLIBSOCKET_DEFINE_CLASS(LoginReq,string,username,string,password,string,salt,bool,login)
// 实现ToJson和To
MYLIBSOCKET_DEFINE_JSON_SERIALIZATION(LoginReq,username,password)


bool LoginReq::CheckLogin(){
    if(this->username == "admin" && this->password == "123456"){
        return true;
    }
    return false;
}

void InitRouter(HttpEngine* engine){
    engine->Post("/login",[](HttpConn* conn){
        LoginReq req;
        Resp resp;
        conn->BindJsonBody(&req);
        if(!req.CheckLogin()){
            resp.Setcode(200);
            resp.Setmsg("账号或密码错误");
            conn->WriteToJson(HttpStatus::StatusOK,&resp);
        }else{
            conn->WriteToJson(HttpStatus::StatusOK,&req);
        }
    });
    
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
