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
#include <thread>
#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include <tuple>

struct SockaddrLess {
    bool operator()(const sockaddr_in& lhs, const sockaddr_in& rhs) const {
        return std::tie(lhs.sin_addr.s_addr, lhs.sin_port) < 
               std::tie(rhs.sin_addr.s_addr, rhs.sin_port);
    }
};



#define PORT     8080 
#define MAXLINE 1024 
constexpr int THREAD_POOL_SIZE = 4;

struct UDPTask{
    public:
    PlayerPacketInput PP;
    sockaddr_in client_addr;
    uint8_t player_id;
};


template<typename T, size_t size> 
class SPSCQueue{
    private:
        static_assert((size & (size-1))==0,"size must be power of 2");
        alignas(64) std::atomic<std::size_t> head{0};
        alignas(64) std::atomic<std::size_t> tail{0};
        alignas(64) std::vector<T> buffer;
    
    public:
        SPSCQueue(){
            buffer.resize(size);
        }

        bool push(const T &item){
            const std::size_t current_tail = tail.load(std::memory_order_relaxed);
            std::size_t next_tail = (current_tail+1) & (size-1);

            if(next_tail ==  head.load(std::memory_order_acquire)){
                return false;
            }

            buffer[current_tail] = item;
            tail.store(next_tail, std::memory_order_release); 
            return true;
        };    

        bool push(T &&item){
            const std::size_t current_tail = tail.load(std::memory_order_relaxed);
            std::size_t next_tail = (current_tail+1) & (size-1);

            if(next_tail ==  head.load(std::memory_order_acquire)){
                return false;
            }
            buffer[current_tail] = std::move(item);
            tail.store(next_tail, std::memory_order_release); 
            return true;
        };    
        
        bool pop(T& data){
            const size_t current_head = head.load(std::memory_order_relaxed);
            if(current_head == tail.load(std::memory_order_relaxed)){
                return false;
            }
            size_t next_head = (current_head+1) & (size-1);
            head.store(next_head, std::memory_order_release); 
            data = buffer[current_head];
            return true;
        }

        bool full(){
            const std::size_t current_tail = tail.load(std::memory_order_relaxed);
            std::size_t next_tail = (current_tail+1) & (size-1);

            if(next_tail ==  head.load(std::memory_order_acquire)){
                return true;
            }
            return false;
        }

        bool empty(){
            if(tail.load(std::memory_order_acquire) ==  head.load(std::memory_order_acquire)){
                return true;
            }
            return false;
        }

};

template<size_t size> 
class Worker{
    private:
        std::unique_ptr<SPSCQueue<UDPTask,size>> queue = std::make_unique<SPSCQueue<UDPTask,size>>();
        std::unique_ptr<Map> map = std::make_unique<Map>();
        std::thread thread;
        std::atomic<bool> running = true;
        std::set<sockaddr_in, SockaddrLess> client_addresses;
        std::map<sockaddr_in, uint8_t, SockaddrLess> player_id_map;
        uint8_t player_id = 0;
        int sockfd;
    public:
        Worker(int sock_fd_) : sockfd(sock_fd_),thread(&Worker<size>::worker_function, this){}

        ~Worker(){
            running.store(false);
            if(thread.joinable()){
                thread.join();
            }
        }

        bool assign_task(UDPTask &item){
            auto it = client_addresses.find(item.client_addr);
            if(it == client_addresses.end()){
                item.player_id = player_id++;
                client_addresses.insert(item.client_addr);
            }
            item.player_id = player_id_map[item.client_addr];
            if(!queue->full()){
                queue->push(item);
                return true;    
            }
            return false;
        }


        void worker_function(){
            UDPTask task;
            while(running.load()){
                if(queue->empty()){
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    continue;
                }
                queue->pop(task);
                PlayerPacketOutput PPO = map->sendUpdate(task.PP, task.player_id);
                std::cout << "x=" << static_cast<int>(PPO.x)
                << " y=" << static_cast<int>(PPO.y)<<std::endl;
                std::cout << "prevx=" << static_cast<int>(PPO.prev_x)
                << " seq_num    " << static_cast<int>(PPO.seq_num)<<std::endl;
                std::vector<uint8_t> payload = PPO.serialize();
                sendto(sockfd, reinterpret_cast<const char *> (payload.data()), payload.size(), MSG_CONFIRM, (const struct sockaddr *)&task.client_addr, sizeof(task.client_addr));
                if(PPO.legal){
                    PPO.active_player = false;
                    PPO.x = 0;
                    PPO.y = 0; 
                    payload = PPO.serialize();
                    for(const auto& client_addr: client_addresses){
                        if(std::memcmp(&client_addr, &task.client_addr,sizeof(sockaddr_in)) == 0){
                            continue;
                        }
                        sendto(sockfd, reinterpret_cast<const char *> (payload.data()), payload.size(), MSG_CONFIRM, (const struct sockaddr *)&client_addr, sizeof(client_addr));
                    }
                }
                
            }
            
        }
};



int main() { 
    int sockfd;     
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr,cliaddr; 
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed hey"); 
        exit(EXIT_FAILURE); 
    }

   
    Worker<64> worker(sockfd);
    socklen_t len;
    len = sizeof(cliaddr);  
    PlayerPacketInput received_packet;
    std::vector<uint8_t> buffer_(65535); 

    while(1){
        ssize_t bytes_received = recvfrom(
            sockfd, 
            reinterpret_cast<char*>(buffer_.data()),         
            buffer_.size(), 
            0, 
            (struct sockaddr*)&cliaddr, 
        &len
        );
        if(bytes_received > 0){
            received_packet = PlayerPacketInput::deserialize(buffer_);
            UDPTask task = {received_packet, cliaddr};
            std::cout << " seq_num=" << static_cast<int>(received_packet.seq_num) << std::endl;
            if(!worker.assign_task(task)){
                std::cerr<<"packet is dropped "<< sizeof(received_packet)<<std::endl;
            }
            else{
                std::cout<<"packet queued"<<std::endl;
            };
        }
        else{
            std::cerr<<"there was an unknown error";
        }
        
    }
    return 0; 
}