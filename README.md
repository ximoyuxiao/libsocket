# libsocket
## 项目介绍
### 项目简介
一个基于C++11和epoll的实现的轻量级网络框架,其采用了事件机制。

### 项目特点
## 安装及其使用
### 安装与依赖
1、环境Ubuntu18.04、C++11
#### 安装nlohmann/json
```sh
方法一:
    sudo apt-get install nlohmann-json-dev
方法二:
https://github.com/nlohmann/json/releases 
wget https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.tar.gz
tar -zxvf v3.11.2.tar.gz
cd json-3.11.2
mkdir build && cd build
cmake ..
make
sudo make install

```
#### 安装libsocket框架
```sh
make
make install
```
### 使用
    1、查看example的实例库进行编写代码。
    2、加入参数 -lsocket、-L /usr/lib/x86_64-linux-gnu/ -I/usr/include/libsocket/

## Q&A


## 联系方式
邮箱：a2571717957@163.com（本项目相关或网络编程相关问题请走issue流程，否则恕不邮件答复）
QQ：2571717957