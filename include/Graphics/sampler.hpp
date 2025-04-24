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
		const SamplerInitializer& initializer() const;
		RHI_Sampler* rhi_sampler() const;
		const Sampler& rhi_bind(byte location) const;
		bool serialize(Archive& ar);

		Sampler& filter(SamplerFilter filter);
		Sampler& address_u(SamplerAddressMode address);
		Sampler& address_v(SamplerAddressMode address);
		Sampler& address_w(SamplerAddressMode address);
		Sampler& compare_mode(CompareMode mode);
		Sampler& compare_func(CompareFunc func);
		Sampler& border_color(const Color& color);

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

		inline SamplerFilter filter() const { return initializer().filter; }
		inline SamplerAddressMode address_u() const { return initializer().address_u; }
		inline SamplerAddressMode address_v() const { return initializer().address_v; }
		inline SamplerAddressMode address_w() const { return initializer().address_w; }
		inline CompareMode compare_mode() const { return initializer().compare_mode; }
		inline CompareFunc compare_func() const { return initializer().compare_func; }
		inline Color border_color() const { return initializer().border_color; }


		inline ~Sampler() { release(); }
	};
}// namespace Engine
