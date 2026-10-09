#include <hash.hpp>

namespace Reflector
{
	static inline std::uint64_t rotl64(std::uint64_t x, std::int8_t r)
	{
		return (x << r) | (x >> (64 - r));
	}

	static inline std::uint64_t getblock64(const std::uint8_t* p)
	{
		std::uint64_t result = 0;
		result |= static_cast<std::uint64_t>(p[0]) << 0;
		result |= static_cast<std::uint64_t>(p[1]) << 8;
		result |= static_cast<std::uint64_t>(p[2]) << 16;
		result |= static_cast<std::uint64_t>(p[3]) << 24;
		result |= static_cast<std::uint64_t>(p[4]) << 32;
		result |= static_cast<std::uint64_t>(p[5]) << 40;
		result |= static_cast<std::uint64_t>(p[6]) << 48;
		result |= static_cast<std::uint64_t>(p[7]) << 56;
		return result;
	}

	static inline std::uint64_t fmix64(std::uint64_t k)
	{
		k ^= k >> 33;
		k *= 0xff51afd7ed558ccdLLU;
		k ^= k >> 33;
		k *= 0xc4ceb9fe1a85ec53LLU;
		k ^= k >> 33;

		return k;
	}

	static inline hash_t murmur_hash(const std::uint8_t* data, const std::size_t size, hash_t seed)
	{
		const std::size_t nblocks = size / 16;

		std::uint64_t h1 = seed.high;
		std::uint64_t h2 = seed.low;

		const std::uint64_t c1 = 0x87c37b91114253d5LLU;
		const std::uint64_t c2 = 0x4cf5ad432745937fLLU;

		for (std::size_t i = 0; i < nblocks; i++)
		{
			std::uint64_t k1 = getblock64(data);
			data += sizeof(std::uint64_t);
			std::uint64_t k2 = getblock64(data);
			data += sizeof(std::uint64_t);

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

		std::uint64_t k1 = 0;
		std::uint64_t k2 = 0;

		switch (size & 15)
		{
			case 15: k2 ^= ((std::uint64_t) data[14]) << 48;
			case 14: k2 ^= ((std::uint64_t) data[13]) << 40;
			case 13: k2 ^= ((std::uint64_t) data[12]) << 32;
			case 12: k2 ^= ((std::uint64_t) data[11]) << 24;
			case 11: k2 ^= ((std::uint64_t) data[10]) << 16;
			case 10: k2 ^= ((std::uint64_t) data[9]) << 8;
			case 9:
				k2 ^= ((std::uint64_t) data[8]) << 0;
				k2 *= c2;
				k2 = rotl64(k2, 33);
				k2 *= c1;
				h2 ^= k2;

			case 8: k1 ^= ((std::uint64_t) data[7]) << 56;
			case 7: k1 ^= ((std::uint64_t) data[6]) << 48;
			case 6: k1 ^= ((std::uint64_t) data[5]) << 40;
			case 5: k1 ^= ((std::uint64_t) data[4]) << 32;
			case 4: k1 ^= ((std::uint64_t) data[3]) << 24;
			case 3: k1 ^= ((std::uint64_t) data[2]) << 16;
			case 2: k1 ^= ((std::uint64_t) data[1]) << 8;
			case 1:
				k1 ^= ((std::uint64_t) data[0]) << 0;
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

		return {h1, h2};
	}

	hash_t memory_hash(const void* memory, const std::size_t size, hash_t seed)
	{
		return murmur_hash(static_cast<const std::uint8_t*>(memory), size, seed);
	}
}// namespace Reflector
