#pragma once
#include <d3d12.h>
#include <wrl.h>

namespace Engine
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	inline bool operator==(const D3D12_VIEWPORT& a, const D3D12_VIEWPORT& b)
	{
		constexpr float depth_epsilon = 0.00001f;

		if (a.TopLeftX != b.TopLeftX)
			return false;

		if (a.TopLeftY != b.TopLeftY)
			return false;

		if (a.Width != b.Width)
			return false;

		if (a.Height != b.Height)
			return false;

		float diff_min_depth = a.MinDepth - b.MinDepth;
		if (!(diff_min_depth > -depth_epsilon && diff_min_depth < depth_epsilon))
			return false;

		float diff_max_depth = a.MaxDepth - b.MaxDepth;
		if (!(diff_max_depth > -depth_epsilon && diff_max_depth < depth_epsilon))
			return false;

		return true;
	}

	inline bool operator!=(const D3D12_VIEWPORT& a, const D3D12_VIEWPORT& b)
	{
		return !(a == b);
	}

	inline bool operator==(const D3D12_RECT& a, const D3D12_RECT& b)
	{
		return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
	}

	inline bool operator!=(const D3D12_RECT& a, const D3D12_RECT& b)
	{
		return !(a == b);
	}
}// namespace Engine
