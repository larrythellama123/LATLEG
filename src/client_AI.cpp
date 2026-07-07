// Client side implementation of UDP client-server model

#include <bits/stdc++.h>
// #include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "player.hpp"
#include "renderer.hpp"
#include <csignal>
#include <atomic>
#include <memory>


#define PORT     8080
#define MAXLINE  1024
std::atomic<bool> keep_running{true};

void signal_handler(int signum) {
    if (signum == SIGINT) {
        keep_running.store(false); 
    }
}

    int main() {
        // initscr();             
        // noecho();              
        // nodelay(stdscr, TRUE); 
        // cbreak();  
        
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        std::signal(SIGINT, signal_handler);
        std::unique_ptr<Player> player = std::make_unique<Player>(); 
        std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>();
        
        //test
        // PlayerPacketOutput PP;
        // while(keep_running.load()){
        //     renderer->render(PP);
        // }

        int sockfd;
        char buffer[MAXLINE];
        const char *hello = "Hello from client";
        struct sockaddr_in servaddr, cliaddr;

        // Create UDP socket
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }
        struct timeval tv;
        tv.tv_sec = 1;  //1 sec timeout
        tv.tv_usec = 0;

        // Set the receive timeout option
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        
        //comment agn for non-blocking option
        // fcntl(sockfd, F_SETFL, O_NONBLOCK);

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));


        // Fill server address info
        servaddr.sin_family = AF_INET;              
        servaddr.sin_port   = htons(PORT);          
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

        socklen_t len = sizeof(servaddr);
        PlayerPacketOutput received_packet;
        bool first_render = true;
        std::vector<uint8_t> buffer_(65535); 

        

        while(keep_running.load()){
            //changes to the player location then send to server
            if(player->AI_move()){
                PlayerPacketInput PPI = player->formPacket();
                std::vector<uint8_t> payload = PPI.serialize();
                sendto(sockfd, reinterpret_cast<const char*>(payload.data()),  payload.size(),  MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
                first_render = false;
            }
            
            ssize_t bytes_received = recvfrom(
                sockfd, 
                reinterpret_cast<char*>(buffer_.data()),      
                buffer_.size(), 
                0, 
                (struct sockaddr*)&cliaddr, 
                &len
            );
            
            if(bytes_received > 0){
                received_packet = PlayerPacketOutput::deserialize(buffer_);
                if(player->fix(received_packet)){
                    renderer->render(received_packet);
                }
            }
            else if (bytes_received < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {} else {
                    std::cerr << "Actual socket read error occurred\n";
                }
            }
            else{
                std::cerr<<"there was an error";
            }
            
        }
        close(sockfd);
        endwin(); 
        return 0;
    }