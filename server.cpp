// Server side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include "player.hpp"
#include "map.hpp"
#include <memory>

#define PORT     8080 
#define MAXLINE 1024 
  
// Driver code 
int main() { 
    std::unique_ptr<Map> map = std::make_unique<Map>();
    int sockfd;     
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    socklen_t len;
    len = sizeof(cliaddr);  
    PlayerPacketInput received_packet;
    while(1){
        //changes to the player location then send to server
        ssize_t bytes_received = recvfrom(
            sockfd, 
            &received_packet,         
            sizeof(received_packet), 
            0, 
            (struct sockaddr*)&cliaddr, 
            &len
        );

        if(sizeof(PlayerPacketInput) ==  sizeof(bytes_received)){
            PlayerPacketOutput payload = map->sendUpdate(received_packet);
            sendto(sockfd, reinterpret_cast<const char *> (&payload), sizeof(payload), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        }
        else{
            std::err<<"there was an error";
        }
        
    }
      
    return 0; 
}