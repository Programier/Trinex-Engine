#pragma once

#include <string>
#include <Core/export.hpp>
#include <Core/engine_types.hpp>
#include <TemplateFunctional/smart_pointer.hpp>


namespace Engine
{
    struct ENGINE_EXPORT Sensor
    {
        // Static variables
        static uint_t _M_sensor_count;
        static Vector<std::string> _M_sensor_names;

        // Non static variables
        uint_t _M_ID;
        std::string _M_sensor_name;
        SmartPointer<void> _M_sensor = nullptr;
        Vector<float> _M_data = {0.f, 0.f, 0.f};

    public:
        Sensor();
        Sensor(const std::string& sensor_name);
        Sensor(uint_t id);
        Sensor(const Sensor& sensor);
        Sensor(Sensor&& sensor);
        Sensor& operator =(const Sensor& sensor);
        Sensor& operator =(Sensor&& sensor);

        Sensor& open(uint_t id);
        Sensor& open(const std::string& sensor_name);
        Sensor& close();
        bool is_open() const;
        const std::string& name() const;

        Sensor& data_size(const std::size_t& size);
        std::size_t data_size() const;
        const Vector<float>& data() const;
        Sensor& update();
        ~Sensor();

        static ENGINE_EXPORT uint_t sensors_count();
        static ENGINE_EXPORT const Vector<std::string>& sensor_list();
        static ENGINE_EXPORT uint_t id_of(const std::string& sensor_name);
        static ENGINE_EXPORT void update_sensors_info();
        friend class Init;
    };
}
