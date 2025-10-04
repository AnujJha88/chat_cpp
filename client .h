#include<iostream>
#include<thread>
#include<string>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>


class ChatClient{
    private:
        int client_socket_;
        bool running_;
    
    public:
        ChatClient();
        ~ChatClient();

        bool connect_to_server(const std::string &host="127.0.0.1",int port=8888);
        

};