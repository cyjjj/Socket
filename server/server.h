//
// Created by 叶帆 on 2021/1/2.
//

#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <cstring>
#include <vector>
#include <mutex>
#include <ctime>

#include <pthread.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define BUFFER_MAX 1024
#define CON_MAX 20
#define MY_PORT 6332


typedef pair<string, int> ip_port;
typedef pair<int , ip_port> fd_client;
typedef pair<int , fd_client> id_client;

struct info{
    int number;
    int con_fd;
};


class Server{
private:
    int sockfd;
    sockaddr_in sin;

public:
    Server(int port);
    ~Server();
    void run();

};

Server::Server(int port) {
    sockfd = socket(AF_INET,SOCK_STREAM,0);//申请socket句柄

    //定义
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sockfd, (sockaddr*)&sin,sizeof(sin));

    listen(sockfd,CON_MAX);
}

Server::~Server() {
    close(sockfd);
}


#endif //SERVER_H
