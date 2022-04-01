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
#define empty_normal(normal) (normal[0] == 0 && normal[1] == 0 && normal[2] == 0)

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
        }                                                                                                              \
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

struct use_point {
    int point1;
    int point2;
};

static void calculate_triangle_points(use_point points, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c,
                                      const float& local_block_size, const float& block_size,
                                      const Engine::Model::Limits& _M_limits, const glm::vec4& plane,
                                      const glm::vec3& normal, std::list<Engine::HeightMapValue>& output)
{
    float max_1 = max(max(a[points.point1], b[points.point1]), c[points.point1]);
    float max_2 = max(max(a[points.point2], b[points.point2]), c[points.point2]);

    float min_1 = min(min(a[points.point1], b[points.point1]), c[points.point1]);
    float min_2 = min(min(a[points.point2], b[points.point2]), c[points.point2]);

    glm::vec2 A(a[points.point1], a[points.point2]);
    glm::vec2 B(b[points.point1], b[points.point2]);
    glm::vec2 C(c[points.point1], c[points.point2]);

    int result_point = 3 - points.point1 - points.point2;
    Engine::ArrayIndex result_coords[3];
    glm::vec3 global_point;


    for (float first = min_1; first < max_1; first += local_block_size)
    {
        for (float second = min_2; second < max_2; second += local_block_size)
        {

            glm::vec2 P(first, second);
            if (!in_triangle(glm::vec3(A, 1.f), glm::vec3(B, 1.0f), glm::vec3(C, 1.0f), glm::vec3(P, 1.0f)))
                continue;

            result_coords[points.point1] = to_array_index(_M_limits.min[points.point1], first, block_size);
            result_coords[points.point2] = to_array_index(_M_limits.min[points.point2], second, block_size);

            global_point[points.point1] = first;
            global_point[points.point2] = second;

            if (plane[result_point] == 0)
            {
                // TODO
            }
            else
            {
                global_point[result_point] =
                        (-plane[3] - (plane[points.point1] * first) - (plane[points.point2] * second)) /
                        plane[result_point];

                global_point[result_point] = global_point[result_point] > _M_limits.max[result_point]
                                                     ? _M_limits.max[result_point]
                                                     : global_point[result_point];

                global_point[result_point] = global_point[result_point] < _M_limits.min[result_point]
                                                     ? _M_limits.min[result_point]
                                                     : global_point[result_point];

                result_coords[result_point] =
                        to_array_index(_M_limits.min[result_point], global_point[result_point], block_size);

                output.push_back({result_coords[0], result_coords[1], result_coords[2], global_point, normal});
            }
        }
    }
}


static void calculate_lines(use_point points, const std::vector<line>& lines, const float& local_block_size,
                            const Engine::Model::Limits& _M_limits, const float& block_size, const glm::vec3& normal,
                            std::list<Engine::HeightMapValue>& output)
{
    for (const auto& line : lines)
    {
        auto v = line.end - line.begin;
        if (v[points.point1] == 0 && v[points.point2] == 0)
            continue;
        int use_coord = -1;

        if (v[points.point1] == 0 || v[points.point2] == 0)
            use_coord = v[points.point1] == 0 ? points.point2 : points.point1;
        else
            use_coord = glm::abs(v[points.point1]) > glm::abs(v[points.point2]) ? points.point1 : points.point2;

        Engine::ArrayIndex indexes[3];
        glm::vec3 result;
        float end = max(line.begin[use_coord], line.end[use_coord]);
        int result_coord = 3 - points.point1 - points.point2;
        for (float start = min(line.begin[use_coord], line.end[use_coord]); start < end; start += local_block_size)
        {
            float t = (start - line.begin[use_coord]) / v[use_coord];
            result[points.point1] = (v[points.point1] * t) + line.begin[points.point1];
            result[points.point2] = (v[points.point2] * t) + line.begin[points.point2];

            result[result_coord] = (v[result_coord] * t) + line.begin[result_coord];
            result[result_coord] = result[result_coord] > _M_limits.max[result_coord] ? _M_limits.max[result_coord]
                                                                                      : result[result_coord];
            result[result_coord] = result[result_coord] < _M_limits.min[result_coord] ? _M_limits.min[result_coord]
                                                                                      : result[result_coord];

            indexes[points.point1] = to_array_index(_M_limits.min[points.point1], result[points.point1], block_size);
            indexes[points.point2] = to_array_index(_M_limits.min[points.point2], result[points.point2], block_size);
            indexes[result_coord] = to_array_index(_M_limits.min[result_coord], result[result_coord], block_size);

            output.push_back({indexes[0], indexes[1], indexes[2], result, normal});
        }
    }
}

static void calculate_height(std::list<HeightPoint>& output, const float* values, std::size_t size,
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
        glm::vec4 plane;

        glm::vec3 normal =
                triangle_block >= 8 ? glm::vec3(values[i + 5], values[i + 6], values[i + 7]) : glm::vec3(0.f, 0.f, 0.f);

        if (triangle_block < 8 || empty_normal(normal))
        {
            plane = {minor(b.y - a.y, c.y - a.y, b.z - a.z, c.z - a.z),
                     minor(b.x - a.x, c.x - a.x, b.z - a.z, c.z - a.z) * (-1),
                     minor(b.x - a.x, c.x - a.x, b.y - a.y, c.y - a.y), 0};
            normal = get_normal;
        }
        else
        {
            plane = glm::vec4(normal, 0.f);
        }
        plane[3] = (-a.x) * plane[0] + (-a.y) * plane[1] + (-a.z) * plane[2];

        output.push_back(point_to_height_point(a));
        output.push_back(point_to_height_point(b));
        output.push_back(point_to_height_point(c));


        std::vector<line> lines = {line({a, b}), line({a, c}), line({b, c})};
        calculate_lines({0, 2}, lines, local_block_size, _M_limits, block_size, normal, output);
        calculate_lines({1, 2}, lines, local_block_size, _M_limits, block_size, normal, output);
        calculate_lines({1, 2}, lines, local_block_size, _M_limits, block_size, normal, output);

        calculate_triangle_points({0, 2}, a, b, c, local_block_size, block_size, _M_limits, plane, normal, output);
        calculate_triangle_points({2, 0}, a, b, c, local_block_size, block_size, _M_limits, plane, normal, output);
        calculate_triangle_points({0, 1}, a, b, c, local_block_size, block_size, _M_limits, plane, normal, output);
        calculate_triangle_points({1, 0}, a, b, c, local_block_size, block_size, _M_limits, plane, normal, output);
        calculate_triangle_points({1, 2}, a, b, c, local_block_size, block_size, _M_limits, plane, normal, output);
        calculate_triangle_points({2, 1}, a, b, c, local_block_size, block_size, _M_limits, plane, normal, output);
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

            _M_array.resize(x_count, HeightMap_Y_Axis(y_count, HeightMap_Z_Axis(z_count, height_map_value)));
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
            std::vector<std::list<HeightPoint>> vectors(processor_count);
            auto vector = mesh.data().data();
            auto size = mesh.data().size();
            auto block = (size / (triangle_point_size * 3)) / processor_count;

            for (unsigned int i = 0; i < processor_count - 1; i++)
            {
                threads.emplace_back(calculate_height, std::ref(vectors[i]),
                                     vector + block * i * triangle_point_size * 3, block * triangle_point_size * 3,
                                     triangle_point_size, block_size, &_M_limits, local_block_size,
                                     std::ref(model_matrix));
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
                    auto& out = _M_array[value.x][value.y][value.z];
                    out.normal += value.normal;
                    if (out < value)
                        out.position = value.position;
                }
            }
        }
        std::clog << std::endl;
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

    float HeightMap::block_size() const
    {
        return _M_block_size;
    }
}// namespace Engine
