// #pragma once
// #include <Core/etl/string.hpp>
// #include <Core/etl/vector.hpp>
// #include <imgui.h>


// namespace Trinex::UI
// {
// 	class DockLayout
// 	{
// 	private:
// 		struct NamedDock {
// 			String id;
// 			u32 dock = 0;
// 		};

// 		DockID m_root = DockID();
// 		DockID m_main = DockID();
// 		Vector<NamedDock> m_named;

// 	public:
// 		bool exists() const;
// 		DockLayout& bind(StringView id, DockID dock);
// 		DockLayout& flags(DockID dock, DockNodeFlags flags);
// 		DockLayout& flags(StringView id, DockNodeFlags flags);
// 		DockID find(StringView id) const;
// 		DockID require(StringView id) const;
// 		bool has(StringView id) const;
// 		Result split(DockID dock, DockSplitDir dir, f32 ratio, StringView id = {});
// 		Result split(DockID dock, DockSplitDir dir, f32 ratio, StringView remainder_id, StringView child_id);
// 		DockID crop(DockID& dock, DockSplitDir dir, f32 ratio, StringView id = {});
// 		DockID crop(DockID& dock, DockSplitDir dir, f32 ratio, StringView remainder_id, StringView child_id);
// 		DockID dock(StringView window_name, DockID dock_id);
// 		DockID dock(StringView window_name, StringView dock_id);

// 		bool begin(Size size = {}, DockNodeFlags flags = DockNodeFlags::Undefined);
// 		bool begin(DockID root, Size size = {}, DockNodeFlags flags = DockNodeFlags::Undefined);
// 		DockLayout& end();

// 		inline DockID root() const { return m_root; }
// 		inline DockID main() const { return m_main; }
// 		inline DockLayout& main(DockID id) { trinex_this_return(m_main = id ? id : m_root); }

// 		inline DockID dock(StringView window_name) { return dock(window_name, m_main); }
// 		inline Result split(DockSplitDir dir, f32 ratio, StringView id = {}) { return split(m_main, dir, ratio, id); }
// 		inline Result split(DockSplitDir dir, f32 ratio, StringView remainder_id, StringView child_id)
// 		{
// 			return split(m_main, dir, ratio, remainder_id, child_id);
// 		}
// 	};
// }// namespace Trinex::UI
