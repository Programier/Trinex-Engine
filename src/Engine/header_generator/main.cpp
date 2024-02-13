#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

#define error_log(...)                                                                                                           \
    {                                                                                                                            \
        fprintf(stderr, __VA_ARGS__);                                                                                            \
        return -1;                                                                                                               \
    }


static void create_header(std::ofstream& file, std::string_view resource, const std::vector<char>& buffer)
{
    file << "#pragma once\n\nnamespace Engine\n{\n\tconstexpr unsigned char " << resource << "_data[] = {";

    size_t size = buffer.size();

    char hex[] = "0x0000";

    for (size_t i = 0; i < size; i++)
    {
        if (i % 10 == 0)
            file << "\n\t\t";

        sprintf(hex, "0x%02X", static_cast<unsigned char>(buffer[i]));
        file << hex << (size - 1 == i ? "};\n" : ", ");
    }

    file << "\n\n\tconstexpr unsigned long long int " << resource << "_len = " << size << ";\n}\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        error_log("Header Generator:\n Usage:\n header_generator <file_path> <header_path> <resource_name>");
    }

    const char* file_path     = argv[1];
    const char* header_path   = argv[2];
    const char* resource_name = argv[3];

    std::ifstream input(file_path, std::ios_base::in | std::ios_base::binary);
    std::ofstream output(header_path, std::ios_base::out);

    if (!output.is_open())
        error_log("Failed to open output file!");

    if (input.is_open())
    {
        std::vector<char> buffer(std::istreambuf_iterator<char>(input), {});
        create_header(output, resource_name, buffer);
    }
    else
    {
        create_header(output, resource_name, {});
    }
}
