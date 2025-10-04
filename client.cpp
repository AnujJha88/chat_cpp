#include "client .h"

ChatClient::ChatClient(){
    client_socket_=-1;
    running_=false;
}

ChatClient::~ChatClient(){
    if(client_socket_!=-1)close(client_socket_);
}

bool ChatClient::connect_to_server(const std::string &host, int port){
    
}