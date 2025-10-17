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
        std::map<int, std::string> client_channels_; // socket -> current channel

        void remove_client(int client_socket);
        void broadcast_to_channel(const std::string& channel, const std::string &message,int exclude_socket=-1);
        void handle_command(int client_socket, const std::string &command);
        void send_help(int client_socket);
        void list_channels(int client_socket);
        void create_channel(int client_socket,const std::string &name);
                void join_channel(int client_socket,const std::string &name);

    public:
        ChatServer(int port=8888);
        ~ChatServer();
        void handle_client(int client_socket);
        void run();
    };