#include<iostream>
#include<thread>
#include<map>
#include<mutex>
#include<string>
#include<set>
#include<vector>
#include<print>


#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>


class ChatServer{
    private:
        int server_;
        std::mutex client_mutex_;
        std::map<int,std::string> clients_;//socket->user
        std::map<std::string, std::set<int>> channels_;//channel->socket
    
    public:
        ChatServer(int port=8888);

    };