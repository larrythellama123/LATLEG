// Client side implementation of UDP client-server model

#include <bits/stdc++.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "player.hpp"
#include "renderer.hpp"
#include <csignal>
#include <atomic>
#include <fcntl.h>
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
        std::signal(SIGINT, signal_handler);
        std::unique_ptr<Player> player = std::make_unique<Player>(); 
        std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(); 

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
        while(keep_running.load()){
            //changes to the player location then send to server
            player->processInput();
            PlayerPacketInput payload= player->formPacket();
            sendto(sockfd, reinterpret_cast<const char *> (&payload), sizeof(payload), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
            ssize_t bytes_received = recvfrom(
                sockfd, 
                &received_packet,         
                sizeof(received_packet),  
                0, 
                (struct sockaddr*)&cliaddr, 
                &len
            );

            if(bytes_received ==  sizeof(received_packet)){
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

    

        // Close socket
        close(sockfd);

        return 0;
    }