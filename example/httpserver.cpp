#include<iostream>
#include<http.h>
#include<httpengine.h>
#include<resptype.h>
using namespace my;
class LoginReq:public JsonType{
    string username;
    string password;
public:
    LoginReq(string username="",string password=""):username(username),password(password){}
public:
    string UserName(){return username;}
    string PassWord(){return password;}
    void UserName(string _user){
        username = _user;
    }
    void PassWord(string _pass){
        password  = _pass;
    }
public:
    void to_json(nlohmann::json& nlohmann_json_j);
    void from_json(const nlohmann::json&nlohmann_json_j);
};
REGISTER_DEFINE_JSON_SERIALIZATION(LoginReq,username,password);

void InitRouter(HttpEngine* engine){
    engine->Post("/login",[](HttpConn* conn){
        LoginReq req;
        conn->BindJsonBody(&req);
        conn->WriteToJson(HttpStatus::StatusOK,&req);
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
