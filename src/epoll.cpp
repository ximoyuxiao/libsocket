#include "epoll.h"
using namespace my;
#define BUFFSIZE 1024
Epoll::Epoll(int _max_conn):max_conn(_max_conn),live(false),conn(0){
}
Epoll::~Epoll(){

}
HandleEventOBJ::~HandleEventOBJ(){}
int Epoll::EventInit(){
    epoll = epoll_create(5);
    live = true;
    connectlock = PTHREAD_MUTEX_INITIALIZER;
    conn = 0;
}
int Epoll::AddEvenet(BaseSocket* socket,int event,shared_ptr<HandleEventOBJ> func){
    pthread_mutex_lock(&connectlock);
    if(conn >= max_conn){
        pthread_mutex_unlock(&connectlock);
        return -1;
    }
    conn++;
    pthread_mutex_unlock(&connectlock);
    socket->SetNonBlock();
    epoll_event ep;
    ep.data.fd = socket->FD();
    ep.events = event;
    epoll_ctl(epoll,EPOLL_CTL_ADD,socket->FD(),&ep);

    handleMap[socket->FD()] = func;
}
int Epoll::ModifyEvent(const BaseSocket* socket,int event){
    epoll_event ep;
    ep.data.fd = socket->FD();;
    ep.events = event;
    return epoll_ctl(epoll,EPOLL_CTL_MOD,socket->FD(),&ep);
}
int Epoll::DelEvent(const BaseSocket* socket){
    pthread_mutex_lock(&connectlock);
    if(conn <= 0){
        pthread_mutex_unlock(&connectlock);
        return -1;
    }
    conn--;
    pthread_mutex_unlock(&connectlock);
    epoll_event ep;
    ep.data.fd = socket->FD();
    epoll_ctl(epoll,EPOLL_CTL_DEL,socket->FD(),&ep);
    handleMap.erase(socket->FD());
}
int Epoll::CtlEvent(const BaseSocket* socket,int op,epoll_event*event){
    epoll_ctl(epoll,op,socket->FD(),event);
}
int Epoll::CloseEvent(){
    live = false;
}
void Epoll::LoopWait(){
    epoll_event __events[max_conn];
    while(live){
        int ret = epoll_wait(epoll,__events,conn,-1);
        for(int i = 0;i<ret;i++){
            auto handle = handleMap[__events[i].data.fd];
            if(handle)
                handle->handle(__events[i]);
        }
    }
}