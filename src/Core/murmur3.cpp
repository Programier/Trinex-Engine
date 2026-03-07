#include <Core/memory.hpp>

namespace Trinex
{
	static FORCE_INLINE u64 rotl64(u64 x, i8 r)
	{
		return (x << r) | (x >> (64 - r));
	}

	static FORCE_INLINE u64 getblock64(const u8* p)
	{
		u64 result = 0;
		result |= static_cast<u64>(p[0]) << 0;
		result |= static_cast<u64>(p[1]) << 8;
		result |= static_cast<u64>(p[2]) << 16;
		result |= static_cast<u64>(p[3]) << 24;
		result |= static_cast<u64>(p[4]) << 32;
		result |= static_cast<u64>(p[5]) << 40;
		result |= static_cast<u64>(p[6]) << 48;
		result |= static_cast<u64>(p[7]) << 56;
		return result;
	}

	static FORCE_INLINE u64 fmix64(u64 k)
	{
		k ^= k >> 33;
		k *= 0xff51afd7ed558ccdLLU;
		k ^= k >> 33;
		k *= 0xc4ceb9fe1a85ec53LLU;
		k ^= k >> 33;

		return k;
	}

	static FORCE_INLINE u128 murmur_hash(const u8* data, const usize size, u128 seed)
	{
		const usize nblocks = size / 16;

		u64 h1 = seed >> 64;
		u64 h2 = seed;

		const u64 c1 = 0x87c37b91114253d5LLU;
		const u64 c2 = 0x4cf5ad432745937fLLU;

		for (usize i = 0; i < nblocks; i++)
		{
			u64 k1 = getblock64(data);
			data += sizeof(u64);
			u64 k2 = getblock64(data);
			data += sizeof(u64);

			k1 *= c1;
			k1 = rotl64(k1, 31);
			k1 *= c2;
			h1 ^= k1;

			h1 = rotl64(h1, 27);
			h1 += h2;
			h1 = h1 * 5 + 0x52dce729;

			k2 *= c2;
			k2 = rotl64(k2, 33);
			k2 *= c1;
			h2 ^= k2;

			h2 = rotl64(h2, 31);
			h2 += h1;
			h2 = h2 * 5 + 0x38495ab5;
		}

		u64 k1 = 0;
		u64 k2 = 0;

		switch (size & 15)
		{
			case 15: k2 ^= ((u64) data[14]) << 48;
			case 14: k2 ^= ((u64) data[13]) << 40;
			case 13: k2 ^= ((u64) data[12]) << 32;
			case 12: k2 ^= ((u64) data[11]) << 24;
			case 11: k2 ^= ((u64) data[10]) << 16;
			case 10: k2 ^= ((u64) data[9]) << 8;
			case 9:
				k2 ^= ((u64) data[8]) << 0;
				k2 *= c2;
				k2 = rotl64(k2, 33);
				k2 *= c1;
				h2 ^= k2;

			case 8: k1 ^= ((u64) data[7]) << 56;
			case 7: k1 ^= ((u64) data[6]) << 48;
			case 6: k1 ^= ((u64) data[5]) << 40;
			case 5: k1 ^= ((u64) data[4]) << 32;
			case 4: k1 ^= ((u64) data[3]) << 24;
			case 3: k1 ^= ((u64) data[2]) << 16;
			case 2: k1 ^= ((u64) data[1]) << 8;
			case 1:
				k1 ^= ((u64) data[0]) << 0;
				k1 *= c1;
				k1 = rotl64(k1, 31);
				k1 *= c2;
				h1 ^= k1;
		};

		h1 ^= size;
		h2 ^= size;

		h1 += h2;
		h2 += h1;

		h1 = fmix64(h1);
		h2 = fmix64(h2);

		h1 += h2;
		h2 += h1;

		return (u128(h1) << 64) | h2;
	}

	ENGINE_EXPORT u128 memory_hash(const void* memory, const usize size, u128 seed)
	{
		return murmur_hash(static_cast<const u8*>(memory), size, seed);
	}
}// namespace Trinex
