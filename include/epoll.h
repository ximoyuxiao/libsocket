#ifndef EPOLL_H__
#define EPOLL_H__
#include<unordered_map>
#include<sys/epoll.h>
#include "socktype.h"
#include "socket.h"
#include "pthread.h"
#include "memory"
namespace my{
class HandleEventOBJ{
    public:
        virtual void handle(epoll_event event) = 0;
        virtual ~HandleEventOBJ();
};
typedef int epoll_t; 
class Epoll{
    std::unordered_map<int,shared_ptr<HandleEventOBJ>> handleMap;
    epoll_t epoll;
    bool live;
    int conn;
    pthread_mutex_t connectlock;
    int max_conn;
    public:
        Epoll(int _max_conn = 1024);
        virtual ~Epoll();
    public:
        int EventInit();
        int AddEvenet(BaseSocket*,int,shared_ptr<HandleEventOBJ>);
        int ModifyEvent(const BaseSocket*,int);
        int DelEvent(const BaseSocket*);
        int CtlEvent(const BaseSocket*,int,epoll_event*);
        int CloseEvent();
        void LoopWait();
};
};

#endif