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
#include <thread>
#include <chrono>

using namespace std::chrono;
#define PORT     8080
#define MAXLINE  1024
std::atomic<bool> keep_running{true};
const int TICKS_PER_SECOND = 5;


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
        
        //comment agn for non-blocking option
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); 

        const auto TICK = microseconds(1000000 / TICKS_PER_SECOND);
        const int  MAX_CATCHUP = 5;

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

        auto previousTime = steady_clock::now();
        microseconds lag(0);

        while(keep_running.load()){
            auto currentTime = steady_clock::now();
            auto elapsedTime = duration_cast<microseconds>(currentTime - previousTime);
            previousTime = currentTime;
            lag += elapsedTime;

            if (lag > TICK * MAX_CATCHUP) lag = TICK * MAX_CATCHUP;

            int steps = 0;

            while (lag >= TICK && steps < MAX_CATCHUP) {
                player->AI_move();
                PlayerPacketInput PPI = player->formPacket();
                PPI.seq_num = player->get_seq_num();
                auto payload = PPI.serialize();
                sendto(sockfd, payload.data(), payload.size(), 0,
                    (const sockaddr*)&servaddr, sizeof(servaddr));
                lag -= TICK;
                ++steps;
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
            //let the server get scheduled
            std::this_thread::sleep_for(milliseconds(1));
            
        }
        close(sockfd);
        endwin(); 
        return 0;
    }