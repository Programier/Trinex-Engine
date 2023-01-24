#pragma once
#include <Core/export.hpp>
#include <Core/object.hpp>
#include <Graphics/basic_object.hpp>
#include <list>


class aiAnimation;
class aiNodeAnim;


namespace Engine
{
    class Bone;
    class AnimatedObject;

    ENGINE_EXPORT class Animation final : public Object
    {
    public:
        ENGINE_EXPORT struct Channel : public BasicObject<Translate, Rotate, Scale> {

            template<typename Type>
            struct Key {
                float time;
                Type value;
            };

            using PositionKey = Key<Vector3D>;
            using ScalingKey = Key<Scale3D>;
            using RotationKey = Key<Quaternion>;

            std::vector<PositionKey> position_keys;
            std::vector<RotationKey> rotation_keys;
            std::vector<ScalingKey> scaling_keys;
            String name;

            Channel& update(float current_time);
            ArrayIndex get_position_index(float current_time) const;
            ArrayIndex get_rotation_index(float current_time) const;
            ArrayIndex get_scaling_index(float current_time) const;
            Bone* bone() const;
            constructor_hpp(Channel);
            delete_copy_constructors(Channel);
            declare_instance_info_hpp(Channel);


        private:
            Bone* _M_bone = nullptr;
            Channel& interpolate_position(float current_time);
            Channel& interpolate_scaling(float current_time);
            Channel& interpolate_rotation(float current_time);
            friend class Animation;
        };

        using ChannelsArray = std::vector<Channel*>;

    private:
        Animation& load(const aiAnimation* anim, const std::list<const aiNodeAnim*>& nodes, Bone* root);
        ChannelsArray _M_channels;
        AnimatedObject* _M_object;


        declare_instance_info_hpp(Animation);
    public:
        constructor_hpp(Animation, AnimatedObject* object);
        delete_copy_constructors(Animation);
        AnimatedObject* object() const;
        const ChannelsArray& channels() const;
        ChannelsArray& channels();
        float duration = 0.f;
        float ticks_per_second = 0.f;

        friend class AnimatedObject;
    };
}// namespace Engine
