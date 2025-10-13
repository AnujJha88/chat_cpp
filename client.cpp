#include "client.h"

ChatClient::ChatClient(){
    client_socket_=-1;
    running_=false;
}

ChatClient::~ChatClient(){
    if(client_socket_!=-1)close(client_socket_);
}

bool ChatClient::connect_to_server(const std::string &host, int port){
    client_socket_=socket(AF_INET,SOCK_STREAM,0); 

    if (client_socket_<0){
        std::cerr<<"Socket creation failed\n";
        return false;
    }

    sockaddr_in server_addr{};//blank default init?

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);//port to byte network order


    if(inet_pton(AF_INET, host.c_str(),&server_addr.sin_addr)<0){
        std::cerr<<"invalid server address"<<host<<std::endl;
        close(client_socket_);
        client_socket_=-1;
        return false;
    }

    std::println("Connecting to server {}:{} \n",host,port);

    if(connect(client_socket_, (sockaddr*)&server_addr, sizeof(server_addr))<0){
        std::cerr<<"Connection failed\n";
        close(client_socket_);
        client_socket_=-1;
        return false;
    }//this is executed only if the connect call fails, otherwise we have established the connection

    std::println("Connected to server");
    return true;
}


void ChatClient::receive_msg(){
    char buffer[1024];// this is the buffer to store incoming messages
    std::println("Listening for messages...");
    while(running_){
        int bytes_received=recv(client_socket_,buffer, sizeof(buffer)-1,0);//wait for data from server
        //where to receive from-> client_socket_
        //buffer-> where to store
        //size-> size of the buffer
        //flags-> 0 means default behavior


        if(bytes_received<=0){
            if(running_)std::println("Disconnected from server");
            running_=false;
            break;
        }

        buffer[bytes_received]='\0';//null terminate the buffer to make it a valid C-string
        std::println("{}",buffer);
        std::cout.flush();//force immediate display
        }
        std::cout<<"Stopped receiving messages \n";
    }

void ChatClient::start_chat(){

    if(!connect_to_server()){
        std::cerr<<"Failed to connect to server\n";
        return;
    }
    running_=true;

    std::thread receiver_thread(&ChatClient::receive_msg,this);//ptr to function we want to use and also we want this object to receive the mesage i.e. call the function
     std::cout << "\n💬 Welcome to Terminal Chat!" << std::endl;
    std::cout << "Type your messages and press Enter to send." << std::endl;
    std::cout << "Type '/quit' to exit." << std::endl;
    std::cout << "──────────────────────────────" << std::endl;
    

    std::string input;

    while(running_){
        std::getline(std::cin,input);

        if(input=="/quit"){
            std::println("Exiting...");
            running_=false;
            break;
        }

        if(!running_)break;
        std::string message= input+"\n";
        int bytes_sent=send(client_socket_,message.c_str(),message.length(),0);

        if(bytes_sent<0){
            std::cerr<<"Failed to send message\n";
            running_=false;
            break;
        }

    }

      std::cout << "Cleaning up..." << std::endl;
        
        // Close the socket (this will make recv() return in the other thread)
        if (client_socket_ != -1) {
            close(client_socket_);
            client_socket_ = -1;
        }
        
        // Wait for receiver thread to finish
        if (receiver_thread.joinable()) {
            receiver_thread.join();
            // join() waits for the thread to complete before continuing
        }
        
        std::cout << "Client shutdown complete" << std::endl;
}

int main(){
    std::cout << "Terminal Chat Client" << std::endl;
    std::cout << "========================" << std::endl;
    
    // Create client instance and start chatting
    ChatClient client;
    client.start_chat();
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}