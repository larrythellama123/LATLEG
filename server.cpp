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
#include <condition_variable>


#define PORT     8080 
#define MAXLINE 1024 
constexpr int THREAD_POOL_SIZE = 4;

struct UDPTask{
    PlayerPacketInput PP;
    sockaddr_in client_addr;
    
    public:
        UDPTask (UDPTask&& task):PP(std::move(task.PP)),client_addr(std::move(task.client_addr)){};
        UDPTask operator=(UDPTask&& task){
            UDPTask new_task = {std::move(task.PP), std::move(task.client_addr)};
            return new_task;
        }
};


class template<typename T, size_t size> SPSCQueue{
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

        size_t size(){
            return abs(tail.load(std::memory_order_acquire) - head.load(std::memory_order_acquire));
        }

        bool empty(){
            if(this->size() > 0){
                return false;
            }
            return true;
        }

        bool full(){
            if(this->size() == 0){
                return true;
            }
            return false;
        }
}

class template<size_t size>Worker{
    private:
        std::unique_ptr<SPSCQueue<UDPTask,size>> queue = std::make_unique<SPSCQueue<UDPTask,size>>();
        std::thread thread;
        std::atomic<bool> running = false;
    public:
        Worker() : worker_thread(&Worker::worker_function, this) {
            running.store(true);
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
                thread(worker_function(queue));
                return true;
            }
            return false;
        }

        bool assign_task(UDPTask &&item){
            if(!queue->full()){
                queue->push(std::move(item));
                thread(worker_function(queue));
                return true;
            }
            return false;
        }

        void worker_function(std::unique_ptr<SPSCQueue<UDPTask,size>> queue){
            while(runnning){
                if(queue.empty()){continue;}
                UDPTask task = queue->pop();
                if(task == nullptr){
                    return;
                }
                PlayerPacketOutput payload = map->sendUpdate(task.PP);
                sendto(sockfd, reinterpret_cast<const char *> (&payload), sizeof(payload), MSG_CONFIRM, (const struct sockaddr *)&task.client_addr, sizeof(task.client_addr));
            }
            
        }
}




// Driver code 
int main() { 
    std::vector<std::unique_ptr<Worker>> workers;
    for(int i = 0 ; i < THREAD_POOL_SIZE; i++){
        workers.push_back(make_unique<Worker<5>>());
    }
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
      
    servaddr.sin_family    = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
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
        ssize_t bytes_received = recvfrom(
            sockfd, 
            &received_packet,         
            sizeof(received_packet), 
            0, 
            (struct sockaddr*)&cliaddr, 
            &len
        );
        if(sizeof(received_packet) ==  sizeof(bytes_received)){
            UDPTask task = {received_packet, cliaddr};
            for(int i=0; i < THREAD_POOL_SIZE; i++){
                if(workers[i].assign_task(std::move(task))){
                    break;
                }
            }
        }
        else{
            std::err<<"there was an error";
        }
        
    }
    return 0; 
}