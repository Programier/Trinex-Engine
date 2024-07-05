#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

#define error_log(...)                                                                                                           \
    {                                                                                                                            \
        fprintf(stderr, __VA_ARGS__);                                                                                            \
        return -1;                                                                                                               \
    }

static void begin_header(std::ofstream& file)
{
    file << "#pragma once\n\nnamespace Engine\n{\n";
}

static void end_header(std::ofstream& file)
{
    file << "}\n";
}

static void create_header(std::ofstream& file, std::string_view resource, const std::vector<char>& buffer)
{
    size_t size = buffer.size();

    file << "\n\tconstexpr unsigned char " << resource << "_data[] = {";

    char hex[] = "0x0000";

    for (size_t i = 0; i < size; i++)
    {
        if (i % 10 == 0)
            file << "\n\t\t";

        sprintf(hex, "0x%02X", static_cast<unsigned char>(buffer[i]));
        file << hex << (size - 1 == i ? "" : ", ");
    }

    file << "};\n";

    file << "\n\n\tconstexpr unsigned long long int " << resource << "_len = " << size << ";\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        error_log("Header Generator:\n Usage:\n header_generator <header_path> <resource_files...>");
    }

    const char* header_path = argv[1];
    std::vector<std::string> files;

    for (int i = 2; i < argc; ++i)
    {
        files.push_back(argv[i]);
    }

    std::ofstream output(header_path, std::ios_base::out);

    begin_header(output);
    for (auto& file_path : files)
    {
        std::ifstream input(file_path, std::ios_base::in | std::ios_base::binary);

        if (!output.is_open())
            error_log("Failed to open output file!");

        std::string resource_name = std::filesystem::path(file_path).filename().stem().string();

        if (input.is_open())
        {

            std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
            create_header(output, resource_name, buffer);
        }
        else
        {
            printf("Failed to open input: %s\n\n", file_path.c_str());
            create_header(output, resource_name, {});
        }
    }

    end_header(output);
}
