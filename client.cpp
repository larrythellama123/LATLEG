// Client side implementation of UDP client-server model

#include <bits/stdc++.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

enum class PlayerState {
    Alive,    // Defaults to 0
    Dead, // Defaults to 1
};


struct PlayerPacket {
    uint8_t player_id;
    std::vector<int> enemy_positions;
    uint8_t x;
    uint8_t y;
    uint8_t 
    PlayerState PS;
};



#define PORT     8080
#define MAXLINE  1024

int main() {
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

    //collect info from player

    // Send message to server to init player
    sendto(sockfd, hello, strlen(hello), MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Hello message sent.\n");

    // Receive reply from server about player init location 
    int n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);

    //init a map with character sprite  
    PlayerPacket received_packet;
    while(1){
        //changes to the player location then send to server
        processInput();
        std::array<uint8_t,3> payload = get_player_coords();
        sendto(sockfd, reinterpret_cast<const char *> (&payload), sizeof(payload), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
        int n = recvfrom(sockfd, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
        ssize_t bytes_received = recvfrom(
            server_fd, 
            &received_packet,         // Pointer to your struct
            sizeof(received_packet),  // Expected size in bytes
            0, 
            (struct sockaddr*)&client_addr, 
            &addr_len
        );

        if(sizeof(PlayerPacket) ==  sizeof(n)){
            processOutput(received_packet);
        }
        else{
            std::err<<"there was an error";
        }
        
    }

  

    // Close socket
    close(sockfd);

    return 0;
}