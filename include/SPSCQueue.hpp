// Server side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <memory>
#include <thread>
#include <queue>

template<typename T, size_t size>
class SPSCQueue{
    private:
    static_assert(size && size & (size - 1) == 0, "size must be a power of 2");
    alignas(64) std::atomic<size_t> tail{0};
    alignas(64) std::atomic<size_t> head{0};
    std::vector<T> buffer;

    public:
    bool push(const T& item){
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail+1) & (size - 1);
        if(next_tail == head.load(std::memory_order_acquire)){
            return false;
        }
        buffer[current_tail] = item;
        tail.store(next_tail,std::memory_order_release);
        return true;
    }

    bool empty(){
        if(tail.load(std::memory_order_acquire) ==  head.load(std::memory_order_acquire)){
            return true;
        }
        return false;
    }

    bool full(){
        size_t current_tail = tail.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail+1) & (size - 1);
        if(next_tail == head.load(std::memory_order_acquire)){
            return true;
        }
        return false;
    }

    bool pop(T& data){
        size_t current_head = head.load(std::memory_order_relaxed);
        if(current_head == tail.load(std::memory_order_acquire)){
            return false;
        }
        data = buffer[current_head];
        size_t next_head = (current_head+1) & (size - 1);
        head.store(next_head, std::memory_order_release);
        return true;
    }
}
