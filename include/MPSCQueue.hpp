#include <atomic>
#include <thread>
#include <chrono>

template<typename T, uint32_t size>
class MPSCQueue{
    static_assert(size && !(size & (size - 1)), "size must be a power of 2");
    
    template<typename Writer>
    struct Writer{
        bool write(){
            while(true){
                uint32_t write_idx_local = write_idx.load(std::memory_order_relaxed);
                if(write_idx.compare_exchange_weak(write_idx_local, write_idx_local+1,std::memory_order_acquire)){
                    continue;
                }
                write_idx_local = write_idx.load(std::memory_order_acquire);
                auto& blk = blks[write_idx.store((write_idx_local+1)%size, std::memory_order_release)];
                writer(blk.data); 
                blk.ready.store(true, std::memory_order_release);
            }
        }
        
        MPSCQueue<T, size>* q = nullptr;
        struct alignas(64) Block
        {
            std::atomic<bool> ready = false;
            T data;
        }blks[size];

        alignas(128) std::atomic<uint32_t> write_idx = 0; 
    }

}