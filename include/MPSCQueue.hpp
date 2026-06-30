#include <atomic>

template<typename T, uint32_t size>
class MPSCQueue{
    static_assert(size && !(size & (size - 1)), "size must be a power of 2");
    struct Reader{
        T* read(){
            auto& blk = q->blks[next_idx % size];
            uint32_t new_idx = blk.idx.load(std::memory_order_acquire);
            if(int(new_idx - next_idx) < 0) return nullptr;
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
        uint32_t next_idx;
    }

    struct Writer{
        T* write(){
            auto& blk = blks[++write_idx % size];
            writer(blk.data); 
            blk.idx.store(write_idx, std::memory_order_release);
            auto& blk = q->blks[next_idx % size];
            uint32_t new_idx = blk.idx.load(std::memory_order_acquire);
            if(int(new_idx - next_idx) < 0) return nullptr;
            next_idx = new_idx + 1;
            return &blk.data;
        }

        
        
        MPSCQueue<T, size>* q = nullptr;
        uint32_t next_idx;
    }

    template<typename Writer>
    write(Writer writer){
        auto& blk = blks[++write_idx % size];
        writer(blk.data); 
        blk.idx.store(write_idx, std::memory_order_release);   
    }

    private:
    friend class Reader;
    struct alignas(64) Block
    {
        std::atomic<uint32_t> idx = 0;
        T data;
    }blks[size];

    alignas(128) uint32_t write_idx = 0; 
}