#pragma once

namespace Trinex::UI
{
	class Widget
	{
	public:
		virtual ~Widget();

		virtual void on_init();
		virtual void on_render();
		virtual void on_close();
	};

	template<typename T>
	class UniqueWidget : public T
	{
	public:
		using T::T;

		template<typename... Args>
		static UniqueWidget* create(const Args&... args)
		{
			return trx_new UniqueWidget<T>(args...);
		}

		virtual void on_close() override
		{
			T::on_close();
			trx_delete this;
		}
	};
}// namespace Trinex::UI
