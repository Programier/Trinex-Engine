#include <Core/logger.hpp>
#include <SDL.h>
#include <Sensors/sensor.hpp>
#include <stdexcept>


namespace Engine
{
    /*
    SDL_SensorGetDeviceName
    SDL_SensorGetDeviceType
    SDL_SensorGetDeviceNonPortableType
    SDL_SensorGetDeviceInstanceID
    SDL_SensorFromInstanceID
    SDL_SensorGetName
    SDL_SensorGetType
    SDL_SensorGetNonPortableType
    SDL_SensorGetInstanceID
    SDL_SensorGetData
    SDL_SensorUpdate
    */

    uint_t Sensor::_M_sensor_count = 0;
    Vector<std::string> Sensor::_M_sensor_names = {};

    // Static methods implementation

    uint_t Sensor::sensors_count()
    {
        return _M_sensor_count;
    }

    const Vector<std::string>& Sensor::sensor_list()
    {
        return Sensor::_M_sensor_names;
    }

    uint_t Sensor::id_of(const std::string& sensor_name)
    {
        for (uint_t id = 0; id < _M_sensor_count; id++)
            if (_M_sensor_names[id] == sensor_name)
                return id;
        throw std::runtime_error("Sensor not found");
    }

    void Sensor::update_sensors_info()
    {
        SDL_SensorUpdate();
        _M_sensor_count = SDL_NumSensors();
        for (uint_t i = 0; i < _M_sensor_count; i++) _M_sensor_names.push_back(SDL_SensorGetDeviceName(i));
    }


    static void close_sensor(void* sensor)
    {
        if (sensor)
        {
            SDL_SensorClose(static_cast<SDL_Sensor*>(sensor));
            logger->log("Sensor: Close sensor %p\n", sensor);
        }
    }


    // Non static methods implementation

    Sensor::Sensor() = default;
    Sensor::Sensor(const std::string& sensor_name)
    {
        open(sensor_name);
    }

    Sensor::Sensor(uint_t id)
    {
        open(id);
    }

    Sensor::Sensor(const Sensor& sensor) = default;
    Sensor::Sensor(Sensor&& sensor) = default;
    Sensor& Sensor::operator=(const Sensor& sensor) = default;
    Sensor& Sensor::operator=(Sensor&& sensor) = default;

    Sensor& Sensor::open(uint_t id)
    {
        close();
        _M_sensor = SmartPointer<void>(SDL_SensorOpen(id), close_sensor);
        if (!_M_sensor.get())
        {
            logger->log("Sensor: Failed to load sensor with ID %d\n", id);
            return *this;
        }

        logger->log("Sensor: Opened sensor with %p\n", _M_sensor.get());
        _M_sensor_name = SDL_SensorGetName(static_cast<SDL_Sensor*>(_M_sensor.get()));
        return *this;
    }

    Sensor& Sensor::open(const std::string& sensor_name)
    {
        try
        {
            return open(Sensor::id_of(sensor_name));
        }
        catch (const std::exception& e)
        {
            logger->log("%s\n", e.what());
        }

        return *this;
    }

    const std::string& Sensor::name() const
    {
        return _M_sensor_name;
    }

    Sensor& Sensor::close()
    {
        _M_sensor = nullptr;
        return *this;
    }

    bool Sensor::is_open() const
    {
        return _M_sensor.get() != nullptr;
    }

    Sensor& Sensor::data_size(const std::size_t& size)
    {
        _M_data.resize(size);
        return *this;
    }

    std::size_t Sensor::data_size() const
    {
        return _M_data.size();
    }

    Sensor& Sensor::update()
    {
        SDL_Sensor* sensor = static_cast<SDL_Sensor*>(_M_sensor.get());

        if (sensor)
            SDL_SensorGetData(sensor, _M_data.data(), _M_data.size());
        return *this;
    }

    const Vector<float>& Sensor::data() const
    {
        return _M_data;
    }

    Sensor::~Sensor()
    {
        close();
    }
}// namespace Engine
