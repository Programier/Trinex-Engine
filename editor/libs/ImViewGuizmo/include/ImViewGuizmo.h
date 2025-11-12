#pragma once

/*===============================================
ImViewGuizmo Single-Header Library by Marcel Kazemi

To use, do this in one (and only one) of your C++ files:
#define IMVIEWGUIZMO_IMPLEMENTATION
 #include "ImViewGuizmo.h"

In all other files, just include the header as usual:
#include "ImViewGuizmo.h"

Copyright (c) 2025 Marcel Kazemi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
======================================================*/

#include <Core/math/math.hpp>
#include <imgui.h>

namespace ImViewGuizmo
{
	using vec3_t = Engine::Vector3f;
	using vec4_t = Engine::Vector4f;
	using quat_t = Engine::Quaternion;
	using mat4_t = Engine::Matrix4f;

	namespace Math = Engine::Math;

	// INTERFACE
	struct Style {
		float scale = 1.f;

		// Axis visuals
		float lineLength   = 0.34f;
		float lineWidth    = 4.0f;
		float circleRadius = 12.0f;
		float fadeFactor   = 0.25f;

		// Highlight
		ImU32 highlightColor = IM_COL32(255, 255, 0, 255);
		float highlightWidth = 2.0f;

		// Axis
		ImU32 axisColors[6] = {
		        IM_COL32(230, 51, 51, 255), // +X
		        IM_COL32(230, 51, 51, 255), // -X
		        IM_COL32(51, 230, 51, 255), // +Y
		        IM_COL32(51, 230, 51, 255), // -Y
		        IM_COL32(51, 128, 255, 255),// +Z
		        IM_COL32(51, 128, 255, 255) // -Z
		};

		// Labels
		float labelSize           = 1.0f;
		const char* axisLabels[6] = {"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
		ImU32 labelColor          = IM_COL32(255, 255, 255, 255);

		//Big Circle
		float bigCircleRadius = 60.0f;
		ImU32 bigCircleColor  = IM_COL32(255, 255, 255, 50);

		// Animation
		bool animateSnap            = true;
		float snapAnimationDuration = 0.5f;// in seconds

		// Zoom/Pan Button Visuals
		float toolButtonRadius       = 20.f;
		float toolButtonInnerPadding = 4.f;

		ImU32 toolButtonColor        = IM_COL32(144, 144, 144, 50);
		ImU32 toolButtonHoveredColor = IM_COL32(215, 215, 215, 50);
		ImU32 toolButtonIconColor    = IM_COL32(215, 215, 215, 225);
	};

	struct Context;

	Context* GetDefaultContext();
	Context* GetCurrentContext();
	void SetCurrentContext(Context* ctx);

	Style& GetStyle();

	void Begin();
	void Begin(Context* ctx);
	bool IsUsing();
	bool IsOver();

	bool Rotate(vec3_t& cameraPos, quat_t& cameraRot, const vec3_t& pivot, ImVec2 position, float rotationSpeed = 0.01f);
	bool Dolly(vec3_t& cameraPos, const quat_t& cameraRot, ImVec2 position, float zoomSpeed = 0.05f);
	bool Pan(vec3_t& cameraPos, const quat_t& cameraRot, ImVec2 position, float panSpeed = 0.01f);
}// namespace ImViewGuizmo
