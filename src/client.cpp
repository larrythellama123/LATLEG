// Client side implementation of UDP client-server model

#include <bits/stdc++.h>
#include <fcntl.h>
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
        fcntl(sockfd, F_SETFL, O_NONBLOCK);

        memset(&servaddr, 0, sizeof(servaddr));
        memset(&cliaddr, 0, sizeof(cliaddr));


        // Fill server address info
        servaddr.sin_family = AF_INET;              // IPv4
        servaddr.sin_port   = htons(PORT);          // Server port
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP

        socklen_t len = sizeof(servaddr);
        PlayerPacketOutput received_packet;
        
        std::vector<uint8_t> buffer(65535); 
        while(keep_running.load()){
            //changes to the player location then send to server
            player->processInput();
            PlayerPacketInput PPI = player->formPacket();
            std::vector<uint8_t> payload = PPI.serialize();
            sendto(sockfd, reinterpret_cast<const char*>(payload.data()),  payload.size(),  MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
            ssize_t bytes_received = recvfrom(
                sockfd, 
                reinterpret_cast<char*>(buffer.data()),      
                buffer.size(), 
                0, 
                (struct sockaddr*)&cliaddr, 
                &len
            );
            if(bytes_received == buffer.size()){
                received_packet = PlayerPacketOutput.deserialize(buffer);
                renderer->render(received_packet);
            }
            else if (bytes_received < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {

                } else {
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