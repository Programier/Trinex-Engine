#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/property.hpp>
#include <Core/reflection/struct.hpp>
#include <Graphics/sampler.hpp>
#include <Widgets/property_renderer.hpp>

namespace Engine
{
	struct EditorSamplerInitializer : SamplerInitializer {
		trinex_declare_struct(EditorSamplerInitializer, void);

		EditorSamplerInitializer(const SamplerInitializer& initializer) : SamplerInitializer(initializer) {}
	};

	trinex_implement_struct(Engine::EditorSamplerInitializer, 0)
	{
		auto self = static_struct_instance();

		trinex_refl_prop(self, This, filter);
		trinex_refl_prop(self, This, address_u);
		trinex_refl_prop(self, This, address_v);
		trinex_refl_prop(self, This, address_w);
		trinex_refl_prop(self, This, compare_mode);
		trinex_refl_prop(self, This, compare_func);
		trinex_refl_prop(self, This, border_color);
	}

	static bool render_sampler_properties(PropertyRenderer* renderer, Refl::Struct* struct_instance, bool read_only)
	{
		Sampler* sampler = reinterpret_cast<Sampler*>(renderer->property_context());

		if (const SamplerInitializer* initializer = sampler->initializer())
		{
			EditorSamplerInitializer new_initializer = *initializer;

			if (renderer->render_struct_properties(&new_initializer, EditorSamplerInitializer::static_struct_instance(),
			                                       read_only))
			{
				sampler->init(new_initializer);
				return true;
			}
		}
		return false;
	}

	static void initialize()
	{
		PropertyRenderer::static_global_renderer_context()->struct_renderer(Sampler::static_struct_instance(),
		                                                                    render_sampler_properties);
	}

	static PreInitializeController initializer(initialize);
}// namespace Engine
