#include <fstream>
#include <iostream>
#include <iterator>
#include <model.hpp>
#include <parser.hpp>
#include <printer.hpp>
#include <stdexcept>
#include <string>

namespace
{

	std::string read_file(const char* path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file)
		{
			throw std::runtime_error(std::string("failed to open file: ") + path);
		}

		return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
	}

}// namespace


static const char* static_source = R"(
trinex_class();
class Source
{
private:
	trinex_property();
	int value;

public:
	trinex_property();
	std::vector<int> value2;

	trinex_function();
	int method(int (Object::*method)(int a, int b) = nullptr) const { return func(1, 2); }
};
)";


int main(int argc, char** argv)
{
	Reflector::Parser parser;

	try
	{
		if (argc == 1)
		{
			Reflector::print(std::cout, parser.parse(static_source, "example.hpp::source"));
			return 0;
		}

		for (int i = 1; i < argc; ++i)
		{
			if (i > 1)
			{
				std::cout << '\n';
			}

			const auto input = read_file(argv[i]);
			Reflector::print(std::cout, parser.parse(input, argv[i]));
		}
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what() << '\n';
		return 1;
	}

	return 0;
}
