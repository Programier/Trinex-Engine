#include <reflector.hpp>

#include <span>
#include <string_view>
#include <vector>

int main(int argc, char** argv)
{
	std::vector<std::string_view> args;
	args.reserve(argc > 1 ? argc - 1 : 0);

	for (int i = 1; i < argc; ++i)
	{
		args.emplace_back(argv[i]);
	}

	return Reflector::Reflector::instance()->execute(args);
}
