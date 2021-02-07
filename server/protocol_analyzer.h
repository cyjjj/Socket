/**
 * a protocol analyer
 * func:
 * request -> protocol class
 * xxx 三位bit表示协议
 * 100: 获取时间
 * 101: 获取服务器名字
 * 110：获取客户端列表
 * 111：发送消息
 * 000：断开
 * 001：连接
 *
 * 111：发送消息后面5位bit表示客户端编号，最多支持32个客户端
 * type:3 + id:5 正好是一个char
 */

/**
 * mypacket.toPacket（）就可以打包成一个数据包 (string)
 */

#ifndef PROTOCOL_ANALYZER_H
#define PROTOCOL_ANALYZER_H

#include <iostream>
#include <string>
#include <bitset>

class Mypacket{
private:
    unsigned int type : 3;
    unsigned int id : 5;

    std::string content;

public:
    Mypacket();

    Mypacket(int type, std::string content, int id = -1);

    void Set(int type, std::string content, int id = -1);

    int getType();

    int getId();

    std::string getContent();

    std::string toPacket();

    void printInfo();
};


/**
 * 分析数据包，得到一个Mypacket的对象
 * @param packet
 * @return Mypacket类的对象
 */
Mypacket analyzePacket(const std::string& packet);

#endif //PROTOCOL_ANALYZER_H
