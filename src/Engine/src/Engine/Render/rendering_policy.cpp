#include <Core/enum.hpp>
#include <Engine/Render/rendering_policy.hpp>

namespace Engine
{
    static Enum* rendering_policy_enum = nullptr;

    implement_enum(RenderingPolicy, Engine);

    static PreInitializeController on_init([]() {
        Enum::Entry entry;
        rendering_policy_enum = Enum::static_find("Engine::RenderingPolicy", true);
        register_policy(Name::color_scene_rendering);
        register_policy(Name::depth_scene_rendering);
    });


    ENGINE_EXPORT Enum* policies_enum()
    {
        return rendering_policy_enum;
    }

    ENGINE_EXPORT PolicyID register_policy(const Name& name)
    {
        return rendering_policy_enum->create_entry(name, name.hash())->value;
    }

    ENGINE_EXPORT Name policy_name(PolicyID id)
    {
        return rendering_policy_enum->entry(id)->name;
    }

    ENGINE_EXPORT PolicyID policy_id(const Name& name)
    {
        auto entry = rendering_policy_enum->entry(name);
        if (entry)
        {
            return entry->value;
        }
        return 0;
    }
}// namespace Engine
