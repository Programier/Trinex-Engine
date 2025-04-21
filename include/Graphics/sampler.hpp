#pragma once
#include <Core/render_resource.hpp>

namespace Engine
{
	struct SamplerInitializer {
		SamplerFilter filter         = SamplerFilter::Point;
		SamplerAddressMode address_u = SamplerAddressMode::Repeat;
		SamplerAddressMode address_v = SamplerAddressMode::Repeat;
		SamplerAddressMode address_w = SamplerAddressMode::Repeat;
		CompareMode compare_mode     = CompareMode::None;
		CompareFunc compare_func     = CompareFunc::Always;
		Color border_color           = Color(0, 0, 0, 255);

		float anisotropy;
		float mip_lod_bias;
		float min_lod;
		float max_lod;

		SamplerInitializer();
		HashIndex hash() const;

		bool operator==(const SamplerInitializer& initializer) const;
		inline bool operator!=(const SamplerInitializer& initializer) const { return !(*this == initializer); }
	};

	class ENGINE_EXPORT Sampler final
	{
		trinex_declare_struct(Sampler, void);

	private:
		class SamplerImpl* m_sampler = nullptr;

		Sampler& add_ref();

	public:
		inline Sampler() { init(SamplerFilter::Point); };
		inline Sampler(const Sampler& sampler) : m_sampler(sampler.m_sampler) { add_ref(); }
		inline Sampler(Sampler&& sampler) : m_sampler(sampler.m_sampler) { sampler.m_sampler = nullptr; }
		inline Sampler(const SamplerInitializer& initializer) { init(initializer); }
		inline Sampler(SamplerFilter filter) { init(filter); }

		Sampler& init(const SamplerInitializer& initializer);
		Sampler& init(SamplerFilter filter);
		Sampler& release();
		const SamplerInitializer* initializer() const;
		RHI_Sampler* rhi_sampler() const;
		const Sampler& rhi_bind(byte location) const;
		bool serialize(Archive& ar);

		inline Sampler& operator=(const Sampler& sampler)
		{
			if (this == &sampler)
				return *this;

			release();
			m_sampler = sampler.m_sampler;
			add_ref();
			return *this;
		}

		inline Sampler& operator=(Sampler&& sampler)
		{
			if (this == &sampler)
				return *this;

			release();
			m_sampler         = sampler.m_sampler;
			sampler.m_sampler = nullptr;
			return *this;
		}

		inline ~Sampler() { release(); }
	};
}// namespace Engine
