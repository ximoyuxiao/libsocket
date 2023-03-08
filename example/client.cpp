#include<tcpsocket.h>
#include "cstring"
#define PORT   "4212"
#define SERVER "127.0.0.1"
using namespace my;
int main(){
    TCPClient client(SERVER,atoi(PORT));
    client.Connection();
    client.WriteString("hello,world\n",strlen("hello,world"));
    char buff[1024] ="";
    client.ReadString(buff,1024);
    printf("%s\n",buff);
    client.ShutDown(SHUT_RDWR);
    client.Close();
}