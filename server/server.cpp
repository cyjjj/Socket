//
// Created by 叶帆 on 2021/1/2.
//

#include "server.h"
#include "protocol_analyzer.h"
#include <map>

int clientListInfo[32]={0};

int getNumber(){
    for(int i = 0 ; i<32 ; i++){
        if(clientListInfo[i]==0){
            clientListInfo[i]=1;
            return i;
        }
    }
    return -1;
}//可以用互斥锁加强安全

map< int , fd_client > clientList;

void* con_thread_handle(void* arg);
string requestProcess(const string& request);

void Server::run() {
    cout<<"Start the Server"<<endl;
    while (true){

        //等待连接
        sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);
        int con_fd = accept(sockfd,(sockaddr*)&client_addr,(socklen_t*)&addrlen);

        cout<<"A client connect"<<endl;//有客户端连接
        int Number = getNumber();
        if(Number ==-1){

        }
        clientList.insert( id_client ( Number , fd_client(con_fd,
                                       ip_port(inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port))) ) );
        cout<<inet_ntoa(client_addr.sin_addr)<<":"<<client_addr.sin_port<<endl;
        pthread_t con_thread;

        info info = {
                Number,
                con_fd
        };

        pthread_create(&con_thread, NULL,con_thread_handle,&info);
    }
}

void* con_thread_handle(void* arg){
    info info = *(struct info *)arg;

    int con_fd = info.con_fd;
    int number = info.number;

    char buffer_recv[BUFFER_MAX];
    string reply;

    while (true){
        if(recv(con_fd,buffer_recv,BUFFER_MAX,0)==0)
            break;//收到信息
        reply.clear();
        string request = buffer_recv;

        reply = requestProcess(request);
        cout<<"reply:\n"<<reply<<endl;
        send(con_fd,reply.c_str(),reply.size(),0);//发送信息
        if(analyzePacket(reply).getType()==0b000){
            cout<<"Disconnect : "<<number<<endl;
            break;
        }

    }

    close(con_fd);
    clientListInfo[number]=0;
}

string requestProcess(const string& request){
    cout<<"requestProcess():"<<endl;
    Mypacket mypacket = analyzePacket(request);
    cout<<"packet:"<<endl;
    mypacket.printInfo();
    Mypacket reply;
    switch (mypacket.getType()) {
        case 0b100:
        {
            time_t t = time(nullptr);
            char buf[128]= {0};
            strftime(buf, 64, "%Y-%m-%d %H:%M:%S", localtime(&t));
            reply.Set(0b100,buf);
            break;
        }

        case 0b101:
        {
            char bufHostName[256];
            gethostname(bufHostName,256);
            reply.Set(0b101,bufHostName);
            break;
        }
        case 0b110:
        {
            //getClinents
            string content;
            for(auto &it : clientList){
                content.append(to_string(it.first)+"|");
                content.append(it.second.second.first+":"+to_string(it.second.second.second)+"\n");
            }

            reply.Set(0b110,content);
            break;
        }
        case 0b111:
        {
            //sendMessage
            int id = mypacket.getId();
            if(clientListInfo[id]==1){
                auto  iter = clientList.find(id);
                Mypacket sendMsg(0b111,mypacket.getContent());
                string content = sendMsg.toPacket();
                send( iter->second.first,content.c_str(),content.size(),0);
                //TODO : 检查是否送达
            }
            reply.Set(0b111,"send success"+to_string(id));
            break;
        }
        case 0b000:
        {
            reply.Set(0b000,"disconnect");
            break;
        }
        case 0b001:
        {

            reply.Set(0b001,"connect");
            break;
        }

        default:
        {
            cout<<"default"<<endl;
            reply.Set(0b011,"error");
        }
    }

    return reply.toPacket();
}


int main(int argc, char const *argv[]){
    Server server(MY_PORT);
    server.run();
}
