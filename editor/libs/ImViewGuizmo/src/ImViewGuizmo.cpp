#include <ImViewGuizmo.h>
#include <algorithm>

namespace ImViewGuizmo
{
	static const vec3_t origin                 = vec3_t(0.f, 0.f, 0.f);
	static constexpr vec3_t worldRight         = vec3_t(1.f, 0.f, 0.f);// +X
	static constexpr vec3_t worldUp            = vec3_t(0.f, 1.f, 0.f);// +Y
	static constexpr vec3_t worldForward       = vec3_t(0.f, 0.f, 1.f);// +Z
	static constexpr vec3_t axisVectors[3]     = {vec3_t(1, 0, 0), vec3_t(0, 1, 0), vec3_t(0, 0, 1)};
	static const mat4_t guizmoProjectionMatrix = glm::orthoRH(-1.f, 1.f, 1.f, -1.f, -1.f, 1.f);

	static Context* g_context = nullptr;

	struct GuizmoAxis {
		int id;          // 0-5 for (+X,-X,+Y,-Y,+Z,-Z), 6=center
		float depth;     // Screen-space depth
		vec3_t direction;// 3D vector
	};

	enum ActiveTool
	{
		TOOL_NONE,
		TOOL_GUIZMO,
		TOOL_DOLLY,
		TOOL_PAN
	};

	struct Context {
		Style style;

		int hoveredAxisID        = -1;
		bool isZoomButtonHovered = false;
		bool isPanButtonHovered  = false;
		ActiveTool activeTool    = TOOL_NONE;

		// Animation state
		bool isAnimating         = false;
		float animationStartTime = 0.f;

		vec3_t startPos;
		vec3_t targetPos;
		vec3_t startUp;
		vec3_t targetUp;

		vec3_t animStartDir, animTargetDir;
		float animStartDist, animTargetDist;
	};

	static inline float ImLengthSqr(const ImVec2& v)
	{
		return v.x * v.x + v.y * v.y;
	}

	Context* GetDefaultContext()
	{
		static Context ctx;
		return &ctx;
	}

	Context* GetCurrentContext()
	{
		if (g_context)
			return g_context;
		return GetDefaultContext();
	}

	void SetCurrentContext(Context* ctx)
	{
		g_context = ctx;
	}

	Style& GetStyle()
	{
		return GetCurrentContext()->style;
	}

	void Begin()
	{
		Context* ctx = GetCurrentContext();

		ctx->hoveredAxisID       = -1;
		ctx->isZoomButtonHovered = false;
		ctx->isPanButtonHovered  = false;

		if (ctx->activeTool != TOOL_NONE && !ImGui::GetIO().MouseDown[0])
			ctx->activeTool = TOOL_NONE;
	}

	void Begin(Context* ctx)
	{
		SetCurrentContext(ctx);
		Begin();
	}

	bool IsUsing()
	{
		return GetCurrentContext()->activeTool != TOOL_NONE;
	}

	bool IsOver()
	{
		Context* ctx = GetCurrentContext();
		return ctx->hoveredAxisID != -1 || ctx->isZoomButtonHovered || ctx->isPanButtonHovered;
	}

	bool Rotate(vec3_t& cameraPos, quat_t& cameraRot, const vec3_t& pivot, ImVec2 position, float rotationSpeed)
	{
		auto& io             = ImGui::GetIO();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		Context* ctx         = GetCurrentContext();
		auto& style          = GetStyle();
		bool wasModified     = false;

		// Animation
		if (ctx->isAnimating)
		{
			float elapsedTime = static_cast<float>(ImGui::GetTime()) - ctx->animationStartTime;
			float t           = Math::min(1.0f, elapsedTime / style.snapAnimationDuration);
			t                 = 1.0f - (1.0f - t) * (1.0f - t);// ease-out quad

			vec3_t currentDir     = Math::normalize(Math::lerp(ctx->animStartDir, ctx->animTargetDir, t));
			float currentDistance = Math::lerp(ctx->animStartDist, ctx->animTargetDist, t);
			cameraPos             = pivot + currentDir * currentDistance;

			vec3_t currentUp = Math::normalize(Math::lerp(ctx->startUp, ctx->targetUp, t));
			cameraRot        = Math::quat_look_at(currentDir, currentUp);

			wasModified = true;

			if (t >= 1.0f)
			{
				cameraPos        = ctx->targetPos;
				cameraRot        = Math::quat_look_at(ctx->animTargetDir, ctx->targetUp);
				ctx->isAnimating = false;
			}
		}

		// Guizmo sizes
		const float guizmoDiameter        = 256.f * style.scale;
		const float halfGuizmoSize        = guizmoDiameter / 2.f;
		const float scaledCircleRadius    = style.circleRadius * style.scale;
		const float scaledBigCircleRadius = style.bigCircleRadius * style.scale;
		const float scaledLineWidth       = style.lineWidth * style.scale;
		const float scaledHighlightWidth  = style.highlightWidth * style.scale;
		const float scaledHighlightRadius = (style.circleRadius + 2.0f) * style.scale;
		const float scaledFontSize        = ImGui::GetFontSize() * style.scale * style.labelSize;

		// Guizmo view matrix (transpose of rotation)
		mat4_t rotMat4 = Math::mat4_cast(cameraRot);

		vec3_t rcol0 = rotMat4[0];
		vec3_t rcol1 = rotMat4[1];
		vec3_t rcol2 = rotMat4[2];

		mat4_t guizmoViewMatrix(1.0f);
		guizmoViewMatrix[0][0] = rcol0.x;
		guizmoViewMatrix[0][1] = rcol1.x;
		guizmoViewMatrix[0][2] = rcol2.x;
		guizmoViewMatrix[0][3] = 0.0f;
		guizmoViewMatrix[1][0] = rcol0.y;
		guizmoViewMatrix[1][1] = rcol1.y;
		guizmoViewMatrix[1][2] = rcol2.y;
		guizmoViewMatrix[1][3] = 0.0f;
		guizmoViewMatrix[2][0] = rcol0.z;
		guizmoViewMatrix[2][1] = rcol1.z;
		guizmoViewMatrix[2][2] = rcol2.z;
		guizmoViewMatrix[2][3] = 0.0f;
		guizmoViewMatrix[3][0] = 0.0f;
		guizmoViewMatrix[3][1] = 0.0f;
		guizmoViewMatrix[3][2] = 0.0f;
		guizmoViewMatrix[3][3] = 1.0f;

		mat4_t guizmoMvp = guizmoProjectionMatrix * guizmoViewMatrix;

		// world->screen
		auto worldToScreen = [&](const vec3_t& worldPos) -> ImVec2 {
			const vec4_t clipPos = guizmoMvp * vec4_t(worldPos.x, worldPos.y, worldPos.z, 1.0f);
			const float w        = clipPos.w;
			if (fabsf(w) < 1e-6f)
				return {-FLT_MAX, -FLT_MAX};
			const float ndcX = clipPos.x / w;
			const float ndcY = clipPos.y / w;
			return {position.x + ndcX * halfGuizmoSize, position.y - ndcY * halfGuizmoSize};
		};

		auto worldToDepth = [&](const vec3_t& worldPos) -> float {
			const vec4_t clipPos = guizmoMvp * vec4_t(worldPos.x, worldPos.y, worldPos.z, 1.0f);
			const float depth    = clipPos.z / clipPos.w;
			return depth;
		};

		auto makeAxis = [&](int id, const vec3_t& direction) -> GuizmoAxis {
			return GuizmoAxis(id, worldToDepth(direction), direction);
		};

		// Axes (same ordering/depth calc as glm version)
		std::array<GuizmoAxis, 6> axes;

		axes[0] = makeAxis(0, axisVectors[0]);
		axes[1] = makeAxis(1, -axisVectors[0]);
		axes[2] = makeAxis(2, axisVectors[1]);
		axes[3] = makeAxis(3, -axisVectors[1]);
		axes[4] = makeAxis(4, axisVectors[2]);
		axes[5] = makeAxis(5, -axisVectors[2]);

		std::sort(axes.begin(), axes.end(), [](const GuizmoAxis& a, const GuizmoAxis& b) { return a.depth < b.depth; });

		const ImVec2 originScreenPos = worldToScreen(origin);

		// Hover detection
		const bool canInteract = !(io.ConfigFlags & ImGuiConfigFlags_NoMouse);
		if (canInteract && ctx->activeTool == TOOL_NONE && !ctx->isAnimating)
		{
			ImVec2 mousePos      = io.MousePos;
			float distToCenterSq = ImLengthSqr(ImVec2(mousePos.x - position.x, mousePos.y - position.y));

			if (distToCenterSq < (halfGuizmoSize + scaledCircleRadius) * (halfGuizmoSize + scaledCircleRadius))
			{
				const float minDistanceSq = scaledCircleRadius * scaledCircleRadius;
				for (const auto& axis : axes)
				{
					if (axis.depth < -0.1f)
						continue;

					ImVec2 handlePos = worldToScreen(axis.direction * style.lineLength);
					if (ImLengthSqr(ImVec2(handlePos.x - mousePos.x, handlePos.y - mousePos.y)) < minDistanceSq)
						ctx->hoveredAxisID = axis.id;
				}
				if (ctx->hoveredAxisID == -1 &&
				    ImLengthSqr(ImVec2(originScreenPos.x - mousePos.x, originScreenPos.y - mousePos.y)) <
				            scaledBigCircleRadius * scaledBigCircleRadius)
					ctx->hoveredAxisID = 6;
			}
		}

		// Drawing
		if (ctx->hoveredAxisID == 6 || ctx->activeTool == TOOL_GUIZMO)
			drawList->AddCircleFilled(originScreenPos, scaledBigCircleRadius, style.bigCircleColor);

		ImFont* font = ImGui::GetFont();
		for (const auto& axis : axes)
		{
			float colorFactor = Math::lerp(style.fadeFactor, 1.0f, axis.depth);
			ImVec4 baseColor  = ImGui::ColorConvertU32ToFloat4(style.axisColors[axis.id]);
			baseColor.w *= colorFactor;
			ImU32 final_color = ImGui::ColorConvertFloat4ToU32(baseColor);

			const ImVec2 handlePos = worldToScreen(axis.direction * style.lineLength);

			ImVec2 lineDir      = {handlePos.x - originScreenPos.x, handlePos.y - originScreenPos.y};
			float lineLengthVal = sqrtf(lineDir.x * lineDir.x + lineDir.y * lineDir.y) + 1e-6f;
			lineDir.x /= lineLengthVal;
			lineDir.y /= lineLengthVal;
			ImVec2 lineEndPos = {handlePos.x - lineDir.x * scaledCircleRadius, handlePos.y - lineDir.y * scaledCircleRadius};

			drawList->AddLine(originScreenPos, lineEndPos, final_color, scaledLineWidth);
			drawList->AddCircleFilled(handlePos, scaledCircleRadius, final_color);

			if (ctx->hoveredAxisID == axis.id)
				drawList->AddCircle(handlePos, scaledHighlightRadius, style.highlightColor, 0, scaledHighlightWidth);

			float textFactor = Math::max(0.0f, Math::min(1.0f, 1.0f + axis.depth * 2.5f));
			if (textFactor > 0.01f)
			{
				ImVec4 textColor = ImGui::ColorConvertU32ToFloat4(style.labelColor);
				textColor.w *= textFactor;
				const char* label = style.axisLabels[axis.id];
				ImVec2 textSize   = font->CalcTextSizeA(scaledFontSize, FLT_MAX, 0.f, label);
				drawList->AddText(font, scaledFontSize, {handlePos.x - textSize.x * 0.5f, handlePos.y - textSize.y * 0.5f},
				                  ImGui::ColorConvertFloat4ToU32(textColor), label);
			}
		}

		// Drag start
		if (canInteract && io.MouseDown[0] && ctx->activeTool == TOOL_NONE && ctx->hoveredAxisID == 6)
		{
			ctx->activeTool  = TOOL_GUIZMO;
			ctx->isAnimating = false;
		}

		// Active tool rotation
		if (ctx->activeTool == TOOL_GUIZMO)
		{
			float yawAngle   = io.MouseDelta.x * rotationSpeed;
			float pitchAngle = io.MouseDelta.y * rotationSpeed;

			quat_t yawRotation   = Math::angle_axis(yawAngle, worldUp);
			vec3_t rightAxis     = cameraRot * worldRight;
			quat_t pitchRotation = Math::angle_axis(pitchAngle, rightAxis);
			quat_t totalRotation = yawRotation * pitchRotation;

			vec3_t relativeCamPos = cameraPos - pivot;
			cameraPos             = pivot + (totalRotation * relativeCamPos);
			cameraRot             = totalRotation * cameraRot;

			wasModified = true;
		}

		// Snap on release
		if (canInteract && ImGui::IsMouseReleased(0) && ctx->hoveredAxisID >= 0 && ctx->hoveredAxisID <= 5 &&
		    ctx->activeTool == TOOL_NONE)
		{
			int axisIndex    = ctx->hoveredAxisID / 2;
			float sign       = (ctx->hoveredAxisID % 2 == 0) ? -1.0f : 1.0f;
			vec3_t targetDir = axisVectors[axisIndex] * sign;

			float currentDistance = Math::length(cameraPos - pivot);
			vec3_t targetPosition = pivot + (targetDir * currentDistance);

			vec3_t dirNormalized = Math::normalize(targetDir);

			vec3_t targetUp = worldUp;
			// If dir is nearly parallel to worldUp, pick a different up to avoid flipping
			if (fabsf(Math::dot(dirNormalized, targetUp)) > 0.999f)
			{
				if (dirNormalized.y > 0.0f)// facing "up"
					targetUp = worldForward;
				else// facing "down"
					targetUp = -worldForward;
			}

			quat_t targetRotation = Math::quat_look_at(targetDir, targetUp);

			if (style.animateSnap && style.snapAnimationDuration > 0.0f)
			{
				vec3_t difference     = cameraPos - targetPosition;
				bool pos_is_different = Math::dot(difference, difference) > 0.0001f;
				bool rot_is_different = (1.0f - fabsf(Math::dot(cameraRot, targetRotation))) > 0.0001f;

				if (pos_is_different || rot_is_different)
				{
					ctx->isAnimating        = true;
					ctx->animationStartTime = static_cast<float>(ImGui::GetTime());
					ctx->startPos           = cameraPos;
					ctx->targetPos          = targetPosition;
					ctx->startUp            = cameraRot * worldUp;
					ctx->targetUp           = targetUp;

					ctx->animStartDist  = Math::length(ctx->startPos - pivot);
					ctx->animTargetDist = Math::length(ctx->targetPos - pivot);
					ctx->animStartDir   = cameraRot * worldForward;
					ctx->animTargetDir  = targetDir;
				}
			}
			else
			{
				cameraRot   = targetRotation;
				cameraPos   = targetPosition;
				wasModified = true;
			}
		}

		return wasModified;
	}

	bool Dolly(vec3_t& cameraPos, const quat_t& cameraRot, const ImVec2 position, const float zoomSpeed)
	{

		const ImGuiIO& io    = ImGui::GetIO();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		Context* ctx         = GetCurrentContext();
		const Style& style   = GetStyle();
		bool wasModified     = false;

		const bool canInteract = !(io.ConfigFlags & ImGuiConfigFlags_NoMouse);
		const float radius     = style.toolButtonRadius * style.scale;
		const ImVec2 center    = {position.x + radius, position.y + radius};

		bool isHovered = false;
		if (canInteract && (ctx->activeTool == TOOL_NONE || ctx->activeTool == TOOL_DOLLY))
		{
			if (ImLengthSqr({io.MousePos.x - center.x, io.MousePos.y - center.y}) < radius * radius)
			{
				isHovered = true;
			}
		}
		ctx->isZoomButtonHovered = isHovered;

		if (canInteract && (isHovered || ctx->activeTool == TOOL_DOLLY))
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
		}

		if (canInteract && isHovered && io.MouseDown[0] && ctx->activeTool == TOOL_NONE)
		{
			ctx->activeTool  = TOOL_DOLLY;
			ctx->isAnimating = false;
		}

		if (ctx->activeTool == TOOL_DOLLY && io.MouseDelta.y != 0.0f)
		{
			vec3_t forwardMovement = (cameraRot * worldForward) * -io.MouseDelta.y * zoomSpeed;
			cameraPos              = cameraPos + forwardMovement;
			wasModified            = true;
		}

		const ImU32 bgColor = (ctx->activeTool == TOOL_DOLLY || isHovered) ? style.toolButtonHoveredColor : style.toolButtonColor;
		drawList->AddCircleFilled(center, radius, bgColor);

		const float p             = style.toolButtonInnerPadding * style.scale;
		const float th            = 2.0f * style.scale;
		const ImU32 iconColor     = style.toolButtonIconColor;
		constexpr float iconScale = 0.5f;
		const float scaledP       = p * iconScale;
		const float scaledRadius  = radius * iconScale;

		ImVec2 glassCenter      = {center.x - scaledP / 2.0f, center.y - scaledP / 2.0f};
		const float glassRadius = scaledRadius - scaledP;
		drawList->AddCircle(glassCenter, glassRadius, iconColor, 0, th);

		const ImVec2 handleStart = {center.x + scaledRadius / 2.0f, center.y + scaledRadius / 2.0f};
		const ImVec2 handleEnd   = {center.x + scaledRadius - (p * iconScale), center.y + scaledRadius - (p * iconScale)};
		drawList->AddLine(handleStart, handleEnd, iconColor, th);

		const float plusHalfSize = glassRadius * 0.5f;
		drawList->AddLine({glassCenter.x, glassCenter.y - plusHalfSize}, {glassCenter.x, glassCenter.y + plusHalfSize}, iconColor,
		                  th);
		drawList->AddLine({glassCenter.x - plusHalfSize, glassCenter.y}, {glassCenter.x + plusHalfSize, glassCenter.y}, iconColor,
		                  th);

		return wasModified;
	}

	bool Pan(vec3_t& cameraPos, const quat_t& cameraRot, const ImVec2 position, const float panSpeed)
	{
		const ImGuiIO& io    = ImGui::GetIO();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		Context* ctx         = GetCurrentContext();
		const Style& style   = GetStyle();
		bool wasModified     = false;

		const bool canInteract = !(io.ConfigFlags & ImGuiConfigFlags_NoMouse);
		const float radius     = style.toolButtonRadius * style.scale;
		const ImVec2 center    = {position.x + radius, position.y + radius};

		bool isHovered = false;
		if (canInteract && (ctx->activeTool == TOOL_NONE || ctx->activeTool == TOOL_PAN))
		{
			if (ImLengthSqr({io.MousePos.x - center.x, io.MousePos.y - center.y}) < radius * radius)
			{
				isHovered = true;
			}
		}
		ctx->isPanButtonHovered = isHovered;

		if (canInteract && (isHovered || ctx->activeTool == TOOL_PAN))
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
		}

		if (canInteract && isHovered && io.MouseDown[0] && ctx->activeTool == TOOL_NONE)
		{
			ctx->activeTool  = TOOL_PAN;
			ctx->isAnimating = false;
		}

		if (ctx->activeTool == TOOL_PAN && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f))
		{
			vec3_t rightMovement = (cameraRot * worldRight) * -io.MouseDelta.x * panSpeed;
			vec3_t upMovement    = (cameraRot * worldUp) * io.MouseDelta.y * panSpeed;
			cameraPos            = cameraPos + rightMovement;
			cameraPos            = cameraPos + upMovement;
			wasModified          = true;
		}

		const ImU32 bgColor = (isHovered || ctx->activeTool == TOOL_PAN) ? style.toolButtonHoveredColor : style.toolButtonColor;
		drawList->AddCircleFilled(center, radius, bgColor);

		const ImU32 iconColor = style.toolButtonIconColor;
		const float th        = 2.0f * style.scale;
		const float size      = radius * 0.5f;
		const float arm       = size * 0.25f;

		const ImVec2 topTip = {center.x, center.y - size};
		drawList->AddLine({topTip.x - arm, topTip.y + arm}, topTip, iconColor, th);
		drawList->AddLine({topTip.x + arm, topTip.y + arm}, topTip, iconColor, th);
		const ImVec2 botTip = {center.x, center.y + size};
		drawList->AddLine({botTip.x - arm, botTip.y - arm}, botTip, iconColor, th);
		drawList->AddLine({botTip.x + arm, botTip.y - arm}, botTip, iconColor, th);
		const ImVec2 leftTip = {center.x - size, center.y};
		drawList->AddLine({leftTip.x + arm, leftTip.y - arm}, leftTip, iconColor, th);
		drawList->AddLine({leftTip.x + arm, leftTip.y + arm}, leftTip, iconColor, th);
		const ImVec2 rightTip = {center.x + size, center.y};
		drawList->AddLine({rightTip.x - arm, rightTip.y - arm}, rightTip, iconColor, th);
		drawList->AddLine({rightTip.x - arm, rightTip.y + arm}, rightTip, iconColor, th);

		return wasModified;
	}
}// namespace ImViewGuizmo
