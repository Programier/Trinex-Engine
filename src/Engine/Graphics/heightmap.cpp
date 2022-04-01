#include <Graphics/heightmap.hpp>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <thread>

#define calculate_count(min, max, block_size) (static_cast<std::size_t>(((max - min) / block_size)) + 1)
#define to_array_index(min, value, block_size) (static_cast<std::size_t>((value - min) / block_size))
#define from_index(index, min, block_size) ((static_cast<float>(index) * block_size) + min)
#define minor(a, b, c, d) (((a) * (d)) - ((b) * (c)))
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a < b ? b : a)
#define in_range(value, min_value, max_value) (value >= min_value && value <= max_value)
#define use_model(variable) (glm::vec3(model_matrix * glm::vec4((variable), 1.0f)))
#define get_normal (glm::vec3(plane))

#define point_to_height_point(point)                                                                                   \
    {                                                                                                                  \
        to_array_index(_M_limits.min.x, point.x, block_size), to_array_index(_M_limits.min.y, point.y, block_size),    \
                to_array_index(_M_limits.min.z, point.z, block_size), point, normal                                    \
    }

#define height_map_value                                                                                               \
    {                                                                                                                  \
        ~Engine::ArrayIndex(0), ~Engine::ArrayIndex(0), ~Engine::ArrayIndex(0), _M_limits.min,                         \
        {                                                                                                              \
            0.f, 0.f, 0.f                                                                                              \
        }


const unsigned int processor_count = std::thread::hardware_concurrency();


struct line {
    glm::vec3 begin;
    glm::vec3 end;
};

using HeightPoint = Engine::HeightMapValue;

#define vec_mult(a, b)                                                                                                 \
    glm::vec3((a).y*(b).z - (a).z * (b).y, (a).z * (b).x - (a).x * (b).z, (a).x * (b).y - (a).y * (b).x)


static bool in_triangle(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C, const glm::vec3& P)
{
    auto AB = B - A;
    auto AP = A - P;
    auto BC = C - B;
    auto BP = B - P;
    auto CA = A - P;
    auto CP = C - P;
    auto res1 = vec_mult(AP, AB).z;
    auto res2 = vec_mult(BP, BC).z;
    auto res3 = vec_mult(CP, CA).z;

    bool positive = res1 >= 0;
    if ((positive != (res2 >= 0)) || (positive != (res3 >= 0)))
        return false;
    return true;
}


#undef check_sort


static void calculate_height(std::vector<HeightPoint>& output, const float* values, std::size_t size,
                             std::size_t triangle_point_size, float block_size, Engine::Model::Limits* limits,
                             float local_block_size, const glm::mat4& model_matrix)
{
    auto& _M_limits = *limits;
    float triangle_block = triangle_point_size * 3;
    for (std::size_t i = 0; i < size; i += triangle_block)
    {
        // Calculation of the formula of the triangle plane
        glm::vec3 a(values[i], values[i + 1], values[i + 2]);
        glm::vec3 b(values[i + triangle_point_size], values[i + 1 + triangle_point_size],
                    values[i + 2 + triangle_point_size]);
        glm::vec3 c(values[i + (2 * triangle_point_size)], values[i + 1 + (2 * triangle_point_size)],
                    values[i + 2 + (2 * triangle_point_size)]);

        a = use_model(a);
        b = use_model(b);
        c = use_model(c);
        glm::vec4 plane = {minor(b.y - a.y, c.y - a.y, b.z - a.z, c.z - a.z),
                           minor(b.x - a.x, c.x - a.x, b.z - a.z, c.z - a.z) * (-1),
                           minor(b.x - a.x, c.x - a.x, b.y - a.y, c.y - a.y), 0};
        plane[3] = (-a.x) * plane[0] + (-a.y) * plane[1] + (-a.z) * plane[2];


        glm::vec2 A(a.x, a.z);
        glm::vec2 B(b.x, b.z);
        glm::vec2 C(c.x, c.z);
        auto normal = get_normal;

        output.push_back(point_to_height_point(a));
        output.push_back(point_to_height_point(b));
        output.push_back(point_to_height_point(c));


        for (const auto& line : {line({a, b}), line({a, c}), line({b, c})})
        {
            auto v = line.end - line.begin;
            if (v.x == 0 && v.z == 0)
                continue;
            int use_coord = -1;

            if (v.x == 0 || v.z == 0)
                use_coord = v.x == 0 ? 2 : 0;
            else
                use_coord = glm::abs(v.x) > glm::abs(v.z) ? 0 : 2;

            float end = max(line.begin[use_coord], line.end[use_coord]);
            for (float start = min(line.begin[use_coord], line.end[use_coord]); start < end; start += local_block_size)
            {
                float t = (start - line.begin[use_coord]) / v[use_coord];
                float x_value = (v.x * t) + line.begin.x;
                float y_value = (v.y * t) + line.begin.y;
                y_value = y_value > _M_limits.max.y ? _M_limits.max.y : y_value;
                y_value = y_value < _M_limits.min.y ? _M_limits.min.y : y_value;
                float z_value = (v.z * t) + line.begin.z;

                std::size_t x = to_array_index(_M_limits.min.x, x_value, block_size);
                std::size_t y = to_array_index(_M_limits.min.y, y_value, block_size);
                std::size_t z = to_array_index(_M_limits.min.z, z_value, block_size);
                output.push_back({x, y, z, {x_value, y_value, z_value}, normal});
            }
        }

        float max_x = max(max(a.x, b.x), c.x);
        float max_z = max(max(a.z, b.z), c.z);

        float min_x = from_index(to_array_index(_M_limits.min.x, min(min(a.x, b.x), c.x), block_size), _M_limits.min.x,
                                 block_size);

        float min_z = from_index(to_array_index(_M_limits.min.z, min(min(a.z, b.z), c.z), block_size), _M_limits.min.z,
                                 block_size);

        for (float x = min_x; x < max_x; x += local_block_size)
        {
            for (float z = min_z; z < max_z; z += local_block_size)
            {

                glm::vec2 P(x, z);
                if (!in_triangle(glm::vec3(A, 1.f), glm::vec3(B, 1.0f), glm::vec3(C, 1.0f), glm::vec3(P, 1.0f)))
                    continue;

                std::size_t x_coord = to_array_index(_M_limits.min.x, x, block_size);
                std::size_t z_coord = to_array_index(_M_limits.min.z, z, block_size);

                if (plane[1] == 0)
                {
                    // TODO
                }
                else
                {
                    float y_value = (-plane[3] - (plane[0] * x) - (plane[2] * z)) / plane[1];
                    y_value = y_value > _M_limits.max.y ? _M_limits.max.y : y_value;
                    y_value = y_value < _M_limits.min.y ? _M_limits.min.y : y_value;
                    std::size_t y_coord = to_array_index(_M_limits.min.y, y_value, block_size);
                    output.push_back({x_coord, y_coord, z_coord, {x, y_value, z}, normal});
                }
            }
        }
    }
}

namespace Engine
{
    bool HeightMapValue::operator<(const HeightMapValue& v) const
    {
        if (position.x < v.position.x)
            return true;
        else if (position.x > v.position.x)
            return false;

        if (position.y < v.position.y)
            return true;
        else if (position.y > v.position.y)
            return false;

        if (position.z < v.position.z)
            return true;
        else if (position.z > v.position.z)
            return false;
        return false;
    }

    bool HeightMapValue::operator<=(const HeightMapValue& v) const
    {
        return (*this < v) || (*this == v);
    }

    bool HeightMapValue::operator>(const HeightMapValue& v) const
    {
        return (!(*this < v)) && (*this != v);
    }

    bool HeightMapValue::operator>=(const HeightMapValue& v) const
    {
        return !(*this < v);
    }

    bool HeightMapValue::operator==(const HeightMapValue& v) const
    {
        return position.x == v.position.x && position.y == v.position.y && position.z == v.position.z;
    }

    bool HeightMapValue::operator!=(const HeightMapValue& v) const
    {
        return !(*this == v);
    }

    HeightMap::HeightMap() = default;

    HeightMap::HeightMap(const Model& model, const float& block_size, const glm::mat4& model_matrix)
    {
        from_model(model, block_size, model_matrix);
    }

    HeightMap& HeightMap::from_model(const Model& model, const float& block_size, const glm::mat4& model_matrix)
    {
        std::clog << "HeightMap: Generating height map" << std::endl;
        _M_array.clear();
        _M_limits = model.limits();

        _M_limits.max = use_model(_M_limits.max);
        _M_limits.min = use_model(_M_limits.min);

        for (int i = 0; i < 3; i++)
            if (_M_limits.min[i] > _M_limits.max[i])
                std::swap(_M_limits.min[i], _M_limits.max[i]);

        _M_block_size = block_size;

        // Resizing array
        {
            std::size_t x_count = calculate_count(_M_limits.min.x, _M_limits.max.x, block_size) + 1;
            std::size_t y_count = calculate_count(_M_limits.min.y, _M_limits.max.y, block_size) + 1;
            std::size_t z_count = calculate_count(_M_limits.min.z, _M_limits.max.z, block_size) + 1;

            _M_array.resize(x_count, HeightMap_Y_Axis(y_count, HeightMap_Z_Axis(z_count, {
                height_map_value}
        })));
    }


    std::size_t index = 1;
    std::size_t count = model.meshes().size();
    float local_block_size = block_size * 0.3;

    for (auto& mesh : model.meshes())
    {
        std::clog << "\rHeightMap: Calculating mesh " << index++ << " of " << count << std::flush;
        auto triangle_point_size = std::accumulate(mesh.attributes().begin(), mesh.attributes().end(), 0);

        // Generating array of threads for calculating height in part of mesh
        std::vector<std::thread> threads;
        std::vector<std::vector<HeightPoint>> vectors(processor_count);
        auto vector = mesh.data().data();
        auto size = mesh.data().size();
        auto block = (size / (triangle_point_size * 3)) / processor_count;

        for (unsigned int i = 0; i < processor_count - 1; i++)
        {
            threads.emplace_back(calculate_height, std::ref(vectors[i]), vector + block * i * triangle_point_size * 3,
                                 block * triangle_point_size * 3, triangle_point_size, block_size, &_M_limits,
                                 local_block_size, std::ref(model_matrix));
        }

        // Calculate last part of mesh in main thread
        calculate_height(vectors.back(), vector + (block * (processor_count - 1) * triangle_point_size * 3),
                         size - (block * triangle_point_size * 3 * (processor_count - 1)), triangle_point_size,
                         block_size, &_M_limits, local_block_size, model_matrix);

        // Waiting for end of calculation
        for (auto& thread : threads) thread.join();

        // Updating data
        for (auto& tmp : vectors)
        {

            for (auto& value : tmp)
            {
                _M_array[value.x][value.y][value.z].push_back(value);
            }
        }
    }

    std::clog << std::endl << "Sorting arrays" << std::endl;
    for (auto& y : _M_array)
        for (auto& z : y)
            for (auto& block : z) std::sort(block.begin(), block.end());
    return *this;
}// namespace Engine

const HeightMapArray& HeightMap::array() const
{
    return _M_array;
}

std::size_t HeightMap::to_x_index(const float& x_coord) const
{

    std::size_t index = to_array_index(_M_limits.min.x, x_coord, _M_block_size);
    if (index >= _M_array.size())
        throw std::runtime_error("HeightMap: Index out of range");

    return index;
}

std::size_t HeightMap::to_y_index(const float& y_coord) const
{
    std::size_t index = to_array_index(_M_limits.min.y, y_coord, _M_block_size);
    if (index >= _M_array[0].size())
        throw std::runtime_error("HeightMap: Index out of range");
    return index;
}

std::size_t HeightMap::to_z_index(const float& z_coord) const
{

    std::size_t index = to_array_index(_M_limits.min.z, z_coord, _M_block_size);
    if (index >= _M_array[0][0].size())
        throw std::runtime_error("HeightMap: Index out of range");

    return index;
}

const Model::Limits& HeightMap::limits() const
{
    return _M_limits;
}
}// namespace Engine
