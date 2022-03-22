#include "obj_loader.hpp"
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <unordered_map>

#define MESH mesh.data()
#define INT_INDEX static_cast<int>(texture_index + 0.5)

typedef std::vector<std::string> split_result;

typedef std::unordered_map<std::string, std::string> mtl_data;

namespace Engine
{
    static std::string& normalize(std::string& path)
    {
        while (!path.empty() && (path.back() == ' ' || path.back() == '\r' || path.back() == '\n'))
        {
            path.pop_back();
        }
        int index = 0;
        while (!path.empty() && path[index] == ' ') index++;
        path = path.substr(index, path.length() - index);
        return path;
    }

    split_result split(const std::string& line, const char& spliter, int max_blocks = -1,
                       const bool& remove_empty = true)
    {
        split_result result;
        std::size_t begin = 0, end = 0, size = line.length();
        while (begin < size && static_cast<int>(result.size()) != max_blocks - 1)
        {
            while (end < size && line[end] != spliter) end++;

            if (begin != end || remove_empty == false)
                result.push_back(line.substr(begin, end - begin));
            begin = end + 1;
            end++;
        }

        if (begin < size)
            result.push_back(line.substr(begin, size - begin));

        return result;
    }

    static void load_mtl(const std::string& mtl_file, mtl_data& data, const std::string& path)
    {
        data.clear();
        std::ifstream mtl(mtl_file);
        std::clog << "Mtl loader: Start load " << mtl_file << std::endl;
        if (!mtl.is_open())
        {
            std::cerr << "Mtl loader: Failed to open '" << mtl_file << "'  mtl file" << std::endl;
            return;
        }
        std::string current_line;

        while (getline(mtl, current_line))
        {
            auto result = split(current_line, ' ', 2);
            if (result.size() < 2)
                continue;
            if (result[0] != "newmtl")
                continue;

            std::string group = normalize(result[1]);
            while (getline(mtl, current_line))
            {
                result = split(current_line, ' ', 2);
                if (result.size() < 2)
                    continue;
                if (result[0] != "map_Kd")
                    continue;
                std::string res = path + normalize(result[1]);
                if (std::ifstream(res, std::ios_base::binary).is_open())
                    data[group] = res;
                break;
            }
        }
    }

    static std::string get_path(std::string file)
    {
        int index = static_cast<int>(file.length() - 1);
        while (index > 0 && file[index] != '\\' && file[index] != '/') index--;
        return file.substr(0, index + 1);
        ;
    }


    void load_obj(TextureArray& texture, Mesh& mesh, const std::string& filename, bool invert)
    {

        std::ifstream obj(filename);
        if (!obj.is_open())
            return;

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec2> texture_vertices;

        // [array of [vertices index : texture vertices index] : address to name of texture]
        std::vector<std::pair<std::vector<std::pair<int, int>>, std::string*>> poligones;

        std::string current_line;
        current_line.reserve(255);
        mtl_data mtl;
        std::string* current_mtl = nullptr;

        std::size_t mesh_data_values = 0;

        while (getline(obj, current_line))
        {
            auto values = split(current_line, ' ', 2);
            if (values.size() == 0)
                continue;
            auto& value = values[0];

            // Comment
            if (value == "#")
                continue;

            // Mtl file loading
            if (value == "mtllib")
            {
                std::clog << "Obj loader: Start load mtl file" << std::endl;
                if (values.size() > 1)
                {
                    value = values[1];
                    if (value.back() == '\n' || value.back() == '\r')
                        value.pop_back();
                    auto p = get_path(filename);
                    value = p + value;
                    load_mtl(value, mtl, p);
                    std::clog << "Mtl loader: Loaded " << mtl.size() << " mtls" << std::endl;
                }
                else
                    std::cerr << "Obj loader: Failed to load mtl file" << std::endl;
                continue;
            }

            // Vertices loading
            if (value == "v")
            {
                if (values.size() > 1)
                    values = split(values[1], ' ', -1, true);
                std::size_t count = values.size();
                vertices.emplace_back();

                for (std::size_t i = 0; i < 3 && i < count; i++)
                {
                    vertices.back()[i] = std::stof(values[i]);
                }
                continue;
            }

            // Texture vertices loading

            if (value == "vt")
            {
                if (values.size() > 1)
                    values = split(values[1], ' ', -1, true);
                std::size_t count = values.size();
                texture_vertices.emplace_back();
                for (std::size_t i = 0; i < 2 && i < count; i++)
                {
                    texture_vertices.back()[i] = std::stof(values[i]);
                }

                continue;
            }

            if (value == "usemtl")
            {
                if (values.size() < 2)
                {
                    std::cerr << "Obj loader: Failed to set mtl" << std::endl;
                }
                else
                {
                    current_mtl = &mtl[normalize(values[1])];
                }
                continue;
            }

            if (value == "f")
            {
                values = split(current_line, ' ', -1, true);
                std::size_t size = values.size();
                poligones.emplace_back();
                // Note
                // [array of [vertices index : texture vertices index] : address to name of texture]
                auto& poligon = poligones.back();
                poligon.second = current_mtl;
                auto& vector = poligon.first;

                for (std::size_t i = 1; i < size; i++)
                {
                    vector.emplace_back();
                    auto indexes = split(values[i], '/', -1, true);
                    if (indexes.size() < 2)
                        continue;
                    auto s = indexes.size();
                    if (s > 0)
                        vector.back().first = std::stoi(indexes[0]) - 1;
                    if (s > 1)
                        vector.back().second = std::stoi(indexes[1]) - 1;
                    mesh_data_values++;
                }
                continue;
            }
        }

        std::clog << "Obj loader: Loaded " << vertices.size() << " vertices" << std::endl;

        std::clog << "Obj loader: Loaded " << texture_vertices.size() << " texture vertices"
                  << std::endl;
        std::clog << "Obj loader: Loaded " << poligones.size() << " poligones" << std::endl;

        // Creating TextureArray
        std::clog << "Obj loader: Creating texture array" << std::endl;
        std::vector<std::string> filenames;
        std::unordered_map<std::string, std::size_t> texture_map;

        std::size_t index = 0;
        for (auto& ell : mtl)
        {
            if (ell.second != "")
            {
                filenames.push_back(ell.second);
                texture_map[ell.second] = index++;
            }
        }

        texture.load(filenames, Engine::LINEAR, invert);

        // Generating mesh
        std::clog << "Obj loader: Generating mesh" << std::endl;
        mesh.attributes({3, 3, 2});

        // Generating data
        mesh.data().clear();
        mesh.data().reserve(mesh_data_values * 8);

        // Note
        // [array of [vertices index : texture vertices index] : address to name of texture]

        for (auto& poligon : poligones)
        {
            auto texture_index =
                    poligon.second ? static_cast<float>(texture_map[*poligon.second]) : 0;

            float w = texture.images()[INT_INDEX].width();
            float h = texture.images()[INT_INDEX].height();
            auto& vector = poligon.first;
            auto& first_point = vector.front();


            auto vector_size = vector.size() - 1;
            for (std::size_t i = 1; i < vector_size; i++)
            {
                // FIRST POINT
                // Push coords
                MESH.push_back(vertices[first_point.first].x);
                MESH.push_back(vertices[first_point.first].y);
                MESH.push_back(vertices[first_point.first].z);

                // Push texture coors
                MESH.push_back(texture_vertices[first_point.second].x);
                MESH.push_back(texture_vertices[first_point.second].y);

                // Push layer
                MESH.push_back(texture_index);
                MESH.push_back(w);
                MESH.push_back(h);

                // SECOND POINT

                // Push coords
                MESH.push_back(vertices[vector[i].first].x);
                MESH.push_back(vertices[vector[i].first].y);
                MESH.push_back(vertices[vector[i].first].z);

                // Push texture coors
                MESH.push_back(texture_vertices[vector[i].second].x);
                MESH.push_back(texture_vertices[vector[i].second].y);

                // Push layer
                MESH.push_back(texture_index);
                MESH.push_back(w);
                MESH.push_back(h);

                // THIRD POINT

                // Push coords
                MESH.push_back(vertices[vector[i + 1].first].x);
                MESH.push_back(vertices[vector[i + 1].first].y);
                MESH.push_back(vertices[vector[i + 1].first].z);

                // Push texture coors
                MESH.push_back(texture_vertices[vector[i + 1].second].x);
                MESH.push_back(texture_vertices[vector[i + 1].second].y);

                // Push layer
                MESH.push_back(texture_index);
                MESH.push_back(w);
                MESH.push_back(h);
            }
        }

        // Calculating vertices count
        mesh.vertices_count(mesh.data().size() / 8);
        mesh.update_buffers();
        std::clog << "Obj loader: Mesh data size: " << MESH.size() << std::endl;
    }
}// namespace Engine
