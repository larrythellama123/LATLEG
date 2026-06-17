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
};

class TaskQueue{
    private:
        std::queue<UDPTask> task_queue;
        std::mutex mtx;
        std::condition_variable cv;
        bool stop_requested  = false;

    public:
        void push(const UDPTask& task){
            task_queue.push(task);
        }

        void pop(){
            
        }
}

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
            if(size > 0){
                return false;
            }
            return true;
        }
}

class template<size_t size>Worker{
    private:
        std::unique_ptr<SPSCQueue<UDPTask,size>> queue = std::make_unique<SPSCQueue<UDPTask,size>>();
        std::thread thread;
    public:
        bool assign_task(const UDPTask &item){
            if(!queue->full()){
                queue->push(item);
                thread(worker_function(queue));
                return true;
            }
            return false;
        }

        void worker_function(std::unique_ptr<SPSCQueue<UDPTask,size>> queue){
            UDPTask task = queue->pop();
            if(task == nullptr){
                return;
            }
            PlayerPacketOutput payload = map->sendUpdate(task.PP);
            sendto(sockfd, reinterpret_cast<const char *> (&payload), sizeof(payload), MSG_CONFIRM, (const struct sockaddr *)&task.client_addr, sizeof(task.client_addr));
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

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET,&(cliaddr.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(cliaddr.sin_port);
        if(sizeof(received_packet) ==  sizeof(bytes_received)){
            UDPTask task = {received_packet, }
        }
        else{
            std::err<<"there was an error";
        }
        
    }
    return 0; 
}