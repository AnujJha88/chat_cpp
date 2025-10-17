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
ChatServer::~ChatServer() {
    if (server_ != -1) {
        close(server_);
    }
}

void ChatServer::handle_client(int client_socket){
    char buffer[1024];// temp storage to hold incoming messages

    std::string welcome_msg="Enter username:";

    send(client_socket,welcome_msg.c_str(),welcome_msg.size(),0);//c_str gives you raw char pointer to its internal buffer , and so we send raw bytes instead of a string object. 0 means default behavior

    int bytes_read=read(client_socket,buffer,sizeof(buffer)-1);//read data from client socket into buffer, -1 to leave space for null terminator.
    //blocks until data is available, or an error occurs

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

    send(client_socket,joining_msg.c_str(),joining_msg.size(),0);//again we send it as raw bytes and stuff.

    broadcast_to_channel("general",username+" has joined the channel.\n",client_socket);// basically like the joining notif you get on discord, and we exclude the client socket so they don't get notified of their own joining

    while(true){

        bytes_read=read(client_socket,buffer,sizeof(buffer)-1);
        if(bytes_read<=0){
            break;//client disconnected or error
        }

        buffer[bytes_read]='\0';
        std::string message(buffer);
        message.erase(message.find_last_not_of(" \n\r\t")+1);//trim whitespaces

        if(message=="/quit"){
            break;//client want to exit
        }

        handle_command(client_socket,message);

        //broadcast the message to all clients in the same channel
        std::lock_guard<std::mutex> lock(client_mutex_);
        std::string formatted_msg=username+": "+message+"\n";

        for(int sock:channels_["general"]){//for now we just have one channel, general
            if(sock!=client_socket){//exclude sender
                send(sock,formatted_msg.c_str(),formatted_msg.size(),0);
            }
        }
    }

    remove_client(client_socket);
close(client_socket);



}

void ChatServer::broadcast_to_channel(const std::string& channel, const std::string &message, int exclude_socket){
    //again we use the guard to lock the mutex

    std::lock_guard<std::mutex> lock(client_mutex_);

    auto channel_it=channels_.find(channel);
    if(channel_it==channels_.end())return;

    //now we send the message to all clients in the channel except the excluded one
    for(int sock:channel_it->second){
        if(sock!=exclude_socket){
            send(sock,message.c_str(),message.size(),0);
    }
}
}

void ChatServer::remove_client(int client_socket){
    std::lock_guard<std::mutex> lock(client_mutex_);//always lock when doing this kind of thing to prevent random ass missing errors or some shi

    auto client_idx=clients_.find(client_socket);
    
    if(client_idx==clients_.end())return;// doesnt exist 

    std::string username=client_idx->second;
    std::println("Removing client: {} \n",username);

    //remove it from all channels
    for(auto& channel_pair:channels_){
        size_t removed_count = channel_pair.second.erase(client_socket);

        if(removed_count>0){
            std::string leave_msg=username+" left the channel. See ya!\n";
            broadcast_to_channel(channel_pair.first,leave_msg,client_socket);
        }
    }

    clients_.erase(client_socket);
     std::cout << "Client " << username << " completely removed." << std::endl;

}

void ChatServer::run(){
    while(true){

        sockaddr_in client_addr{};
        socklen_t client_len=sizeof(client_addr);

        int client_socket=accept(server_,(sockaddr*)&client_addr,&client_len);// blocks till a client connects

        if(client_socket<0){
            std::cerr<<"Failed to accept client connection\n";
            continue;//try again
        }

        char client_ip[INET_ADDRSTRLEN];//human readable buffer for IP address, INET_ADDRSTRLEN is defined in <arpa/inet.h> and is 16 for IPv4
        inet_ntop(AF_INET,&client_addr.sin_addr,client_ip,INET_ADDRSTRLEN);//does not write into memory so we pass in the buffer where we want it to write to.Again AF_INET tells us this is IPv4 and the second param s the byte format of the IP addres that we receive the other two are just the location and the size of the buffer
        // also ntop is network to presentation

        // log the new connection
        std::println("New connection from {}",client_ip);

        std::thread client_thread(&ChatServer::handle_client,this,client_socket);//spawn a new thread to handle the client
        client_thread.detach();//detach the thread to let it run independently
       
    }
}
void ChatServer::send_help(int client_socket){
    std::string help_msg=R"(
═══════════════════════════════════════
           CHAT COMMANDS
═══════════════════════════════════════
/help              - Show this help message
/channels          - List all available channels
/join <channel>    - Join a channel
/create <channel>  - Create a new channel
/users             - List users in current channel
/quit              - Exit the chat
═══════════════════════════════════════
)";
send(client_socket,help_msg.c_str(),help_msg.size(),0);
}

void ChatServer::list_channels(int client_socket){
    std::lock_guard<std::mutex> lock(client_mutex_);

    std::string msg = "\nAvailable Channels:\n";
    msg += "━━━━━━━━━━━━━━━━━━━━━━\n";
    
    for (const auto& [channel_name, members] : channels_) {
        msg += "  • " + channel_name + " (" + std::to_string(members.size()) + " users)\n";
    }
    
    msg += "━━━━━━━━━━━━━━━━━━━━━━\n";
    send(client_socket, msg.c_str(), msg.size(), 0);
}

void ChatServer::create_channel(int client_socket,const std::string &name){
    // ok so we lock first 
    std::lock_guard<std::mutex> lock(client_mutex_);

    if(channels_.find(name)!=channels_.end()){
        std::string msg = " Channel '" + name + "' already exists.\n";
        send(client_socket, msg.c_str(), msg.size(), 0);
        return;
    }

    std::string username=clients_[client_socket];
    std::string current_channel=client_channels_[client_socket];
    //make new entry in map
    channels_[name]={};
    // if others in the current channel send them message saying the user left this channel

    if(!current_channel.empty()){
            channels_[current_channel].erase(client_socket);
            std::string goodbye_msg=username+"left the channel \n";

            for(int sock: channels_[current_channel]){
                send(sock, goodbye_msg.c_str(), goodbye_msg.size(), 0);
            }
    }

    // we want to update the mappings handling the user to channel stuff now ig

    client_channels_[client_socket]=name;
    // so that is taken care of so we now joined the channel when we are the one creating it.


    
}

void ChatServer::join_channel(int client_socket, const std::string &name){
    // here what we want to do is 
        client_channels_[client_socket]=name;
// and now maybe we send a message to all other members in the same channel that so and so user has joined

std::string username=clients_[client_socket];
std::string intro_message= username+"has joined the channel";

for(auto sock: channels_[name]){
    send(sock,intro_message.c_str(),intro_message.size(),0);
}

// do we need to do any more stuff?

// i guess handling the actual switching of places where the messages appear is left.
}

void ChatServer::handle_command(int client_socket, const std::string &message){
    if(message=="/help"){
        send_help(client_socket);
    }
    else if(message=="/channels"){
        list_channels(client_socket);
    }
    else if(message=="/create"){
        std::string channel_name;
            std::getline(std::cin,channel_name);

            create_channel(client_socket,channel_name);
    }
    else if(message=="/join"){

    }
    else{

    }
}


int main(){
    try{
        ChatServer server(8888);
        server.run();
    }
    catch(const std::exception &e){
        std::cerr<<"Error: "<<e.what()<<"\n";
        return 1;
    }
    return 0;
    }