// Server side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <memory>
#include <thread>
#include <queue>

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