#include <Core/math/random.hpp>
#include <random>

namespace Engine::Random
{
	static std::mt19937_64 s_random(std::random_device{}());

	float_t floating(float min, float max)
	{
		std::uniform_real_distribution<float_t> dist(min, max);
		return dist(s_random);
	}

	int_t integer(int_t min, int_t max)
	{
		std::uniform_int_distribution<int_t> dist(min, max);
		return dist(s_random);
	}
}// namespace Engine::Random
