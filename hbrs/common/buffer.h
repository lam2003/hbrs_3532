#pragma once

//self
#include "global.h"

namespace rs
{

struct MMZBuffer
{
    explicit MMZBuffer(int requested_size)
    {
        int ret = HI_MPI_SYS_MmzAlloc(&phy_addr, reinterpret_cast<void **>(&vir_addr), nullptr, "ddr1", requested_size);
        RS_ASSERT(ret == 0);
    }
    virtual ~MMZBuffer()
    {
        int ret = HI_MPI_SYS_MmzFree(phy_addr, vir_addr);
        RS_ASSERT(ret == 0);
    }
    uint8_t *vir_addr;
    uint32_t phy_addr;
};

template <unsigned BlockSize>
struct default_block_allocator_malloc_free
{
    enum
    {
        requested_size = BlockSize
    };
};

typedef default_block_allocator_malloc_free<1 * 1024> allocator_1k;
typedef default_block_allocator_malloc_free<2 * 1024> allocator_2k;
typedef default_block_allocator_malloc_free<4 * 1024> allocator_4k;
typedef default_block_allocator_malloc_free<8 * 1024> allocator_8k;
typedef default_block_allocator_malloc_free<16 * 1024> allocator_16k;
typedef default_block_allocator_malloc_free<32 * 1024> allocator_32k;
typedef default_block_allocator_malloc_free<64 * 1024> allocator_64k;
typedef default_block_allocator_malloc_free<128 * 1024> allocator_128k;
typedef default_block_allocator_malloc_free<256 * 1024> allocator_256k;
typedef default_block_allocator_malloc_free<512 * 1024> allocator_512k;
typedef default_block_allocator_malloc_free<1024 * 1024> allocator_1024k;
typedef default_block_allocator_malloc_free<2048 * 1024> allocator_2048k;
typedef default_block_allocator_malloc_free<4096 * 1024> allocator_4096k;

template <typename BlockAllocator = allocator_1024k>
class Buffer
{
public:
    typedef BlockAllocator allocator;
    Buffer() : mmz_buffer_(allocator::requested_size)
    {
        data_ = mmz_buffer_.vir_addr;
        start_pos_ = 0;
        end_pos_ = 0;
    }

    inline bool Append(uint8_t *data, uint32_t len)
    {
        if (FreeSpace() < len)
            return false;

        if (allocator::requested_size - end_pos_ < len)
        {
            uint32_t size = Size();
            memmove(data_, data_ + start_pos_, size);
            start_pos_ = 0;
            end_pos_ = size;
        }
        memcpy(data_ + end_pos_, data, len);
        end_pos_ += len;
        return true;
    }

    inline bool Get(uint8_t *buf, uint32_t size)
    {
        if (Size() < size)
            return false;

        memcpy(buf, data_ + start_pos_, size);
        start_pos_ += size;
        return true;
    }
    inline uint32_t Size() const
    {
        return end_pos_ - start_pos_;
    }

    inline uint32_t FreeSpace() const
    {
        return allocator::requested_size - Size();
    }

    inline uint8_t *GetCurrentPos()
    {
        return data_ + start_pos_;
    }

    inline bool Consume(uint32_t size)
    {
        if (Size() < size)
            return false;
        start_pos_ += size;
        return true;
    }

    inline void Clear()
    {
        start_pos_ = 0;
        end_pos_ = 0;
    }

    virtual ~Buffer()
    {
        start_pos_ = 0;
        end_pos_ = 0;
    }

private:
    MMZBuffer mmz_buffer_;
    uint8_t *data_;
    uint32_t start_pos_;
    uint32_t end_pos_;
};

} // namespace rs
