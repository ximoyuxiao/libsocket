#ifndef EPOOL_H__
#define EPOOL_H__
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
typedef int epool_t; 
class Epool{
    std::unordered_map<int,shared_ptr<HandleEventOBJ>> handleMap;
    epool_t epool;
    bool live;
    int conn;
    pthread_mutex_t connectlock;
    int max_conn;
    public:
        Epool(int _max_conn = 1024);
        virtual ~Epool();
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