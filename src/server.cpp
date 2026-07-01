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


#define PORT     8080 
#define MAXLINE 1024 
constexpr int THREAD_POOL_SIZE = 4;

struct UDPTask{
    PlayerPacketInput PP;
    sockaddr_in client_addr;
};


template<typename T, size_t size> 
class SPSCQueue{
    private:
        static_assert((size & (size-1))==0,"size must be power of 2");
        alignas(64) std::atomic<std::size_t> head{0};
        alignas(64) std::atomic<std::size_t> tail{0};
        alignas(64) std::vector<T> buffer;
    
    public:
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
        
        T pop(){
            const size_t current_head = head.load(std::memory_order_relaxed);
            if(current_head == tail.load(std::memory_order_relaxed)){
                return false;
            }
            std::size_t next_head = (current_head+1) & (size-1);
            head.store(next_head, std::memory_order_release); 
            return buffer[current_head];
        }

        size_t count(){
            return tail.load(std::memory_order_acquire) - head.load(std::memory_order_acquire);
        }

        bool empty(){
            if(count() > 0){
                return false;
            }
            return true;
        }

        bool full(){
            if(count() == 0){
                return true;
            }
            return false;
        }
};

template<size_t size> 
class Worker{
    private:
        std::unique_ptr<SPSCQueue<UDPTask,size>> queue = std::make_unique<SPSCQueue<UDPTask,size>>();
        std::thread thread;
        std::atomic<bool> running = false;
        int sockfd;
    public:
        Worker(int sock_fd_) : sockfd(sock_fd_){
            running.store(true);
            thread(&Worker::worker_function, this);
        }

        ~Worker(){
            running.store(false);
            if(thread.joinable()){
                thread.join();
            }
        }

        bool assign_task(const UDPTask &item){
            if(!queue->full()){
                queue->push(item);
                return true;
            }
            return false;
        }


        void worker_function(){
            while(running.load()){
                if(queue.empty()){
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    continue;
                }
                UDPTask task = queue->pop();
                PlayerPacketOutput payload = map->sendUpdate(task.PP);
                sendto(sockfd, reinterpret_cast<const char *> (&payload), sizeof(payload), MSG_CONFIRM, (const struct sockaddr *)&task.client_addr, sizeof(task.client_addr));
            }
            
        }
};



int main() { 
    
    std::unique_ptr<Map> map = std::make_unique<Map>();
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
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

   
    Worker<20> worker;
    socklen_t len;
    len = sizeof(cliaddr);  
    PlayerPacketInput received_packet;
    while(1){
        ssize_t bytes_received = recvfrom(
            sockfd, 
            &received_packet,         
            sizeof(received_packet), 
            0, 
            (struct sockaddr*)&cliaddr, 
            &len
        );
        if(sizeof(received_packet) ==  bytes_received){
            UDPTask task = {received_packet, cliaddr};
            if(!worker.assign_task(task)){
                std::cerr<<"packet is dropped"<<std::endl;
            };
        }
        else{
            std::cerr<<"there was an unknown error";
        }
        
    }
    return 0; 
}