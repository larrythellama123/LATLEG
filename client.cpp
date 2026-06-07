// Client side implementation of UDP client-server model

#include <bits/stdc++.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "player.hpp"


#define PORT     8080
#define MAXLINE  1024

int main() {
    std::unique_ptr<Player> player = std::make_unique<Player>(); 
    int sockfd;
    char buffer[MAXLINE];
    const char *hello = "Hello from client";
    struct sockaddr_in servaddr;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Fill server address info
    servaddr.sin_family = AF_INET;              // IPv4
    servaddr.sin_port   = htons(PORT);          // Server port
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP

    socklen_t len = sizeof(servaddr);
    PlayerPacketOutput received_packet;
    while(1){
        //changes to the player location then send to server
        Player.processInput();
        PlayerPacket payload= Player.formPacket();
        sendto(sockfd, reinterpret_cast<const char *> (&payload), sizeof(payload), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        ssize_t bytes_received = recvfrom(
            sockfd, 
            &received_packet,         // Pointer to your struct
            sizeof(received_packet),  // Expected size in bytes
            0, 
            (struct sockaddr*)&client_addr, 
            &addr_len
        );

        if(sizeof(PlayerPacketOutput) ==  sizeof(n)){
            render(received_packet);
        }
        else{
            std::err<<"there was an error";
        }
        
    }

  

    // Close socket
    close(sockfd);

    return 0;
}