#include <Core/math/random.hpp>
#include <random>

namespace Engine::Random
{
	static std::mt19937_64 s_random(std::random_device{}());

	f32 floating(float min, float max)
	{
		std::uniform_real_distribution<f32> dist(min, max);
		return dist(s_random);
	}

	i32 integer(i32 min, i32 max)
	{
		std::uniform_int_distribution<i32> dist(min, max);
		return dist(s_random);
	}
}// namespace Engine::Random
