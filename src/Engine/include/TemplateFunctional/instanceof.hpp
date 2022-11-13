#pragma once

namespace Engine
{

    template<typename InstanceType, typename ObjectType>
    bool is_instance_of(ObjectType* object)
    {
        return dynamic_cast<InstanceType*>(object) != nullptr;
    }

    template<typename InstanceType, typename ObjectType>
    bool is_instance_of(const ObjectType& object)
    {
        return is_instance_of<const InstanceType>(&object);
    }
}// namespace Engine
