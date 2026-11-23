#include <reflector.hpp>
#include <cctype>
#include <fstream>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

int main(int argc, char** argv)
{
	std::vector<std::string> expanded_args;
	expanded_args.reserve(argc > 1 ? argc - 1 : 0);

	for (int i = 1; i < argc; ++i)
	{
		std::string_view argument = argv[i];
		if (!argument.starts_with('@'))
		{
			expanded_args.emplace_back(argument);
			continue;
		}

		std::ifstream manifest(std::string(argument.substr(1)));
		
		if (!manifest)
		{
			std::cerr << "Failed to open manifest file: " << argument.substr(1) << '\n';
			return 1;
		}

		std::string line;
		while (std::getline(manifest, line))
		{
			auto first = line.begin();
			auto last  = line.end();
			while (first != last && std::isspace(static_cast<unsigned char>(*first)))
				++first;
			while (first != last && std::isspace(static_cast<unsigned char>(*(last - 1))))
				--last;

			if (first != last)
				expanded_args.emplace_back(first, last);
		}
	}

	std::vector<std::string_view> args;
	args.reserve(expanded_args.size());
	for (const std::string& argument : expanded_args)
		args.emplace_back(argument);

	return Reflector::Reflector::instance()->execute(args);
}
