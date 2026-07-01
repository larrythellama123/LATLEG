#include <atomic>
#include <thread>
#include <chrono>


template<typename T, uint32_t size>
class MPSCQueue{
    static_assert(size && !(size & (size - 1)), "size must be a power of 2");
    static constexpr unint32_t mask  = size - 1;
    struct Writer{
        bool write(T data){
            uint32_t write_idx_local = write_idx.load(std::memory_order_relaxed);
            while(!write_idx.compare_exchange_weak(write_idx_local, write_idx_local+1,std::memory_order_release, std::memory_order_relaxed)){
                uint32_t r = read_idx.load(std::memory_order_acquire);
                if(write_idx_local - r <= 0){
                    return false;
                }
            }
            uint32_t idx = write_idx_local & mask;
            blks[idx].data = data;
            blks[idx].ready.store(true, std::memory_order_release);
            return true;
        }
        MPSCQueue<T, size>* q = nullptr;
    }

    struct Reader{
        T* read(){
            auto& blk = q->blks[read_idx & mask];
            uint32_t read_idx = blk.idx.load(std::memory_order_acquire);
            if(int(new_idx - read_idx) < 0) return nullptr;
            next_idx = new_idx + 1;
            return &blk.data;
        }

        T* readLast(){
            T* ret = nullptr;
            while(T* cur = read()){
                ret=cur;
            }   
            return ret;
        }
        MPSCQueue<T, size>* q = nullptr;
    }

    alignas(128) std::atomic<uint32_t> write_idx = 0; 
    alignas(128) std::atomic<uint32_t> read_idx = 0; 
    struct alignas(64) Block
    {
        std::atomic<size_t> seq = 0;
        T data;
    }blks[size];
};