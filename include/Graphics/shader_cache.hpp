#pragma once
#include <Core/engine_types.hpp>

namespace Engine
{
    struct ENGINE_EXPORT ShaderCache {
        Buffer vertex;
        Buffer tessellation_control;
        Buffer tessellation;
        Buffer geometry;
        Buffer fragment;
        Buffer compute;

        void init_from(const class Pipeline* pipeline);
        void apply_to(class Pipeline* pipeline);
        bool load(const StringView& object_path, StringView rhi_name = {});
        bool store(const StringView& object_path, StringView rhi_name = {}) const;
    };
}// namespace Engine
