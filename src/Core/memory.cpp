#include <Core/etl/allocator.hpp>
#include <Core/memory.hpp>
#include <cstring>

namespace Engine
{
	// CRC64 hash function implementation
	static const HashIndex crc64_lookup_table[256] = {
	        0x0000000000000000ull, 0x42f0e1eba9ea3693ull, 0x85e1c3d753d46d26ull, 0xc711223cfa3e5bb5ull, 0x493366450e42ecdfull,
	        0x0bc387aea7a8da4cull, 0xccd2a5925d9681f9ull, 0x8e224479f47cb76aull, 0x9266cc8a1c85d9beull, 0xd0962d61b56fef2dull,
	        0x17870f5d4f51b498ull, 0x5577eeb6e6bb820bull, 0xdb55aacf12c73561ull, 0x99a54b24bb2d03f2ull, 0x5eb4691841135847ull,
	        0x1c4488f3e8f96ed4ull, 0x663d78ff90e185efull, 0x24cd9914390bb37cull, 0xe3dcbb28c335e8c9ull, 0xa12c5ac36adfde5aull,
	        0x2f0e1eba9ea36930ull, 0x6dfeff5137495fa3ull, 0xaaefdd6dcd770416ull, 0xe81f3c86649d3285ull, 0xf45bb4758c645c51ull,
	        0xb6ab559e258e6ac2ull, 0x71ba77a2dfb03177ull, 0x334a9649765a07e4ull, 0xbd68d2308226b08eull, 0xff9833db2bcc861dull,
	        0x388911e7d1f2dda8ull, 0x7a79f00c7818eb3bull, 0xcc7af1ff21c30bdeull, 0x8e8a101488293d4dull, 0x499b3228721766f8ull,
	        0x0b6bd3c3dbfd506bull, 0x854997ba2f81e701ull, 0xc7b97651866bd192ull, 0x00a8546d7c558a27ull, 0x4258b586d5bfbcb4ull,
	        0x5e1c3d753d46d260ull, 0x1cecdc9e94ace4f3ull, 0xdbfdfea26e92bf46ull, 0x990d1f49c77889d5ull, 0x172f5b3033043ebfull,
	        0x55dfbadb9aee082cull, 0x92ce98e760d05399ull, 0xd03e790cc93a650aull, 0xaa478900b1228e31ull, 0xe8b768eb18c8b8a2ull,
	        0x2fa64ad7e2f6e317ull, 0x6d56ab3c4b1cd584ull, 0xe374ef45bf6062eeull, 0xa1840eae168a547dull, 0x66952c92ecb40fc8ull,
	        0x2465cd79455e395bull, 0x3821458aada7578full, 0x7ad1a461044d611cull, 0xbdc0865dfe733aa9ull, 0xff3067b657990c3aull,
	        0x711223cfa3e5bb50ull, 0x33e2c2240a0f8dc3ull, 0xf4f3e018f031d676ull, 0xb60301f359dbe0e5ull, 0xda050215ea6c212full,
	        0x98f5e3fe438617bcull, 0x5fe4c1c2b9b84c09ull, 0x1d14202910527a9aull, 0x93366450e42ecdf0ull, 0xd1c685bb4dc4fb63ull,
	        0x16d7a787b7faa0d6ull, 0x5427466c1e109645ull, 0x4863ce9ff6e9f891ull, 0x0a932f745f03ce02ull, 0xcd820d48a53d95b7ull,
	        0x8f72eca30cd7a324ull, 0x0150a8daf8ab144eull, 0x43a04931514122ddull, 0x84b16b0dab7f7968ull, 0xc6418ae602954ffbull,
	        0xbc387aea7a8da4c0ull, 0xfec89b01d3679253ull, 0x39d9b93d2959c9e6ull, 0x7b2958d680b3ff75ull, 0xf50b1caf74cf481full,
	        0xb7fbfd44dd257e8cull, 0x70eadf78271b2539ull, 0x321a3e938ef113aaull, 0x2e5eb66066087d7eull, 0x6cae578bcfe24bedull,
	        0xabbf75b735dc1058ull, 0xe94f945c9c3626cbull, 0x676dd025684a91a1ull, 0x259d31cec1a0a732ull, 0xe28c13f23b9efc87ull,
	        0xa07cf2199274ca14ull, 0x167ff3eacbaf2af1ull, 0x548f120162451c62ull, 0x939e303d987b47d7ull, 0xd16ed1d631917144ull,
	        0x5f4c95afc5edc62eull, 0x1dbc74446c07f0bdull, 0xdaad56789639ab08ull, 0x985db7933fd39d9bull, 0x84193f60d72af34full,
	        0xc6e9de8b7ec0c5dcull, 0x01f8fcb784fe9e69ull, 0x43081d5c2d14a8faull, 0xcd2a5925d9681f90ull, 0x8fdab8ce70822903ull,
	        0x48cb9af28abc72b6ull, 0x0a3b7b1923564425ull, 0x70428b155b4eaf1eull, 0x32b26afef2a4998dull, 0xf5a348c2089ac238ull,
	        0xb753a929a170f4abull, 0x3971ed50550c43c1ull, 0x7b810cbbfce67552ull, 0xbc902e8706d82ee7ull, 0xfe60cf6caf321874ull,
	        0xe224479f47cb76a0ull, 0xa0d4a674ee214033ull, 0x67c58448141f1b86ull, 0x253565a3bdf52d15ull, 0xab1721da49899a7full,
	        0xe9e7c031e063acecull, 0x2ef6e20d1a5df759ull, 0x6c0603e6b3b7c1caull, 0xf6fae5c07d3274cdull, 0xb40a042bd4d8425eull,
	        0x731b26172ee619ebull, 0x31ebc7fc870c2f78ull, 0xbfc9838573709812ull, 0xfd39626eda9aae81ull, 0x3a28405220a4f534ull,
	        0x78d8a1b9894ec3a7ull, 0x649c294a61b7ad73ull, 0x266cc8a1c85d9be0ull, 0xe17dea9d3263c055ull, 0xa38d0b769b89f6c6ull,
	        0x2daf4f0f6ff541acull, 0x6f5faee4c61f773full, 0xa84e8cd83c212c8aull, 0xeabe6d3395cb1a19ull, 0x90c79d3fedd3f122ull,
	        0xd2377cd44439c7b1ull, 0x15265ee8be079c04ull, 0x57d6bf0317edaa97ull, 0xd9f4fb7ae3911dfdull, 0x9b041a914a7b2b6eull,
	        0x5c1538adb04570dbull, 0x1ee5d94619af4648ull, 0x02a151b5f156289cull, 0x4051b05e58bc1e0full, 0x87409262a28245baull,
	        0xc5b073890b687329ull, 0x4b9237f0ff14c443ull, 0x0962d61b56fef2d0ull, 0xce73f427acc0a965ull, 0x8c8315cc052a9ff6ull,
	        0x3a80143f5cf17f13ull, 0x7870f5d4f51b4980ull, 0xbf61d7e80f251235ull, 0xfd913603a6cf24a6ull, 0x73b3727a52b393ccull,
	        0x31439391fb59a55full, 0xf652b1ad0167feeaull, 0xb4a25046a88dc879ull, 0xa8e6d8b54074a6adull, 0xea16395ee99e903eull,
	        0x2d071b6213a0cb8bull, 0x6ff7fa89ba4afd18ull, 0xe1d5bef04e364a72ull, 0xa3255f1be7dc7ce1ull, 0x64347d271de22754ull,
	        0x26c49cccb40811c7ull, 0x5cbd6cc0cc10fafcull, 0x1e4d8d2b65facc6full, 0xd95caf179fc497daull, 0x9bac4efc362ea149ull,
	        0x158e0a85c2521623ull, 0x577eeb6e6bb820b0ull, 0x906fc95291867b05ull, 0xd29f28b9386c4d96ull, 0xcedba04ad0952342ull,
	        0x8c2b41a1797f15d1ull, 0x4b3a639d83414e64ull, 0x09ca82762aab78f7ull, 0x87e8c60fded7cf9dull, 0xc51827e4773df90eull,
	        0x020905d88d03a2bbull, 0x40f9e43324e99428ull, 0x2cffe7d5975e55e2ull, 0x6e0f063e3eb46371ull, 0xa91e2402c48a38c4ull,
	        0xebeec5e96d600e57ull, 0x65cc8190991cb93dull, 0x273c607b30f68faeull, 0xe02d4247cac8d41bull, 0xa2dda3ac6322e288ull,
	        0xbe992b5f8bdb8c5cull, 0xfc69cab42231bacfull, 0x3b78e888d80fe17aull, 0x7988096371e5d7e9ull, 0xf7aa4d1a85996083ull,
	        0xb55aacf12c735610ull, 0x724b8ecdd64d0da5ull, 0x30bb6f267fa73b36ull, 0x4ac29f2a07bfd00dull, 0x08327ec1ae55e69eull,
	        0xcf235cfd546bbd2bull, 0x8dd3bd16fd818bb8ull, 0x03f1f96f09fd3cd2ull, 0x41011884a0170a41ull, 0x86103ab85a2951f4ull,
	        0xc4e0db53f3c36767ull, 0xd8a453a01b3a09b3ull, 0x9a54b24bb2d03f20ull, 0x5d45907748ee6495ull, 0x1fb5719ce1045206ull,
	        0x919735e51578e56cull, 0xd367d40ebc92d3ffull, 0x1476f63246ac884aull, 0x568617d9ef46bed9ull, 0xe085162ab69d5e3cull,
	        0xa275f7c11f7768afull, 0x6564d5fde549331aull, 0x279434164ca30589ull, 0xa9b6706fb8dfb2e3ull, 0xeb46918411358470ull,
	        0x2c57b3b8eb0bdfc5ull, 0x6ea7525342e1e956ull, 0x72e3daa0aa188782ull, 0x30133b4b03f2b111ull, 0xf7021977f9cceaa4ull,
	        0xb5f2f89c5026dc37ull, 0x3bd0bce5a45a6b5dull, 0x79205d0e0db05dceull, 0xbe317f32f78e067bull, 0xfcc19ed95e6430e8ull,
	        0x86b86ed5267cdbd3ull, 0xc4488f3e8f96ed40ull, 0x0359ad0275a8b6f5ull, 0x41a94ce9dc428066ull, 0xcf8b0890283e370cull,
	        0x8d7be97b81d4019full, 0x4a6acb477bea5a2aull, 0x089a2aacd2006cb9ull, 0x14dea25f3af9026dull, 0x562e43b4931334feull,
	        0x913f6188692d6f4bull, 0xd3cf8063c0c759d8ull, 0x5dedc41a34bbeeb2ull, 0x1f1d25f19d51d821ull, 0xd80c07cd676f8394ull,
	        0x9afce626ce85b507ull};


	ENGINE_EXPORT byte* allocate_memory(size_t size, size_t alignment)
	{
		return ByteAllocator().allocate_aligned(size, alignment);
	}

	ENGINE_EXPORT byte* allocate_memory(size_t size, size_t alignment, const void* src)
	{
		byte* memory = allocate_memory(size, alignment);
		if (src)
			std::memcpy(memory, src, size);
		return memory;
	}

	ENGINE_EXPORT void release_memory(void* ptr)
	{
		return ByteAllocator().deallocate(static_cast<ByteAllocator::pointer>(ptr));
	}

	ENGINE_EXPORT HashIndex memory_hash_fast(const void* memory, const size_t size, HashIndex start_hash)
	{
		const byte* data = reinterpret_cast<const byte*>(memory);

		start_hash = ~start_hash;

		for (size_t index = 0; index < size; ++index)
		{
			start_hash = crc64_lookup_table[static_cast<byte>(start_hash >> 56) ^ (*data)] ^ (start_hash << 8);
			++data;
		}
		return ~start_hash;
	}

	ENGINE_EXPORT const byte* memory_search(const byte* haystack, size_t haystack_len, const byte* needle, size_t needle_len)
	{
		if (needle_len > haystack_len)
		{
			return nullptr;
		}

		if (needle_len == 0)
			return haystack;

		for (size_t i = 0, count = haystack_len - needle_len; i <= count; ++i)
		{
			if (memcmp(haystack + i, needle, needle_len) == 0)
			{
				return haystack + i;
			}
		}

		return nullptr;
	}
}// namespace Engine
