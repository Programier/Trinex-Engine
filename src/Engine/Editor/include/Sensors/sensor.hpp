#pragma once
#include <vector>
#include <string>
#include <Core/init.hpp>
#include <TemplateFunctional/smart_pointer.hpp>


namespace Engine
{
    CLASS Sensor
    {
        // Static variables
        static unsigned int _M_sensor_count;
        static std::vector<std::string> _M_sensor_names;

        // Non static variables
        unsigned int _M_ID;
        std::string _M_sensor_name;
        SmartPointer<void> _M_sensor = nullptr;
        std::vector<float> _M_data = {0.f, 0.f, 0.f};

    public:
        Sensor();
        Sensor(const std::string& sensor_name);
        Sensor(unsigned int id);
        Sensor(const Sensor& sensor);
        Sensor(Sensor&& sensor);
        Sensor& operator =(const Sensor& sensor);
        Sensor& operator =(Sensor&& sensor);

        Sensor& open(unsigned int id);
        Sensor& open(const std::string& sensor_name);
        Sensor& close();
        bool is_open() const;
        const std::string& name() const;

        Sensor& data_size(const std::size_t& size);
        std::size_t data_size() const;
        const std::vector<float>& data() const;
        Sensor& update();
        ~Sensor();

        static ENGINE_EXPORT unsigned int sensors_count();
        static ENGINE_EXPORT const std::vector<std::string>& sensor_list();
        static ENGINE_EXPORT unsigned int id_of(const std::string& sensor_name);
        static ENGINE_EXPORT void update_sensors_info();
        friend class Init;
    };
}
