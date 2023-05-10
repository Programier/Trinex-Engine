#include <iostream>

#include <stdexcept>

static std::string input_file;
static std::string output_file;

// function(int argc, int param_index, char** argv)

#define args_func(param_name) static int parse_##param_name##_parameter(int argc, int index, char** args)

args_func(i)
{
    return index;
}

args_func(o)
{
    return index;
}

args_func(h)
{
    std::clog << "USAGE:\n\t./MeshGenerator -i <input_file_path> -o <output_file_path>" << std::endl;
    exit(0);
    return index;
}

static Map<std::string, int (*)(int, int, char**)> args_parser_map = {
        {"-i", parse_i_parameter},
        {"-o", parse_o_parameter},
        {"-h", parse_h_parameter},
};

static void parse_args(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        auto it = args_parser_map.find(argv[i]);
        if (it != args_parser_map.end())
        {
            i = it->second(argc, i, argv);
        }
    }

    if (input_file.empty())
    {
        throw std::runtime_error("MeshGenerator: Input file not found! Use -h parameter for show usage info!");
    }

    if (output_file.empty())
    {
        throw std::runtime_error("MeshGenerator: Output file not found! Use -h parameter for show usage info!");
    }
}


int main(int argc, char** argv)
try
{
    parse_args(argc, argv);
}
catch (const std::exception& e)
{
    std::clog << "CRITICAL ERROR:\n\t" << e.what() << std::endl;
    return 1;
}
