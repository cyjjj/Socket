//
// Created by 叶帆 on 2021/1/7.
//

#include "protocol_analyzer.h"

Mypacket::Mypacket(){

}

Mypacket::Mypacket(int type, std::string content, int id){
    this->type = type;
    this->id = id;
    this->content = content;
}

void Mypacket::Set(int type, std::string content, int id ){
    this->type = type;
    this->id = id;
    this->content = content;
}

int Mypacket::getType(){
    return type;
}

int Mypacket::getId(){
    if(type == 0b111)
        return id;
    else
        return -1;
}

std::string Mypacket::getContent(){
    return content;
}

std::string Mypacket::toPacket(){
    std::string rtn;

    //按位操作
//    char type_id = (char) type << 5;
//    type_id = type_id | id;

    std::string type_id;
    for(int i=0; i<3; i++){
        type_id.append(std::to_string((type>>(2-i))&1));
    }
    for(int i=0; i<5; i++){
        type_id.append(std::to_string((id>>(4-i))&1));
    }
//    std::cout<<"type_id"<<type_id<<std::endl;

    rtn.append(type_id);
    rtn.append(content);
//    std::cout<<rtn<<std::endl;
    return rtn;
}

void Mypacket::printInfo(){
    std::cout<<"type:"<<type<<std::endl;
    std::cout<<"id:"<<id<<std::endl;
    std::cout<<"content:"<<content<<std::endl;
}

Mypacket analyzePacket(const std::string& packet){

    //得到type 和 id
//    std::cout<<packet<<std::endl;

//    char type_id[1];
//    if(packet.size()>0)
//        strcpy(type_id,packet.substr(0,1).c_str());
//    int type = (*type_id & 0b11100000)>>5;
//    int id = *type_id & 0b00011111;

    //**************
    try {
        std::bitset<3> type(packet.substr(0, 3));
        std::bitset<5> id(packet.substr(3, 5));
        std::string content;
        if(packet.size()>8)
            content = packet.substr(8);

        Mypacket rtn(type.to_ulong(),content,id.to_ulong());
        return rtn;
    } catch (std::exception e) {
        Mypacket rtn(0b001,"parse packet eroor");
        return rtn;
    }


    //**************
    //得到内容

}

//int main(){
//    Mypacket mypacket = analyzePacket("00etststetq");
//    mypacket.printInfo();
//    std::cout<<mypacket.toPacket()<<std::endl;
//}