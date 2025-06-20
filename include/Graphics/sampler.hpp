#pragma once
#include <Core/types/color.hpp>
#include <RHI/enums.hpp>

namespace Engine
{
	struct RHISamplerInitializer;
	struct RHI_Sampler;

	class ENGINE_EXPORT Sampler final
	{
		trinex_declare_struct(Sampler, void);

	private:
		class SamplerImpl* m_sampler = nullptr;

		Sampler& add_ref();

	public:
		inline Sampler() { init(RHISamplerFilter::Point); };
		inline Sampler(const Sampler& sampler) : m_sampler(sampler.m_sampler) { add_ref(); }
		inline Sampler(Sampler&& sampler) : m_sampler(sampler.m_sampler) { sampler.m_sampler = nullptr; }
		inline Sampler(const RHISamplerInitializer& initializer) { init(initializer); }
		inline Sampler(RHISamplerFilter filter) { init(filter); }

		Sampler& init(const RHISamplerInitializer& initializer);
		Sampler& init(RHISamplerFilter filter);
		Sampler& release();
		const RHISamplerInitializer& initializer() const;
		RHI_Sampler* rhi_sampler() const;
		const Sampler& rhi_bind(byte location) const;
		bool serialize(class Archive& ar);

		Sampler& filter(RHISamplerFilter filter);
		Sampler& address_u(RHISamplerAddressMode address);
		Sampler& address_v(RHISamplerAddressMode address);
		Sampler& address_w(RHISamplerAddressMode address);
		Sampler& compare_func(RHICompareFunc func);
		Sampler& border_color(const Color& color);

		RHISamplerFilter filter() const;
		RHISamplerAddressMode address_u() const;
		RHISamplerAddressMode address_v() const;
		RHISamplerAddressMode address_w() const;
		RHICompareFunc compare_func() const;
		Color border_color() const;

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
