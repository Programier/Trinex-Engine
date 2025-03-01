#pragma once


namespace Engine
{
	template<typename RendererClass>
	class Renderer : public RendererClass
	{
	public:
		Renderer()
		{
			Renderer::initialize();
		}

		~Renderer() override
		{
			Renderer::finalize();
		}
	};
}// namespace Engine
