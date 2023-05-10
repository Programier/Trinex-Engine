#pragma once
#include <Core/string_functions.hpp>
#include <algorithm>
#include <cstdint>

#include <stdexcept>


#define UNIFORM_BLOCK_SIZE (1 << 23)

namespace Engine
{
    template<typename Type, std::size_t block_size>
    class BlockAllocator
    {
    private:
        using Offset = std::size_t;
        using Size   = std::size_t;

        struct MemoryFragment {
            Offset _M_offset = 0;
            Size _M_size     = 0;
        };

        struct AllocatedBlock {
            Type _M_block = nullptr;
            List<MemoryFragment> _M_memory_fragments;
        };

        List<AllocatedBlock> _M_blocks;
        Type (*_M_allocator)(std::size_t) = nullptr;

        BlockAllocator& create_new_block()
        {
            _M_blocks.emplace_back();
            _M_blocks.back()._M_block = _M_allocator(block_size);
            _M_blocks.back()._M_memory_fragments.push_back({0, block_size});
            return *this;
        }

    public:
        struct Block {
            Type _M_block         = nullptr;
            std::size_t _M_size   = 0;
            std::size_t _M_offset = 0;
        };

    public:
        BlockAllocator& allocator(Type (*function)(std::size_t))
        {
            _M_allocator = function;
            return *this;
        }

        Block allocate(std::size_t size)
        {
            if (size > block_size)
                throw std::runtime_error(Strings::format(
                        "Vulkan API: Failed to allocate memory: Max size of allocated memory is {} bytes", block_size));

            Block block;
            block._M_size = size;

            auto predicate = [&size](const MemoryFragment& fragment) -> bool { return fragment._M_size >= size; };

            for (auto& allocated_block : _M_blocks)
            {
                auto it = std::find_if(allocated_block._M_memory_fragments.begin(),
                                       allocated_block._M_memory_fragments.end(), predicate);

                if (it == allocated_block._M_memory_fragments.end())
                    continue;

                block._M_offset = it->_M_offset;
                block._M_block  = allocated_block._M_block;

                if (it->_M_size == size)
                {
                    allocated_block._M_memory_fragments.erase(it);
                }else
                {
                    it->_M_size -= size;
                    it->_M_offset += size;
                }

                return block;
            }

            create_new_block();

            block._M_offset = 0;
            block._M_block  = _M_blocks.back()._M_block;
            _M_blocks.back()._M_memory_fragments.back()._M_size -= size;
            _M_blocks.back()._M_memory_fragments.back()._M_offset += size;

            return block;
        }

        BlockAllocator& deallocate(const Block& block)
        {
            auto it = std::find_if(_M_blocks.begin(), _M_blocks.end(),
                                   [&block](const AllocatedBlock& a) -> bool { return a._M_block == block._M_block; });

            if (it == _M_blocks.end())
                return *this;

            auto predicate = [&block](const MemoryFragment& fragment) -> bool {
                return (fragment._M_size + fragment._M_offset == block._M_offset) ||
                       (fragment._M_offset == block._M_offset + block._M_size);
            };

            List<MemoryFragment>& fragments = (*it)._M_memory_fragments;

            auto node = std::find_if(fragments.begin(), fragments.end(), predicate);

            if (node == fragments.end())
            {
                fragments.push_back({block._M_offset, block._M_size});
                return *this;
            }

            MemoryFragment& node_element = *node;

            node_element._M_size += block._M_size;

            if (node_element._M_offset > block._M_offset)
                node_element._M_offset = block._M_offset;

            return *this;
        }

        BlockAllocator& destroy()
        {
            for (AllocatedBlock& block : _M_blocks) delete block._M_block;
            _M_blocks.clear();
            return *this;
        }

        ~BlockAllocator()
        {
            destroy();
        }
    };
}// namespace Engine
