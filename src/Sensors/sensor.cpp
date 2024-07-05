//#include <Core/logger.hpp>
//#include <SDL.h>
//#include <Sensors/sensor.hpp>
//#include <stdexcept>


//namespace Engine
//{
//    /*
//    SDL_SensorGetDeviceName
//    SDL_SensorGetDeviceType
//    SDL_SensorGetDeviceNonPortableType
//    SDL_SensorGetDeviceInstanceID
//    SDL_SensorFromInstanceID
//    SDL_SensorGetName
//    SDL_SensorGetType
//    SDL_SensorGetNonPortableType
//    SDL_SensorGetInstanceID
//    SDL_SensorGetData
//    SDL_SensorUpdate
//    */

//    uint_t Sensor::m_sensor_count              = 0;
//    Vector<std::string> Sensor::m_sensor_names = {};

//    // Static methods implementation

//    uint_t Sensor::sensors_count()
//    {
//        return m_sensor_count;
//    }

//    const Vector<std::string>& Sensor::sensor_list()
//    {
//        return Sensor::m_sensor_names;
//    }

//    uint_t Sensor::id_of(const std::string& sensor_name)
//    {
//        for (uint_t id = 0; id < m_sensor_count; id++)
//            if (m_sensor_names[id] == sensor_name)
//                return id;
//        throw std::runtime_error("Sensor not found");
//    }

//    void Sensor::update_sensors_info()
//    {
//        SDL_SensorUpdate();
//        m_sensor_count = SDL_NumSensors();
//        for (uint_t i = 0; i < m_sensor_count; i++) m_sensor_names.push_back(SDL_SensorGetDeviceName(i));
//    }


//    static void close_sensor(void* sensor)
//    {
//        if (sensor)
//        {
//            SDL_SensorClose(static_cast<SDL_Sensor*>(sensor));
//            info_log("Sensor", "Close sensor %p\n", sensor);
//        }
//    }


//    // Non static methods implementation

//    Sensor::Sensor() = default;
//    Sensor::Sensor(const std::string& sensor_name)
//    {
//        open(sensor_name);
//    }

//    Sensor::Sensor(uint_t id)
//    {
//        open(id);
//    }

//    Sensor::Sensor(const Sensor& sensor)            = default;
//    Sensor::Sensor(Sensor&& sensor)                 = default;
//    Sensor& Sensor::operator=(const Sensor& sensor) = default;
//    Sensor& Sensor::operator=(Sensor&& sensor)      = default;

//    Sensor& Sensor::open(uint_t id)
//    {
//        close();
//        m_sensor = SmartPointer<void>(SDL_SensorOpen(id), close_sensor);
//        if (!m_sensor.get())
//        {
//            info_log("Sensor", "Failed to load sensor with ID %d\n", id);
//            return *this;
//        }

//        info_log("Sensor", "Opened sensor with %p\n", m_sensor.get());
//        m_sensor_name = SDL_SensorGetName(static_cast<SDL_Sensor*>(m_sensor.get()));
//        return *this;
//    }

//    Sensor& Sensor::open(const std::string& sensor_name)
//    {
//        try
//        {
//            return open(Sensor::id_of(sensor_name));
//        }
//        catch (const std::exception& e)
//        {
//            info_log("Sensor", "%s\n", e.what());
//        }

//        return *this;
//    }

//    const std::string& Sensor::name() const
//    {
//        return m_sensor_name;
//    }

//    Sensor& Sensor::close()
//    {
//        m_sensor = nullptr;
//        return *this;
//    }

//    bool Sensor::is_open() const
//    {
//        return m_sensor.get() != nullptr;
//    }

//    Sensor& Sensor::data_size(const std::size_t& size)
//    {
//        m_data.resize(size);
//        return *this;
//    }

//    std::size_t Sensor::data_size() const
//    {
//        return m_data.size();
//    }

//    Sensor& Sensor::update()
//    {
//        SDL_Sensor* sensor = static_cast<SDL_Sensor*>(m_sensor.get());

//        if (sensor)
//            SDL_SensorGetData(sensor, m_data.data(), m_data.size());
//        return *this;
//    }

//    const Vector<float>& Sensor::data() const
//    {
//        return m_data;
//    }

//    Sensor::~Sensor()
//    {
//        close();
//    }
//}// namespace Engine
