#pragma once
#include <Core/engine_types.hpp>
#include <Core/etl/bit.hpp>
#include <Core/etl/flat_map.hpp>

namespace Trinex
{
	template<u32 bins = 32>
	class RangeAllocator
	{
	public:
		static_assert(bins > 0);

		struct Range {
			u32 offset = 0;
			u32 size   = 0;

			inline bool is_valid() const { return size != 0; }
			operator bool() const { return is_valid(); }
		};

	private:
		Vector<Range> m_bins[bins];
		FlatMap<u32, u32> m_free_chunks;

	private:
		static inline u32 bin_index(u32 count)
		{
			u32 index = etl::bit_width(count);
			return index < bins ? index : bins - 1;
		}

	public:
		Range alloc(u32 count)
		{
			if (count == 0)
				return {};

			for (u32 i = bin_index(count); i < bins; ++i)
			{
				while (!m_bins[i].empty())
				{
					Range block = m_bins[i].back();
					m_bins[i].pop_back();

					auto it = m_free_chunks.find(block.offset);
					if (it == m_free_chunks.end() || it->second != block.size)
					{
						continue;
					}

					m_free_chunks.erase(it);

					if (block.size > count)
					{
						Range leftover{block.offset + count, block.size - count};
						free(leftover);
					}

					return Range{block.offset, count};
				}
			}

			return {};
		}

		void free(Range range)
		{
			if (range.size == 0)
				return;

			auto next_it = m_free_chunks.lower_bound(range.offset);

			if (next_it != m_free_chunks.end() && next_it->first == range.offset + range.size)
			{
				range.size += next_it->second;
				m_free_chunks.erase(next_it);
				next_it = m_free_chunks.lower_bound(range.offset);
			}

			if (next_it != m_free_chunks.begin())
			{
				auto prev_it = std::prev(next_it);
				if (prev_it->first + prev_it->second == range.offset)
				{
					range.offset = prev_it->first;
					range.size += prev_it->second;
					m_free_chunks.erase(prev_it);
				}
			}

			m_free_chunks[range.offset] = range.size;
			m_bins[bin_index(range.size)].push_back(range);
		}
	};

}// namespace Trinex
