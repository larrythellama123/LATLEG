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
#include <chrono>

using namespace std::chrono;
#define PORT     8080
#define MAXLINE  1024
std::atomic<bool> keep_running{true};
const int TICKS_PER_SECOND = 30;
const microseconds TICK_RATE_DURATION(16666);
const microseconds TICK_RATE_BASE(0);
void signal_handler(int signum) {
    if (signum == SIGINT) {
        keep_running.store(false); 
    }
}

    int main(int argc, char* argv[]) {
        
        initscr();
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        nodelay(stdscr, TRUE);
        std::signal(SIGINT, signal_handler);
        std::unique_ptr<Player> player = std::make_unique<Player>(); 
        std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>();

        if (argc != 4 && argc > 1) {
            std::cout << "Need 3 args";
            return 1;
        }
        else if(argc == 4){
            player->init(static_cast<uint8_t>(std::stoi(argv[1])),static_cast<uint8_t>(std::stoi(argv[2])),argv[3][0]);
        }
        

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
        bool first_render = true;
        std::vector<uint8_t> buffer_(65535);
        auto previousTime = steady_clock::now();
        microseconds lag(0);

        while(keep_running.load()){
            auto currentTime = steady_clock::now();
            auto elapsedTime = duration_cast<microseconds>(currentTime - previousTime);
            previousTime = currentTime;
            lag += elapsedTime;


            // while(lag >= TICK_RATE_DURATION){
                // std::cout << "Current lag: " << lag.count() << " ms" << std::endl;
                bool render = player->processInput();
                
                if(render || first_render){
                    PlayerPacketInput PPI = player->formPacket();
                    PPI.seq_num = player->get_seq_num();
                    std::vector<uint8_t> payload = PPI.serialize();
                    sendto(sockfd, reinterpret_cast<const char*>(payload.data()),  payload.size(),  MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr));
                    first_render = false;    
                }
                // lag -= TICK_RATE_DURATION;
            // }
        
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
                if(received_packet.active_player){
                    if(received_packet.seq_num != player->get_seq_num()){
                        continue;
                    }
                    player->add_seq_num();
                    if(player->fix(received_packet)){
                        renderer->render(received_packet);
                    }
                }
                else{
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