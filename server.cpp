#include "server.h"

ChatServer::ChatServer(int port){
    //first we want to create the socket
    server_=socket(AF_INET,SOCK_STREAM,0);//protocol 0 means automatically chosen.AF_INET means we use IPv4 and SOCK_STREAM is TCP 

    if(server_<0){
        throw std::runtime_error("Socket creation failed");
    }
    int option=1;
    setsockopt(server_,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));//set socket option to reuse address 
    //restart server quickly after crash

    sockaddr_in server_addr{};// this is a struct containing the address family, port number and IP address


    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;//listen to all interfaces,makes it 0.0.0.0 as cherno pointed out
    //might see if there's any improvements we can do here later

    server_addr.sin_port=htons(port);//htons is host to network short and converts port number to network byte order 

    if(bind(server_,(sockaddr*)&server_addr,sizeof(server_addr))<0){
        throw std::runtime_error("Binding failed");//fail means the port is already in use elsewhere
    }

    if(listen(server_,5)<0){//listen for incoming connections,5 is the max number of queued connections
        throw std::runtime_error("Listening failed");
    }

    channels_["general"]={};//create a default channel called general
    channels_["cats"]={};

    std::println("Server started on port{} ",port);

}

void ChatServer::handle_client(int client_socket){
    char buffer[1024];// temp storage to hold incoming messages

    std::string welcome_msg="Enter username:";

    send(client_socket,welcome_msg.c_str(),welcome_msg.size(),0);//c_str gives you raw char pointer to its internal buffer , and so we send raw bytes instead of a string object. 0 means default behavior

    int bytes_read=read(client_socket,buffer,sizeof(buffer)-1);//read data from client socket into buffer, -1 to leave space for null terminator.
    //reads blocks until data is available

    if(bytes_read<=0){
        close(client_socket);
        return;
    }// hang up if no data read 

    buffer[bytes_read]='\0';//null terminate the buffer to make it a valid C-string
    

    std::string username(buffer);
    username.erase(username.find_last_not_of(" \n\r\t")+1);//trim whitespaces

    {
        std::lock_guard<std::mutex> lock(client_mutex_);
        //this lock will be released when lock goes out of scope
        clients_[client_socket]=username;//add client to the map
        channels_["general"].insert(client_socket);//add client to general channel by default
    }

    //Now we welcome the user to the actual chat room
    std::string joining_msg="Welcome "+username+"! You have joined the general channel.\n Type /help for commands\n";
    
}