#include <Core/engine_loading_controllers.hpp>
#include <Core/enums.hpp>
#include <Core/string_functions.hpp>
#include <Graphics/imgui.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_primitives.hpp>
#include <ScriptEngine/script_type_info.hpp>
#include <scriptarray.h>

namespace Engine
{
	// Wrappers

	template<typename In>
	struct ImGuiTypeWrapper {
		static In wrap(In in)
		{
			return in;
		}
	};

	template<typename In>
	struct ImGuiTypeWrapper<In&> {
		static In* wrap(In& in)
		{
			return &in;
		}
	};

	template<typename In>
	struct ImGuiTypeWrapper<const In&> {
		static const In* wrap(const In& in)
		{
			return &in;
		}
	};

	template<>
	struct ImGuiTypeWrapper<const String&> {
		static const char* wrap(const String& in)
		{
			return in.c_str();
		}
	};

	template<>
	struct ImGuiTypeWrapper<String&> {
		static const char* wrap(String& in)
		{
			return in.c_str();
		}
	};

	template<>
	struct ImGuiTypeWrapper<const ImVec2&> {
		static const ImVec2& wrap(const ImVec2& in)
		{
			return in;
		}
	};

	template<>
	struct ImGuiTypeWrapper<const ImVec4&> {
		static const ImVec4& wrap(const ImVec4& in)
		{
			return in;
		}
	};

	template<>
	struct ImGuiTypeWrapper<Vector2D&> {
		static float* wrap(Vector2D& in)
		{
			return &in.x;
		}
	};

	template<>
	struct ImGuiTypeWrapper<Vector3D&> {
		static float* wrap(Vector3D& in)
		{
			return &in.x;
		}
	};

	template<>
	struct ImGuiTypeWrapper<Vector4D&> {
		static float* wrap(Vector4D& in)
		{
			return &in.x;
		}
	};

	template<>
	struct ImGuiTypeWrapper<IntVector2D&> {
		static int* wrap(IntVector2D& in)
		{
			return &in.x;
		}
	};

	template<>
	struct ImGuiTypeWrapper<IntVector3D&> {
		static int* wrap(IntVector3D& in)
		{
			return &in.x;
		}
	};

	template<>
	struct ImGuiTypeWrapper<IntVector4D&> {
		static int* wrap(IntVector4D& in)
		{
			return &in.x;
		}
	};

	template<>
	struct ImGuiTypeWrapper<Boolean*> {
		static bool* wrap(Boolean* in)
		{
			if (in)
			{
				return &in->value;
			}
			return nullptr;
		}
	};


	// Inverse type wrapper
	template<typename T>
	struct ImGuiInvType {
		using Type = T;
	};

	template<typename T>
	struct ImGuiInvType<T*> {
		using Type = T&;
	};

	template<>
	struct ImGuiInvType<float[2]> {
		using Type = Engine::Vector2D&;
	};

	template<>
	struct ImGuiInvType<float[3]> {
		using Type = Engine::Vector3D&;
	};

	template<>
	struct ImGuiInvType<float[4]> {
		using Type = Engine::Vector4D&;
	};

	template<>
	struct ImGuiInvType<int[2]> {
		using Type = Engine::IntVector2D&;
	};

	template<>
	struct ImGuiInvType<int[3]> {
		using Type = Engine::IntVector3D&;
	};

	template<>
	struct ImGuiInvType<int[4]> {
		using Type = Engine::IntVector4D&;
	};

	template<typename T>
	struct ImGuiInvType<const T*> {
		using Type = const T&;
	};

	template<>
	struct ImGuiInvType<const char*> {
		using Type = const String&;
	};

	template<>
	struct ImGuiInvType<const ImVec2&> {
		using Type = const ImVec2&;
	};

	template<>
	struct ImGuiInvType<const ImVec4&> {
		using Type = const ImVec4&;
	};

	// Result type wrapper
	template<typename T>
	struct ImGuiResultType {
		using Type = T;
	};

	template<>
	struct ImGuiResultType<const char*> {
		using Type = String;
	};


	template<auto func, typename Result, typename... Args>
	static Result result_wrapped_func(Args... args)
	{
		return func(ImGuiTypeWrapper<Args>::wrap(args)...);
	}

	template<auto func, typename Result, typename... Args>
	static auto make_wrap_func(Result (*)(Args...))
	{
		return result_wrapped_func<func, typename ImGuiResultType<Result>::Type, typename ImGuiInvType<Args>::Type...>;
	}

	template<auto func>
	static auto make_wrap()
	{
		return make_wrap_func<func>(func);
	}


	static void wrapped_text(const String& text)
	{
		ImGui::Text("%s", text.c_str());
	}

	static void wrapped_text_colored(const ImVec4& color, const String& text)
	{
		ImGui::TextColored(color, "%s", text.c_str());
	}

	static void wrapped_text_wrapped(const String& text)
	{
		ImGui::TextWrapped("%s", text.c_str());
	}

	static void wrapped_text_disabled(const String& text)
	{
		ImGui::TextDisabled("%s", text.c_str());
	}

	static void wrapped_show_style_editor(const String& text)
	{
		ImGui::ShowStyleEditor();
	}

	static const char* combo_getter(void* _array, int index)
	{
		CScriptArray* array = reinterpret_cast<CScriptArray*>(_array);
		String* string      = reinterpret_cast<String*>(array->At(index));

		if (string)
		{
			return string->c_str();
		}
		return "";
	}

	static bool wrapped_combo(const String& name, int& current, CScriptArray* array, int value)
	{
		return ImGui::Combo(name.c_str(), &current, combo_getter, array, array->GetSize(), value);
	}

	static bool wrap_input_text(const String& label, String& buffer, int flags)
	{
		return ImGui::InputText(label.c_str(), buffer, flags);
	}

	static bool wrap_input_text_hint(const String& label, const String& hint, String& buffer, int flags)
	{
		return ImGui::InputTextWithHint(label.c_str(), hint.c_str(), buffer, flags);
	}

	static bool wrap_input_text_multiline(const String& label, String& buffer, const ImVec2& size, int flags)
	{
		return ImGui::InputTextMultiline(label.c_str(), buffer, size, flags);
	}

	static bool is_mouse_pos_valid_wrap1()
	{
		return ImGui::IsMousePosValid();
	}

	static bool is_mouse_pos_valid_wrap2(const ImVec2& pos)
	{
		return ImGui::IsMousePosValid(&pos);
	}

	static void bullet_text(const String& text)
	{
		ImGui::BulletText("%s", text.c_str());
	}

	static void set_tooltip(const String& in)
	{
		ImGui::SetTooltip("%s", in.c_str());
	}


	template<typename Type, size_t count = 1>
	void register_vector_type(const char* name)
	{
		ScriptClassRegistrar::ValueInfo info;
		info.more_constructors       = true;
		info.has_constructor         = true;
		info.has_assignment_operator = true;
		info.is_class                = true;
		info.all_floats              = true;

		ScriptClassRegistrar registrar = ScriptClassRegistrar::value_class(name, sizeof(Type), info);
		registrar.behave(ScriptClassBehave::Construct, "void f()", ScriptClassRegistrar::constructor<Type>,
		                 ScriptCallConv::CDeclObjFirst);
		registrar.behave(ScriptClassBehave::Construct, Strings::format("void f(const {}& in)", registrar.class_name()).c_str(),
		                 ScriptClassRegistrar::constructor<Type, const Type&>, ScriptCallConv::CDeclObjFirst);


		if constexpr (count == 2)
			registrar.behave(ScriptClassBehave::Construct, "void f(float, float)",
			                 ScriptClassRegistrar::constructor<Type, float, float>, ScriptCallConv::CDeclObjFirst);

		if constexpr (count == 4)
			registrar.behave(ScriptClassBehave::Construct, "void f(float, float, float, float)",
			                 ScriptClassRegistrar::constructor<Type, float, float, float, float>, ScriptCallConv::CDeclObjFirst);


		registrar.behave(ScriptClassBehave::Destruct, "void f()", ScriptClassRegistrar::destructor<Type>,
		                 ScriptCallConv::CDeclObjFirst);
	}


#define reg_func_nw_ns(a, b) ScriptEngine::register_function(a, b)
#define reg_func_nw(a, b) ScriptEngine::register_function(a, ImGui::b)
#define reg_func(a, b) ScriptEngine::register_function(a, make_wrap<ImGui::b>())
#define reg_func_no_ns(a, b) ScriptEngine::register_function(a, make_wrap<b>())

	static void on_init()
	{
		register_vector_type<ImVec2, 2>("ImVec2");
		register_vector_type<ImVec4, 4>("ImVec4");
#define new_enum_v(a, b) new_enum.set(#b, a##_##b)
#define new_enum_v2(a, b) new_enum.set(#b, a##b)

		{
			ScriptEnumRegistrar new_enum("ImGuiWindowFlags");
			new_enum.set("None", ImGuiWindowFlags_None);
			new_enum.set("NoTitleBar", ImGuiWindowFlags_NoTitleBar);
			new_enum.set("NoResize", ImGuiWindowFlags_NoResize);
			new_enum.set("NoMove", ImGuiWindowFlags_NoMove);
			new_enum.set("NoScrollbar", ImGuiWindowFlags_NoScrollbar);
			new_enum.set("NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse);
			new_enum.set("NoCollapse", ImGuiWindowFlags_NoCollapse);
			new_enum.set("AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize);
			new_enum.set("NoBackground", ImGuiWindowFlags_NoBackground);
			new_enum.set("NoSavedSettings", ImGuiWindowFlags_NoSavedSettings);
			new_enum.set("NoMouseInputs", ImGuiWindowFlags_NoMouseInputs);
			new_enum.set("MenuBar", ImGuiWindowFlags_MenuBar);
			new_enum.set("HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar);
			new_enum.set("NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing);
			new_enum.set("NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus);
			new_enum.set("AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar);
			new_enum.set("AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar);
			new_enum.set("NoNavInputs", ImGuiWindowFlags_NoNavInputs);
			new_enum.set("NoNavFocus", ImGuiWindowFlags_NoNavFocus);
			new_enum.set("UnsavedDocument", ImGuiWindowFlags_UnsavedDocument);
			new_enum.set("NoDocking", ImGuiWindowFlags_NoDocking);
			new_enum.set("NoNav", ImGuiWindowFlags_NoNav);
			new_enum.set("NoDecoration", ImGuiWindowFlags_NoDecoration);
			new_enum.set("NoInputs", ImGuiWindowFlags_NoInputs);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiDir");
			new_enum.set("None", ImGuiDir_None);
			new_enum.set("Left", ImGuiDir_Left);
			new_enum.set("Right", ImGuiDir_Right);
			new_enum.set("Up", ImGuiDir_Up);
			new_enum.set("Down", ImGuiDir_Down);
			new_enum.set("COUNT", ImGuiDir_COUNT);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiChildFlags");
			new_enum.set("None", ImGuiChildFlags_None);
			new_enum.set("Border", ImGuiChildFlags_Border);
			new_enum.set("AlwaysUseWindowPadding", ImGuiChildFlags_AlwaysUseWindowPadding);
			new_enum.set("ResizeX", ImGuiChildFlags_ResizeX);
			new_enum.set("ResizeY", ImGuiChildFlags_ResizeY);
			new_enum.set("AutoResizeX", ImGuiChildFlags_AutoResizeX);
			new_enum.set("AutoResizeY", ImGuiChildFlags_AutoResizeY);
			new_enum.set("AlwaysAutoResize", ImGuiChildFlags_AlwaysAutoResize);
			new_enum.set("FrameStyle", ImGuiChildFlags_FrameStyle);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiComboFlags");
			new_enum.set("None", ImGuiComboFlags_None);
			new_enum.set("PopupAlignLeft", ImGuiComboFlags_PopupAlignLeft);
			new_enum.set("HeightSmall", ImGuiComboFlags_HeightSmall);
			new_enum.set("HeightRegular", ImGuiComboFlags_HeightRegular);
			new_enum.set("HeightLarge", ImGuiComboFlags_HeightLarge);
			new_enum.set("HeightLargest", ImGuiComboFlags_HeightLargest);
			new_enum.set("NoArrowButton", ImGuiComboFlags_NoArrowButton);
			new_enum.set("NoPreview", ImGuiComboFlags_NoPreview);
			new_enum.set("WidthFitPreview", ImGuiComboFlags_WidthFitPreview);
			new_enum.set("HeightMask_", ImGuiComboFlags_HeightMask_);
		}
		{
			ScriptEnumRegistrar new_enum("ImGuiDragDropFlags");
			new_enum.set("None", ImGuiDragDropFlags_None);
			new_enum.set("SourceNoPreviewTooltip", ImGuiDragDropFlags_SourceNoPreviewTooltip);
			new_enum.set("SourceNoDisableHover", ImGuiDragDropFlags_SourceNoDisableHover);
			new_enum.set("SourceNoHoldToOpenOthers", ImGuiDragDropFlags_SourceNoHoldToOpenOthers);
			new_enum.set("SourceAllowNullID", ImGuiDragDropFlags_SourceAllowNullID);
			new_enum.set("SourceExtern", ImGuiDragDropFlags_SourceExtern);
			new_enum.set("SourceAutoExpirePayload", ImGuiDragDropFlags_SourceAutoExpirePayload);
			new_enum.set("AcceptBeforeDelivery", ImGuiDragDropFlags_AcceptBeforeDelivery);
			new_enum.set("AcceptNoDrawDefaultRect", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
			new_enum.set("AcceptNoPreviewTooltip", ImGuiDragDropFlags_AcceptNoPreviewTooltip);
			new_enum.set("AcceptPeekOnly", ImGuiDragDropFlags_AcceptPeekOnly);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiPopupFlags");

			new_enum.set("None", ImGuiPopupFlags_None);
			new_enum.set("MouseButtonLeft", ImGuiPopupFlags_MouseButtonLeft);
			new_enum.set("MouseButtonRight", ImGuiPopupFlags_MouseButtonRight);
			new_enum.set("MouseButtonMiddle", ImGuiPopupFlags_MouseButtonMiddle);
			new_enum.set("MouseButtonMask_", ImGuiPopupFlags_MouseButtonMask_);
			new_enum.set("MouseButtonDefault_", ImGuiPopupFlags_MouseButtonDefault_);
			new_enum.set("NoOpenOverExistingPopup", ImGuiPopupFlags_NoOpenOverExistingPopup);
			new_enum.set("NoOpenOverItems", ImGuiPopupFlags_NoOpenOverItems);
			new_enum.set("AnyPopupId", ImGuiPopupFlags_AnyPopupId);
			new_enum.set("AnyPopupLevel", ImGuiPopupFlags_AnyPopupLevel);
			new_enum.set("AnyPopup", ImGuiPopupFlags_AnyPopup);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiMouseButton");
			new_enum.set("Left", ImGuiMouseButton_Left);
			new_enum.set("Right", ImGuiMouseButton_Right);
			new_enum.set("Middle", ImGuiMouseButton_Middle);
			new_enum.set("COUNT", ImGuiMouseButton_COUNT);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiTabBarFlags");
			new_enum.set("None", ImGuiTabBarFlags_None);
			new_enum.set("Reorderable", ImGuiTabBarFlags_Reorderable);
			new_enum.set("AutoSelectNewTabs", ImGuiTabBarFlags_AutoSelectNewTabs);
			new_enum.set("TabListPopupButton", ImGuiTabBarFlags_TabListPopupButton);
			new_enum.set("NoCloseWithMiddleMouseButton", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton);
			new_enum.set("NoTabListScrollingButtons", ImGuiTabBarFlags_NoTabListScrollingButtons);
			new_enum.set("NoTooltip", ImGuiTabBarFlags_NoTooltip);
			new_enum.set("FittingPolicyResizeDown", ImGuiTabBarFlags_FittingPolicyResizeDown);
			new_enum.set("FittingPolicyScroll", ImGuiTabBarFlags_FittingPolicyScroll);
			new_enum.set("FittingPolicyMask_", ImGuiTabBarFlags_FittingPolicyMask_);
			new_enum.set("FittingPolicyDefault_", ImGuiTabBarFlags_FittingPolicyDefault_);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiHoveredFlags");
			new_enum.set("None", ImGuiHoveredFlags_None);
			new_enum.set("ChildWindows", ImGuiHoveredFlags_ChildWindows);
			new_enum.set("RootWindow", ImGuiHoveredFlags_RootWindow);
			new_enum.set("AnyWindow", ImGuiHoveredFlags_AnyWindow);
			new_enum.set("NoPopupHierarchy", ImGuiHoveredFlags_NoPopupHierarchy);
			new_enum.set("DockHierarchy", ImGuiHoveredFlags_DockHierarchy);
			new_enum.set("AllowWhenBlockedByPopup", ImGuiHoveredFlags_AllowWhenBlockedByPopup);
			new_enum.set("AllowWhenBlockedByActiveItem", ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
			new_enum.set("AllowWhenOverlappedByItem", ImGuiHoveredFlags_AllowWhenOverlappedByItem);
			new_enum.set("AllowWhenOverlappedByWindow", ImGuiHoveredFlags_AllowWhenOverlappedByWindow);
			new_enum.set("AllowWhenDisabled", ImGuiHoveredFlags_AllowWhenDisabled);
			new_enum.set("NoNavOverride", ImGuiHoveredFlags_NoNavOverride);
			new_enum.set("AllowWhenOverlapped", ImGuiHoveredFlags_AllowWhenOverlapped);
			new_enum.set("RectOnly", ImGuiHoveredFlags_RectOnly);
			new_enum.set("RootAndChildWindows", ImGuiHoveredFlags_RootAndChildWindows);
			new_enum.set("ForTooltip", ImGuiHoveredFlags_ForTooltip);
			new_enum.set("Stationary", ImGuiHoveredFlags_Stationary);
			new_enum.set("DelayNone", ImGuiHoveredFlags_DelayNone);
			new_enum.set("DelayShort", ImGuiHoveredFlags_DelayShort);
			new_enum.set("DelayNormal", ImGuiHoveredFlags_DelayNormal);
			new_enum.set("NoSharedDelay", ImGuiHoveredFlags_NoSharedDelay);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiTableFlags");
			new_enum.set("None", ImGuiTableFlags_None);
			new_enum.set("Resizable", ImGuiTableFlags_Resizable);
			new_enum.set("Reorderable", ImGuiTableFlags_Reorderable);
			new_enum.set("Hideable", ImGuiTableFlags_Hideable);
			new_enum.set("Sortable", ImGuiTableFlags_Sortable);
			new_enum.set("NoSavedSettings", ImGuiTableFlags_NoSavedSettings);
			new_enum.set("ContextMenuInBody", ImGuiTableFlags_ContextMenuInBody);
			new_enum.set("RowBg", ImGuiTableFlags_RowBg);
			new_enum.set("BordersInnerH", ImGuiTableFlags_BordersInnerH);
			new_enum.set("BordersOuterH", ImGuiTableFlags_BordersOuterH);
			new_enum.set("BordersInnerV", ImGuiTableFlags_BordersInnerV);
			new_enum.set("BordersOuterV", ImGuiTableFlags_BordersOuterV);
			new_enum.set("BordersH", ImGuiTableFlags_BordersH);
			new_enum.set("BordersV", ImGuiTableFlags_BordersV);
			new_enum.set("BordersInner", ImGuiTableFlags_BordersInner);
			new_enum.set("BordersOuter", ImGuiTableFlags_BordersOuter);
			new_enum.set("Borders", ImGuiTableFlags_Borders);
			new_enum.set("NoBordersInBody", ImGuiTableFlags_NoBordersInBody);
			new_enum.set("NoBordersInBodyUntilResize", ImGuiTableFlags_NoBordersInBodyUntilResize);
			new_enum.set("SizingFixedFit", ImGuiTableFlags_SizingFixedFit);
			new_enum.set("SizingFixedSame", ImGuiTableFlags_SizingFixedSame);
			new_enum.set("SizingStretchProp", ImGuiTableFlags_SizingStretchProp);
			new_enum.set("SizingStretchSame", ImGuiTableFlags_SizingStretchSame);
			new_enum.set("NoHostExtendX", ImGuiTableFlags_NoHostExtendX);
			new_enum.set("NoHostExtendY", ImGuiTableFlags_NoHostExtendY);
			new_enum.set("NoKeepColumnsVisible", ImGuiTableFlags_NoKeepColumnsVisible);
			new_enum.set("PreciseWidths", ImGuiTableFlags_PreciseWidths);
			new_enum.set("NoClip", ImGuiTableFlags_NoClip);
			new_enum.set("PadOuterX", ImGuiTableFlags_PadOuterX);
			new_enum.set("NoPadOuterX", ImGuiTableFlags_NoPadOuterX);
			new_enum.set("NoPadInnerX", ImGuiTableFlags_NoPadInnerX);
			new_enum.set("ScrollX", ImGuiTableFlags_ScrollX);
			new_enum.set("ScrollY", ImGuiTableFlags_ScrollY);
			new_enum.set("SortMulti", ImGuiTableFlags_SortMulti);
			new_enum.set("SortTristate", ImGuiTableFlags_SortTristate);
			new_enum.set("HighlightHoveredColumn", ImGuiTableFlags_HighlightHoveredColumn);
			new_enum.set("SizingMask_", ImGuiTableFlags_SizingMask_);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiTreeNodeFlags");
			new_enum.set("None", ImGuiTreeNodeFlags_None);
			new_enum.set("Selected", ImGuiTreeNodeFlags_Selected);
			new_enum.set("Framed", ImGuiTreeNodeFlags_Framed);
			new_enum.set("AllowOverlap", ImGuiTreeNodeFlags_AllowOverlap);
			new_enum.set("NoTreePushOnOpen", ImGuiTreeNodeFlags_NoTreePushOnOpen);
			new_enum.set("NoAutoOpenOnLog", ImGuiTreeNodeFlags_NoAutoOpenOnLog);
			new_enum.set("DefaultOpen", ImGuiTreeNodeFlags_DefaultOpen);
			new_enum.set("OpenOnDoubleClick", ImGuiTreeNodeFlags_OpenOnDoubleClick);
			new_enum.set("OpenOnArrow", ImGuiTreeNodeFlags_OpenOnArrow);
			new_enum.set("Leaf", ImGuiTreeNodeFlags_Leaf);
			new_enum.set("Bullet", ImGuiTreeNodeFlags_Bullet);
			new_enum.set("FramePadding", ImGuiTreeNodeFlags_FramePadding);
			new_enum.set("SpanAvailWidth", ImGuiTreeNodeFlags_SpanAvailWidth);
			new_enum.set("SpanFullWidth", ImGuiTreeNodeFlags_SpanFullWidth);
			new_enum.set("SpanAllColumns", ImGuiTreeNodeFlags_SpanAllColumns);
			new_enum.set("NavLeftJumpsBackHere", ImGuiTreeNodeFlags_NavLeftJumpsBackHere);
			new_enum.set("CollapsingHeader", ImGuiTreeNodeFlags_CollapsingHeader);

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
			new_enum.set("AllowItemOverlap", ImGuiTreeNodeFlags_AllowItemOverlap);
#endif
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiColorEditFlags");
			new_enum.set("None", ImGuiColorEditFlags_None);
			new_enum.set("NoAlpha", ImGuiColorEditFlags_NoAlpha);
			new_enum.set("NoPicker", ImGuiColorEditFlags_NoPicker);
			new_enum.set("NoOptions", ImGuiColorEditFlags_NoOptions);
			new_enum.set("NoSmallPreview", ImGuiColorEditFlags_NoSmallPreview);
			new_enum.set("NoInputs", ImGuiColorEditFlags_NoInputs);
			new_enum.set("NoTooltip", ImGuiColorEditFlags_NoTooltip);
			new_enum.set("NoLabel", ImGuiColorEditFlags_NoLabel);
			new_enum.set("NoSidePreview", ImGuiColorEditFlags_NoSidePreview);
			new_enum.set("NoDragDrop", ImGuiColorEditFlags_NoDragDrop);
			new_enum.set("NoBorder", ImGuiColorEditFlags_NoBorder);
			new_enum.set("AlphaBar", ImGuiColorEditFlags_AlphaBar);
			new_enum.set("AlphaPreview", ImGuiColorEditFlags_AlphaPreview);
			new_enum.set("AlphaPreviewHalf", ImGuiColorEditFlags_AlphaPreviewHalf);
			new_enum.set("HDR", ImGuiColorEditFlags_HDR);
			new_enum.set("DisplayRGB", ImGuiColorEditFlags_DisplayRGB);
			new_enum.set("DisplayHSV", ImGuiColorEditFlags_DisplayHSV);
			new_enum.set("DisplayHex", ImGuiColorEditFlags_DisplayHex);
			new_enum.set("Uint8", ImGuiColorEditFlags_Uint8);
			new_enum.set("Float", ImGuiColorEditFlags_Float);
			new_enum.set("PickerHueBar", ImGuiColorEditFlags_PickerHueBar);
			new_enum.set("PickerHueWheel", ImGuiColorEditFlags_PickerHueWheel);
			new_enum.set("InputRGB", ImGuiColorEditFlags_InputRGB);
			new_enum.set("InputHSV", ImGuiColorEditFlags_InputHSV);
			new_enum.set("DefaultOptions_", ImGuiColorEditFlags_DefaultOptions_);
			new_enum.set("DisplayMask_", ImGuiColorEditFlags_DisplayMask_);
			new_enum.set("DataTypeMask_", ImGuiColorEditFlags_DataTypeMask_);
			new_enum.set("PickerMask_", ImGuiColorEditFlags_PickerMask_);
			new_enum.set("InputMask_", ImGuiColorEditFlags_InputMask_);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiSliderFlags");
			new_enum.set("None", ImGuiSliderFlags_None);
			new_enum.set("AlwaysClamp", ImGuiSliderFlags_AlwaysClamp);
			new_enum.set("Logarithmic", ImGuiSliderFlags_Logarithmic);
			new_enum.set("NoRoundToFormat", ImGuiSliderFlags_NoRoundToFormat);
			new_enum.set("NoInput", ImGuiSliderFlags_NoInput);
			new_enum.set("InvalidMask_", ImGuiSliderFlags_InvalidMask_);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiButtonFlags");
			new_enum.set("None", ImGuiButtonFlags_None);
			new_enum.set("MouseButtonLeft", ImGuiButtonFlags_MouseButtonLeft);
			new_enum.set("MouseButtonRight", ImGuiButtonFlags_MouseButtonRight);
			new_enum.set("MouseButtonMiddle", ImGuiButtonFlags_MouseButtonMiddle);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiInputTextFlags");
			new_enum.set("None", ImGuiInputTextFlags_None);
			new_enum.set("CharsDecimal", ImGuiInputTextFlags_CharsDecimal);
			new_enum.set("CharsHexadecimal", ImGuiInputTextFlags_CharsHexadecimal);
			new_enum.set("CharsUppercase", ImGuiInputTextFlags_CharsUppercase);
			new_enum.set("CharsNoBlank", ImGuiInputTextFlags_CharsNoBlank);
			new_enum.set("AutoSelectAll", ImGuiInputTextFlags_AutoSelectAll);
			new_enum.set("EnterReturnsTrue", ImGuiInputTextFlags_EnterReturnsTrue);
			new_enum.set("CallbackCompletion", ImGuiInputTextFlags_CallbackCompletion);
			new_enum.set("CallbackHistory", ImGuiInputTextFlags_CallbackHistory);
			new_enum.set("CallbackAlways", ImGuiInputTextFlags_CallbackAlways);
			new_enum.set("CallbackCharFilter", ImGuiInputTextFlags_CallbackCharFilter);
			new_enum.set("AllowTabInput", ImGuiInputTextFlags_AllowTabInput);
			new_enum.set("CtrlEnterForNewLine", ImGuiInputTextFlags_CtrlEnterForNewLine);
			new_enum.set("NoHorizontalScroll", ImGuiInputTextFlags_NoHorizontalScroll);
			new_enum.set("AlwaysOverwrite", ImGuiInputTextFlags_AlwaysOverwrite);
			new_enum.set("ReadOnly", ImGuiInputTextFlags_ReadOnly);
			new_enum.set("Password", ImGuiInputTextFlags_Password);
			new_enum.set("NoUndoRedo", ImGuiInputTextFlags_NoUndoRedo);
			new_enum.set("CharsScientific", ImGuiInputTextFlags_CharsScientific);
			new_enum.set("CallbackResize", ImGuiInputTextFlags_CallbackResize);
			new_enum.set("CallbackEdit", ImGuiInputTextFlags_CallbackEdit);
			new_enum.set("EscapeClearsAll", ImGuiInputTextFlags_EscapeClearsAll);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiCond");
			new_enum.set("None", ImGuiCond_None);
			new_enum.set("Always", ImGuiCond_Always);
			new_enum.set("Once", ImGuiCond_Once);
			new_enum.set("FirstUseEver", ImGuiCond_FirstUseEver);
			new_enum.set("Appearing", ImGuiCond_Appearing);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiConfigFlags");
			new_enum.set("None", ImGuiConfigFlags_None);
			new_enum.set("NavEnableKeyboard", ImGuiConfigFlags_NavEnableKeyboard);
			new_enum.set("NavEnableGamepad", ImGuiConfigFlags_NavEnableGamepad);
			new_enum.set("NavEnableSetMousePos", ImGuiConfigFlags_NavEnableSetMousePos);
			new_enum.set("NavNoCaptureKeyboard", ImGuiConfigFlags_NavNoCaptureKeyboard);
			new_enum.set("NoMouse", ImGuiConfigFlags_NoMouse);
			new_enum.set("NoMouseCursorChange", ImGuiConfigFlags_NoMouseCursorChange);
			new_enum.set("DockingEnable", ImGuiConfigFlags_DockingEnable);
			new_enum.set("ViewportsEnable", ImGuiConfigFlags_ViewportsEnable);
			new_enum.set("DpiEnableScaleViewports", ImGuiConfigFlags_DpiEnableScaleViewports);
			new_enum.set("DpiEnableScaleFonts", ImGuiConfigFlags_DpiEnableScaleFonts);
			new_enum.set("IsSRGB", ImGuiConfigFlags_IsSRGB);
			new_enum.set("IsTouchScreen", ImGuiConfigFlags_IsTouchScreen);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiDockNodeFlags");
			new_enum.set("None", ImGuiDockNodeFlags_None);
			new_enum.set("KeepAliveOnly", ImGuiDockNodeFlags_KeepAliveOnly);
			new_enum.set("NoDockingOverCentralNode", ImGuiDockNodeFlags_NoDockingOverCentralNode);
			new_enum.set("PassthruCentralNode", ImGuiDockNodeFlags_PassthruCentralNode);
			new_enum.set("NoDockingSplit", ImGuiDockNodeFlags_NoDockingSplit);
			new_enum.set("NoResize", ImGuiDockNodeFlags_NoResize);
			new_enum.set("AutoHideTabBar", ImGuiDockNodeFlags_AutoHideTabBar);
			new_enum.set("NoUndocking", ImGuiDockNodeFlags_NoUndocking);

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
			new_enum.set("NoSplit", ImGuiDockNodeFlags_NoSplit);
			new_enum.set("NoDockingInCentralNode", ImGuiDockNodeFlags_NoDockingInCentralNode);
#endif
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiFocusedFlags");
			new_enum_v(ImGuiFocusedFlags, None);
			new_enum_v(ImGuiFocusedFlags, ChildWindows);
			new_enum_v(ImGuiFocusedFlags, RootWindow);
			new_enum_v(ImGuiFocusedFlags, AnyWindow);
			new_enum_v(ImGuiFocusedFlags, NoPopupHierarchy);
			new_enum_v(ImGuiFocusedFlags, DockHierarchy);
			new_enum_v(ImGuiFocusedFlags, RootAndChildWindows);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiKey");
			new_enum_v(ImGuiKey, None);
			new_enum_v(ImGuiKey, Tab);
			new_enum_v(ImGuiKey, LeftArrow);
			new_enum_v(ImGuiKey, RightArrow);
			new_enum_v(ImGuiKey, UpArrow);
			new_enum_v(ImGuiKey, DownArrow);
			new_enum_v(ImGuiKey, PageUp);
			new_enum_v(ImGuiKey, PageDown);
			new_enum_v(ImGuiKey, Home);
			new_enum_v(ImGuiKey, End);
			new_enum_v(ImGuiKey, Insert);
			new_enum_v(ImGuiKey, Delete);
			new_enum_v(ImGuiKey, Backspace);
			new_enum_v(ImGuiKey, Space);
			new_enum_v(ImGuiKey, Enter);
			new_enum_v(ImGuiKey, Escape);
			new_enum_v(ImGuiKey, LeftCtrl);
			new_enum_v(ImGuiKey, LeftShift);
			new_enum_v(ImGuiKey, LeftAlt);
			new_enum_v(ImGuiKey, LeftSuper);
			new_enum_v(ImGuiKey, RightCtrl);
			new_enum_v(ImGuiKey, RightShift);
			new_enum_v(ImGuiKey, RightAlt);
			new_enum_v(ImGuiKey, RightSuper);
			new_enum_v(ImGuiKey, Menu);
			new_enum_v2(ImGuiKey, _0);
			new_enum_v2(ImGuiKey, _1);
			new_enum_v2(ImGuiKey, _2);
			new_enum_v2(ImGuiKey, _3);
			new_enum_v2(ImGuiKey, _4);
			new_enum_v2(ImGuiKey, _5);
			new_enum_v2(ImGuiKey, _6);
			new_enum_v2(ImGuiKey, _7);
			new_enum_v2(ImGuiKey, _8);
			new_enum_v2(ImGuiKey, _9);
			new_enum_v(ImGuiKey, A);
			new_enum_v(ImGuiKey, B);
			new_enum_v(ImGuiKey, C);
			new_enum_v(ImGuiKey, D);
			new_enum_v(ImGuiKey, E);
			new_enum_v(ImGuiKey, F);
			new_enum_v(ImGuiKey, G);
			new_enum_v(ImGuiKey, H);
			new_enum_v(ImGuiKey, I);
			new_enum_v(ImGuiKey, J);
			new_enum_v(ImGuiKey, K);
			new_enum_v(ImGuiKey, L);
			new_enum_v(ImGuiKey, M);
			new_enum_v(ImGuiKey, N);
			new_enum_v(ImGuiKey, O);
			new_enum_v(ImGuiKey, P);
			new_enum_v(ImGuiKey, Q);
			new_enum_v(ImGuiKey, R);
			new_enum_v(ImGuiKey, S);
			new_enum_v(ImGuiKey, T);
			new_enum_v(ImGuiKey, U);
			new_enum_v(ImGuiKey, V);
			new_enum_v(ImGuiKey, W);
			new_enum_v(ImGuiKey, X);
			new_enum_v(ImGuiKey, Y);
			new_enum_v(ImGuiKey, Z);
			new_enum_v(ImGuiKey, F1);
			new_enum_v(ImGuiKey, F2);
			new_enum_v(ImGuiKey, F3);
			new_enum_v(ImGuiKey, F4);
			new_enum_v(ImGuiKey, F5);
			new_enum_v(ImGuiKey, F6);
			new_enum_v(ImGuiKey, F7);
			new_enum_v(ImGuiKey, F8);
			new_enum_v(ImGuiKey, F9);
			new_enum_v(ImGuiKey, F10);
			new_enum_v(ImGuiKey, F11);
			new_enum_v(ImGuiKey, F12);
			new_enum_v(ImGuiKey, F13);
			new_enum_v(ImGuiKey, F14);
			new_enum_v(ImGuiKey, F15);
			new_enum_v(ImGuiKey, F16);
			new_enum_v(ImGuiKey, F17);
			new_enum_v(ImGuiKey, F18);
			new_enum_v(ImGuiKey, F19);
			new_enum_v(ImGuiKey, F20);
			new_enum_v(ImGuiKey, F21);
			new_enum_v(ImGuiKey, F22);
			new_enum_v(ImGuiKey, F23);
			new_enum_v(ImGuiKey, F24);
			new_enum_v(ImGuiKey, Apostrophe);
			new_enum_v(ImGuiKey, Comma);
			new_enum_v(ImGuiKey, Minus);
			new_enum_v(ImGuiKey, Period);
			new_enum_v(ImGuiKey, Slash);
			new_enum_v(ImGuiKey, Semicolon);
			new_enum_v(ImGuiKey, Equal);
			new_enum_v(ImGuiKey, LeftBracket);
			new_enum_v(ImGuiKey, Backslash);
			new_enum_v(ImGuiKey, RightBracket);
			new_enum_v(ImGuiKey, GraveAccent);
			new_enum_v(ImGuiKey, CapsLock);
			new_enum_v(ImGuiKey, ScrollLock);
			new_enum_v(ImGuiKey, NumLock);
			new_enum_v(ImGuiKey, PrintScreen);
			new_enum_v(ImGuiKey, Pause);
			new_enum_v(ImGuiKey, Keypad0);
			new_enum_v(ImGuiKey, Keypad1);
			new_enum_v(ImGuiKey, Keypad2);
			new_enum_v(ImGuiKey, Keypad3);
			new_enum_v(ImGuiKey, Keypad4);
			new_enum_v(ImGuiKey, Keypad5);
			new_enum_v(ImGuiKey, Keypad6);
			new_enum_v(ImGuiKey, Keypad7);
			new_enum_v(ImGuiKey, Keypad8);
			new_enum_v(ImGuiKey, Keypad9);
			new_enum_v(ImGuiKey, KeypadDecimal);
			new_enum_v(ImGuiKey, KeypadDivide);
			new_enum_v(ImGuiKey, KeypadMultiply);
			new_enum_v(ImGuiKey, KeypadSubtract);
			new_enum_v(ImGuiKey, KeypadAdd);
			new_enum_v(ImGuiKey, KeypadEnter);
			new_enum_v(ImGuiKey, KeypadEqual);
			new_enum_v(ImGuiKey, AppBack);
			new_enum_v(ImGuiKey, AppForward);
			new_enum_v(ImGuiKey, GamepadStart);
			new_enum_v(ImGuiKey, GamepadBack);
			new_enum_v(ImGuiKey, GamepadFaceLeft);
			new_enum_v(ImGuiKey, GamepadFaceRight);
			new_enum_v(ImGuiKey, GamepadFaceUp);
			new_enum_v(ImGuiKey, GamepadFaceDown);
			new_enum_v(ImGuiKey, GamepadDpadLeft);
			new_enum_v(ImGuiKey, GamepadDpadRight);
			new_enum_v(ImGuiKey, GamepadDpadUp);
			new_enum_v(ImGuiKey, GamepadDpadDown);
			new_enum_v(ImGuiKey, GamepadL1);
			new_enum_v(ImGuiKey, GamepadR1);
			new_enum_v(ImGuiKey, GamepadL2);
			new_enum_v(ImGuiKey, GamepadR2);
			new_enum_v(ImGuiKey, GamepadL3);
			new_enum_v(ImGuiKey, GamepadR3);
			new_enum_v(ImGuiKey, GamepadLStickLeft);
			new_enum_v(ImGuiKey, GamepadLStickRight);
			new_enum_v(ImGuiKey, GamepadLStickUp);
			new_enum_v(ImGuiKey, GamepadLStickDown);
			new_enum_v(ImGuiKey, GamepadRStickLeft);
			new_enum_v(ImGuiKey, GamepadRStickRight);
			new_enum_v(ImGuiKey, GamepadRStickUp);
			new_enum_v(ImGuiKey, GamepadRStickDown);
			new_enum_v(ImGuiKey, MouseLeft);
			new_enum_v(ImGuiKey, MouseRight);
			new_enum_v(ImGuiKey, MouseMiddle);
			new_enum_v(ImGuiKey, MouseX1);
			new_enum_v(ImGuiKey, MouseX2);
			new_enum_v(ImGuiKey, MouseWheelX);
			new_enum_v(ImGuiKey, MouseWheelY);

			new_enum.set("Mod_None", ImGuiMod_None);
			new_enum.set("Mod_Ctrl", ImGuiMod_Ctrl);
			new_enum.set("Mod_Shift", ImGuiMod_Shift);
			new_enum.set("Mod_Alt", ImGuiMod_Alt);
			new_enum.set("Mod_Super", ImGuiMod_Super);
			new_enum.set("Mod_Shortcut", ImGuiMod_Shortcut);
			new_enum.set("Mod_Mask_", ImGuiMod_Mask_);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiSelectableFlags");

			new_enum_v(ImGuiSelectableFlags, None);
			new_enum_v(ImGuiSelectableFlags, DontClosePopups);
			new_enum_v(ImGuiSelectableFlags, SpanAllColumns);
			new_enum_v(ImGuiSelectableFlags, AllowDoubleClick);
			new_enum_v(ImGuiSelectableFlags, Disabled);
			new_enum_v(ImGuiSelectableFlags, AllowOverlap);

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
			new_enum_v(ImGuiSelectableFlags, AllowItemOverlap);
#endif
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiTabItemFlags");

			new_enum_v(ImGuiTabItemFlags, None);
			new_enum_v(ImGuiTabItemFlags, UnsavedDocument);
			new_enum_v(ImGuiTabItemFlags, SetSelected);
			new_enum_v(ImGuiTabItemFlags, NoCloseWithMiddleMouseButton);
			new_enum_v(ImGuiTabItemFlags, NoPushId);
			new_enum_v(ImGuiTabItemFlags, NoTooltip);
			new_enum_v(ImGuiTabItemFlags, NoReorder);
			new_enum_v(ImGuiTabItemFlags, Leading);
			new_enum_v(ImGuiTabItemFlags, Trailing);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiTableColumnFlags");
			new_enum_v(ImGuiTableColumnFlags, None);
			new_enum_v(ImGuiTableColumnFlags, Disabled);
			new_enum_v(ImGuiTableColumnFlags, DefaultHide);
			new_enum_v(ImGuiTableColumnFlags, DefaultSort);
			new_enum_v(ImGuiTableColumnFlags, WidthStretch);
			new_enum_v(ImGuiTableColumnFlags, WidthFixed);
			new_enum_v(ImGuiTableColumnFlags, NoResize);
			new_enum_v(ImGuiTableColumnFlags, NoReorder);
			new_enum_v(ImGuiTableColumnFlags, NoHide);
			new_enum_v(ImGuiTableColumnFlags, NoClip);
			new_enum_v(ImGuiTableColumnFlags, NoSort);
			new_enum_v(ImGuiTableColumnFlags, NoSortAscending);
			new_enum_v(ImGuiTableColumnFlags, NoSortDescending);
			new_enum_v(ImGuiTableColumnFlags, NoHeaderLabel);
			new_enum_v(ImGuiTableColumnFlags, NoHeaderWidth);
			new_enum_v(ImGuiTableColumnFlags, PreferSortAscending);
			new_enum_v(ImGuiTableColumnFlags, PreferSortDescending);
			new_enum_v(ImGuiTableColumnFlags, IndentEnable);
			new_enum_v(ImGuiTableColumnFlags, IndentDisable);
			new_enum_v(ImGuiTableColumnFlags, AngledHeader);
			new_enum_v(ImGuiTableColumnFlags, IsEnabled);
			new_enum_v(ImGuiTableColumnFlags, IsVisible);
			new_enum_v(ImGuiTableColumnFlags, IsSorted);
			new_enum_v(ImGuiTableColumnFlags, IsHovered);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiTableRowFlags");
			new_enum_v(ImGuiTableRowFlags, None);
			new_enum_v(ImGuiTableRowFlags, Headers);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiViewportFlags");
			new_enum_v(ImGuiViewportFlags, None);
			new_enum_v(ImGuiViewportFlags, IsPlatformWindow);
			new_enum_v(ImGuiViewportFlags, IsPlatformMonitor);
			new_enum_v(ImGuiViewportFlags, OwnedByApp);
			new_enum_v(ImGuiViewportFlags, NoDecoration);
			new_enum_v(ImGuiViewportFlags, NoTaskBarIcon);
			new_enum_v(ImGuiViewportFlags, NoFocusOnAppearing);
			new_enum_v(ImGuiViewportFlags, NoFocusOnClick);
			new_enum_v(ImGuiViewportFlags, NoInputs);
			new_enum_v(ImGuiViewportFlags, NoRendererClear);
			new_enum_v(ImGuiViewportFlags, NoAutoMerge);
			new_enum_v(ImGuiViewportFlags, TopMost);
			new_enum_v(ImGuiViewportFlags, CanHostOtherWindows);
			new_enum_v(ImGuiViewportFlags, IsMinimized);
			new_enum_v(ImGuiViewportFlags, IsFocused);
		}

		{
			ScriptEnumRegistrar new_enum("ImGuiTableBgTarget");

			new_enum_v(ImGuiTableBgTarget, None);
			new_enum_v(ImGuiTableBgTarget, RowBg0);
			new_enum_v(ImGuiTableBgTarget, RowBg1);
			new_enum_v(ImGuiTableBgTarget, CellBg);
		}

		ScriptEngine::register_typedef("ImGuiID", "uint");
		ScriptEngine::default_namespace("ImGui");

		reg_func_nw_ns("bool Begin(const string& in, Bool@ = null, int = 0)",
		               (result_wrapped_func<ImGui::Begin, bool, const String&, Boolean*, int>) );

		ScriptEngine::register_function("void End()", ImGui::End);
		ScriptEngine::register_function("void EndChild()", ImGui::EndChild);
		ScriptEngine::register_function("void EndCombo()", ImGui::EndCombo);
		ScriptEngine::register_function("void Text(const string& in)", wrapped_text);
		ScriptEngine::register_function("void TextColored(const ImVec4& in, const string& in)", wrapped_text_colored);
		ScriptEngine::register_function("void TextDisabled(const string& in)", wrapped_text_disabled);
		ScriptEngine::register_function("void TextWrapped(const string& in)", wrapped_text_wrapped);
		ScriptEngine::register_function("void IsWindowAppearing()", ImGui::IsWindowAppearing);
		ScriptEngine::register_function("void IsWindowCollapsed()", ImGui::IsWindowCollapsed);

		reg_func("bool ArrowButton(const string& in, ImGuiDir)", ArrowButton);

		{
			constexpr auto func = func_of<bool(const char*, const ImVec2&, int, int)>(ImGui::BeginChild);
			reg_func_no_ns("bool BeginChild(const string&, const ImVec2& in = ImVec2(0, 0), int = 0, int = 0)", func);
		}
		{
			constexpr auto func = func_of<bool(ImGuiID, const ImVec2&, int, int)>(ImGui::BeginChild);
			reg_func_no_ns("bool BeginChild(ImGuiID, const ImVec2& in = ImVec2(0, 0), int = 0, int = 0)", func);
		}

		reg_func("bool BeginCombo(const string& in, const string& in, int = 0)", BeginCombo);
		reg_func("bool BeginDragDropSource(int = 0)", BeginDragDropSource);
		reg_func("bool BeginDragDropTarget()", BeginDragDropTarget);
		reg_func("bool BeginItemTooltip()", BeginItemTooltip);
		reg_func("bool BeginListBox(const string& in, const ImVec2& in = ImVec2(0, 0))", BeginListBox);
		reg_func("bool BeginMainMenuBar()", BeginMainMenuBar);
		reg_func("bool BeginMenuBar()", BeginMenuBar);
		reg_func("bool BeginMenu(const string& in, bool = true)", BeginMenu);
		reg_func("bool BeginPopup(const string& in, int = 0)", BeginPopup);
		reg_func("bool BeginPopupContextItem(const string& in, int = 1)", BeginPopupContextItem);
		reg_func("bool BeginPopupContextVoid(const string& in, int = 1)", BeginPopupContextVoid);
		reg_func("bool BeginPopupContextWindow(const string& in, int = 1)", BeginPopupContextWindow);
		reg_func_nw_ns("bool BeginPopupModal(const string& in name, Bool@ = null, int = 0)",
		               (result_wrapped_func<ImGui::BeginPopupModal, bool, const String&, Boolean*, int>) );
		reg_func("bool BeginTabBar(const string& in, int = 0)", BeginTabBar);
		reg_func_nw_ns("bool BeginTabItem(const string& in, Bool@ = null, int = 0)",
		               (result_wrapped_func<ImGui::BeginTabItem, bool, const String&, Boolean*, int>) );
		reg_func("bool BeginTable(const string& in, int, int = 0, const ImVec2& = ImVec2(0.0f, 0.0f), float = 0.0f)", BeginTable);
		reg_func_nw("bool BeginTooltip()", BeginTooltip);
		reg_func("bool Button(const string& in, const ImVec2& size = ImVec2(0, 0))", Button);
		reg_func("bool Checkbox(const string& in, bool&)", Checkbox);
		reg_func_no_ns("bool CheckboxFlags(const string& in, int& flags, int flags_value)",
		               (func_of<bool(const char*, int*, int)>(ImGui::CheckboxFlags)));
		reg_func_no_ns("bool CheckboxFlags(const string& in, uint& flags, uint flags_value)",
		               (func_of<bool(const char*, unsigned int*, unsigned int)>(ImGui::CheckboxFlags)));
		reg_func_no_ns("bool CollapsingHeader(const string& in, bool& , int = 0)",
		               (func_of<bool(const char*, bool*, int)>(ImGui::CollapsingHeader)));
		reg_func_no_ns("bool CollapsingHeader(const string& in, int = 0)",
		               (func_of<bool(const char*, int)>(ImGui::CollapsingHeader)));
		reg_func("bool ColorButton(const string& in, const ImVec4& col, int = 0, const ImVec2& size = ImVec2(0, 0))",
		         ColorButton);

		reg_func("bool ColorEdit3(const string& in, Engine::Vector3D& inout, int = 0)", ColorEdit3);
		reg_func("bool ColorEdit4(const string& in, Engine::Vector4D& inout, int = 0)", ColorEdit4);
		reg_func("bool ColorPicker3(const string& in, Engine::Vector3D& inout, int = 0)", ColorPicker3);
		reg_func("bool ColorPicker4(const string& in, Engine::Vector4D& inout, int = 0)", ColorPicker4);
		reg_func_nw_ns("bool Combo(const string& in, int&, const string[]& in, int = -1)", wrapped_combo);
		reg_func("bool DebugCheckVersionAndDataLayout(const string& in, uint64, uint64, uint64, uint64, uint64, uint64)",
		         DebugCheckVersionAndDataLayout);

		reg_func("bool DragFloat2(const string&, Engine::Vector2D& inout, float = 1.0f, float = 0.0f, float = 0.0f, "
		         "const string& in = \"%.3f\", int = 0)",
		         DragFloat2);

		reg_func("bool DragFloat3(const string&, Engine::Vector3D& inout, float = 1.0f, float = 0.0f, float = 0.0f, "
		         "const string& in = \"%.3f\", int = 0)",
		         DragFloat3);

		reg_func("bool DragFloat4(const string&, Engine::Vector4D& inout, float = 1.0f, float = 0.0f, float = 0.0f, "
		         "const string& in = \"%.3f\", int = 0)",
		         DragFloat4);

		reg_func("bool DragFloat(const string&, float&, float = 1.0f, float = 0.0f, float = 0.0f, "
		         "const string& in = \"%.3f\", int = 0)",
		         DragFloat);

		reg_func("bool DragFloatRange2(const string& in, float&, float&, float = 1.0f, float = 0.0f, float "
		         "= 0.0f, const string& in = \"%.3f\", const string& in = \"%.3f\", int = 0)",
		         DragFloatRange2);


		reg_func("bool DragInt2(const string&, Engine::IntVector2D& inout, int = 1.0f, int = 0, int = 0,"
		         "const string& in = \"%.d\", int = 0)",
		         DragInt2);

		reg_func("bool DragInt3(const string&, Engine::IntVector3D& inout, int = 1.0f, int = 0, int = 0,"
		         "const string& in = \"%.d\", int = 0)",
		         DragInt3);

		reg_func("bool DragInt4(const string&, Engine::IntVector4D& inout, int = 1.0f, int = 0, int = 0,"
		         "const string& in = \"%.d\", int = 0)",
		         DragInt4);

		reg_func("bool DragInt(const string&, int&, float = 1.0f, int= 0, int = 0,"
		         "const string& in = \"%.d\", int = 0)",
		         DragInt);

		reg_func("bool DragIntRange2(const string& in, int&, int&, float = 1.0f, int = 0, int"
		         "= 0, const string& in = \"%.d\", const string& in = \"%.d\", int = 0)",
		         DragIntRange2);

		//        bool  ImageButton(const char* str_id, ImTextureID user_texture_id, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

		reg_func("bool InputDouble(const string& in, double&, double = 0.0, double = 0.0, const string& in = \"%.6f\", "
		         "int = 0)",
		         InputDouble);
		reg_func("bool InputFloat2(const string& in, Engine::Vector2D& inout, const string& in = \"%.3f\", int = 0)",
		         InputFloat2);
		reg_func("bool InputFloat3(const string& in, Engine::Vector3D& inout, const string& in = \"%.3f\", int = 0)",
		         InputFloat3);
		reg_func("bool InputFloat4(const string& in, Engine::Vector4D& inout, const string& in = \"%.3f\", int = 0)",
		         InputFloat4);
		reg_func("bool InputFloat(const string& in, float& inout, float = 0.0f, float= 0.0f, const string& in = "
		         "\"%.3f\", int = 0)",
		         InputFloat);

		reg_func("bool InputInt2(const string& in, Engine::IntVector2D& inout, int = 0)", InputInt2);
		reg_func("bool InputInt3(const string& in, Engine::IntVector3D& inout, int = 0)", InputInt3);
		reg_func("bool InputInt4(const string& in, Engine::IntVector4D& inout, int = 0)", InputInt4);
		reg_func("bool InputInt(const string& in, int& inout, int = 1, int = 100, int = 0)", InputInt);
		reg_func_nw_ns("bool InputText(const string& in, string& inout, int = 0)", wrap_input_text);
		reg_func_nw_ns("bool InputTextWithHint(const string& in, const string& in, string& inout, int = 0)",
		               wrap_input_text_hint);
		reg_func_nw_ns("bool InputTextMultiline(const string& in, string& inout, const ImVec2& size = ImVec2(0, 0), int = 0)",
		               wrap_input_text_multiline);
		reg_func("bool InvisibleButton(const string&, const ImVec2&, int = 0)", InvisibleButton);
		reg_func("bool IsAnyItemActive()", IsAnyItemActive);
		reg_func("bool IsAnyItemFocused()", IsAnyItemFocused);
		reg_func("bool IsAnyItemHovered()", IsAnyItemHovered);
		reg_func("bool IsAnyMouseDown()", IsAnyMouseDown);
		reg_func("bool IsItemActivated()", IsItemActivated);
		reg_func("bool IsItemActive()", IsItemActive);
		reg_func("bool IsItemClicked(ImGuiMouseButton = 0)", IsItemClicked);
		reg_func("bool IsItemDeactivated()", IsItemDeactivated);
		reg_func("bool IsItemDeactivatedAfterEdit()", IsItemDeactivatedAfterEdit);
		reg_func("bool IsItemEdited()", IsItemFocused);
		reg_func("bool IsItemFocused()", IsItemFocused);
		reg_func("bool IsItemHovered(int = 0)", IsItemHovered);
		reg_func("bool IsItemToggledOpen()", IsItemToggledOpen);
		reg_func("bool IsItemVisible()", IsItemVisible);
		reg_func_nw("bool IsKeyChordPressed(int)", IsKeyChordPressed);
		reg_func_nw("bool IsKeyDown(ImGuiKey)", IsKeyDown);
		reg_func_nw("bool IsKeyPressed(ImGuiKey, bool = true)", IsKeyPressed);
		reg_func_nw("bool IsKeyReleased(ImGuiKey)", IsKeyReleased);
		reg_func_nw("bool IsMouseClicked(ImGuiMouseButton, bool = false)", IsMouseClicked);
		reg_func_nw("bool IsMouseDoubleClicked(ImGuiMouseButton)", IsMouseDoubleClicked);
		reg_func_nw("bool IsMouseDown(ImGuiMouseButton)", IsMouseDown);
		reg_func_nw("bool IsMouseDragging(ImGuiMouseButton, float = -1.0f)", IsMouseDragging);
		reg_func_nw("bool IsMouseHoveringRect(const ImVec2&, const ImVec2&, bool = true)", IsMouseHoveringRect);
		reg_func_nw_ns("bool IsMousePosValid()", is_mouse_pos_valid_wrap1);
		reg_func_nw_ns("bool IsMousePosValid(const ImVec2&)", is_mouse_pos_valid_wrap2);
		reg_func_nw("bool  IsMouseReleased(ImGuiMouseButton)", IsMouseReleased);
		reg_func("bool IsPopupOpen(const string& in, int = 0)", IsPopupOpen);
		reg_func_no_ns("bool IsRectVisible(const ImVec2& in, const ImVec2& in)",
		               func_of<bool(const ImVec2&, const ImVec2&)>(ImGui::IsRectVisible));
		reg_func_no_ns("bool IsRectVisible(const ImVec2& in)", (func_of<bool(const ImVec2&)>(ImGui::IsRectVisible)));
		reg_func("bool IsWindowDocked()", IsWindowDocked);
		reg_func_nw("bool IsWindowFocused(int=0)", IsWindowFocused);
		reg_func_nw("bool IsWindowHovered(int=0)", IsWindowHovered);
		//        bool  ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
		//        bool  ListBox(const char* label, int* current_item, const char* (*getter)(void* user_data, int idx), void* user_data, int items_count, int height_in_items = -1);
		//        bool  MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);
		//        bool  MenuItem(const char* label, const char* shortcut = NULL, bool selected = false, bool enabled = true);
		reg_func_nw_ns("bool RadioButton(const string& in, bool)", func_of<bool(const char*, bool)>(ImGui::RadioButton));
		//        bool  RadioButton(const char* label, int* v, int v_button);
		//        bool  Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
		//        bool  Selectable(const char* label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0));
		reg_func("bool ShowStyleSelector(const string& in)", ShowStyleSelector);
		//        bool  SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", ImGuiSliderFlags flags = 0);
		//        bool  SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
		//        bool  SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
		//        bool  SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
		//        bool  SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
		//        bool  SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
		//        bool  SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
		//        bool  SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
		//        bool  SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
		reg_func("bool SmallButton(const string& in)", SmallButton);
		reg_func("bool TabItemButton(const string& in, int = 0)", TabItemButton);
		reg_func_nw("bool TableNextColumn()", TableNextColumn);
		reg_func_nw("bool TableSetColumnIndex(int)", TableSetColumnIndex);
		reg_func_no_ns("bool TreeNode(const string& in)", (func_of<bool(const char*)>(ImGui::TreeNode)));
		reg_func_no_ns("bool TreeNodeEx(const string& in, int = 0)",
		               func_of<bool(const char*, ImGuiTreeNodeFlags)>(ImGui::TreeNodeEx));
		reg_func_nw_ns(
		        "bool  TreeNodeEx(const string& in, int flags, const string& in)",
		        func_of<bool(const String&, int, const String&)>([](const String& id, int flags, const String& fmt) -> bool {
			        return ImGui::TreeNodeEx(id.c_str(), flags, "%s", fmt.c_str());
		        }));

		//        bool  VSliderFloat(const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
		//        bool  VSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
		reg_func("string GetClipboardText()", GetClipboardText);
		reg_func("string GetKeyName(ImGuiKey)", GetKeyName);
		reg_func("string GetStyleColorName(int)", GetStyleColorName);
		reg_func("string GetVersion()", GetVersion);
		//const char*   SaveIniSettingsToMemory(size_t* out_ini_size = NULL);
		reg_func("string TableGetColumnName(int column_n = -1)", TableGetColumnName);
		reg_func_nw("const ImVec4& GetStyleColorVec4(int)", GetStyleColorVec4);
		ScriptEngine::register_function("double GetTime()", make_wrap<ImGui::GetTime>());
		ScriptEngine::register_function("float CalcItemWidth()", make_wrap<ImGui::CalcItemWidth>());
		ScriptEngine::register_function("float GetColumnOffset(int = -1)", make_wrap<ImGui::GetColumnOffset>());
		ScriptEngine::register_function("float GetColumnWidth(int = -1)", make_wrap<ImGui::GetColumnWidth>());
		ScriptEngine::register_function("float GetCursorPosX()", make_wrap<ImGui::GetCursorPosX>());
		ScriptEngine::register_function("float GetCursorPosY()", make_wrap<ImGui::GetCursorPosY>());
		ScriptEngine::register_function("float GetFontSize()", make_wrap<ImGui::GetFontSize>());
		ScriptEngine::register_function("float GetFrameHeight()", make_wrap<ImGui::GetFrameHeight>());
		ScriptEngine::register_function("float GetFrameHeightWithSpacing()", make_wrap<ImGui::GetFrameHeightWithSpacing>());
		ScriptEngine::register_function("float GetScrollMaxX()", make_wrap<ImGui::GetScrollMaxX>());
		ScriptEngine::register_function("float GetScrollMaxY()", make_wrap<ImGui::GetScrollMaxY>());
		ScriptEngine::register_function("float GetScrollX()", make_wrap<ImGui::GetScrollX>());
		ScriptEngine::register_function("float GetScrollY()", make_wrap<ImGui::GetScrollY>());
		ScriptEngine::register_function("float GetTextLineHeight()", make_wrap<ImGui::GetTextLineHeight>());
		ScriptEngine::register_function("float GetTextLineHeightWithSpacing()", make_wrap<ImGui::GetTextLineHeightWithSpacing>());
		ScriptEngine::register_function("float GetTreeNodeToLabelSpacing()", make_wrap<ImGui::GetTreeNodeToLabelSpacing>());
		ScriptEngine::register_function("float GetWindowDpiScale()", make_wrap<ImGui::GetWindowDpiScale>());
		ScriptEngine::register_function("float GetWindowHeight()", make_wrap<ImGui::GetWindowHeight>());
		ScriptEngine::register_function("float GetWindowWidth()", make_wrap<ImGui::GetWindowWidth>());
		//        ImGuiID       DockSpace(ImGuiID id, const ImVec2& size = ImVec2(0, 0), ImGuiDockNodeFlags flags = 0, const ImGuiWindowClass* window_class = NULL);
		//        ImGuiID       DockSpaceOverViewport(const ImGuiViewport* viewport = NULL, ImGuiDockNodeFlags flags = 0, const ImGuiWindowClass* window_class = NULL);
		reg_func_no_ns("ImGuiID GetID(const string& in)", func_of<ImGuiID(const char*)>(ImGui::GetID));
		reg_func_nw("ImGuiID GetItemID()", GetItemID);
		reg_func_nw("ImGuiID GetWindowDockID()", GetWindowDockID);
		reg_func_nw("int GetMouseCursor()", GetMouseCursor);
		reg_func_nw("int TableGetColumnFlags(int = -1)", TableGetColumnFlags);
		reg_func_nw("uint ColorConvertFloat4ToU32(const ImVec4& )", ColorConvertFloat4ToU32);
		reg_func_nw_ns("uint GetColorU32(const ImVec4& )", (func_of<ImU32(const ImVec4&)>(ImGui::GetColorU32)));
		reg_func_nw_ns("uint GetColorU32(int, float = 1.0f)", (func_of<ImU32(ImGuiCol, float)>(ImGui::GetColorU32)));
		reg_func_nw_ns("uint GetColorU32(uint)", (func_of<ImU32(ImU32)>(ImGui::GetColorU32)));
		reg_func_nw_ns("ImVec2 CalcTextSize(const string& in, bool = false, float = -1.0f)",
		               func_of<ImVec2(const String&, bool, float)>([](const String& text, bool hide, float wrap) -> ImVec2 {
			               return ImGui::CalcTextSize(text.c_str(), nullptr, hide, wrap);
		               }));
		reg_func_nw("ImVec2 GetContentRegionAvail()", GetContentRegionAvail);
		reg_func_nw("ImVec2 GetContentRegionMax()", GetContentRegionMax);
		reg_func_nw("ImVec2 GetCursorPos()", GetCursorPos);
		reg_func_nw("ImVec2 GetCursorScreenPos()", GetCursorScreenPos);
		reg_func_nw("ImVec2 GetCursorStartPos()", GetCursorStartPos);
		reg_func_nw("ImVec2 GetFontTexUvWhitePixel()", GetFontTexUvWhitePixel);
		reg_func_nw("ImVec2 GetItemRectMax()", GetItemRectMax);
		reg_func_nw("ImVec2 GetItemRectMin()", GetItemRectMin);
		reg_func_nw("ImVec2 GetItemRectSize()", GetItemRectSize);
		reg_func_nw("ImVec2 GetMouseDragDelta(ImGuiMouseButton = 0, float = -1.0f)", GetMouseDragDelta);
		reg_func_nw("ImVec2 GetMousePos()", GetMousePos);
		reg_func_nw("ImVec2 GetMousePosOnOpeningCurrentPopup()", GetMousePosOnOpeningCurrentPopup);
		reg_func_nw("ImVec2 GetWindowContentRegionMax()", GetWindowContentRegionMax);
		reg_func_nw("ImVec2 GetWindowContentRegionMin()", GetWindowContentRegionMin);
		reg_func_nw("ImVec2 GetWindowPos()", GetWindowPos);
		reg_func_nw("ImVec2 GetWindowSize()", GetWindowSize);
		reg_func_nw("ImVec4 ColorConvertU32ToFloat4(uint)", ColorConvertU32ToFloat4);
		reg_func_nw("int GetColumnIndex()", GetColumnIndex);
		reg_func_nw("int GetColumnsCount()", GetColumnsCount);
		reg_func_nw("int GetFrameCount()", GetFrameCount);
		reg_func_nw("int GetKeyPressedAmount(ImGuiKey key, float repeat_delay, float rate)", GetKeyPressedAmount);
		reg_func_nw("int GetMouseClickedCount(ImGuiMouseButton)", GetMouseClickedCount);
		reg_func_nw("int TableGetColumnCount()", TableGetColumnCount);
		reg_func_nw("int TableGetColumnIndex()", TableGetColumnIndex);
		reg_func_nw("int TableGetRowIndex()", TableGetRowIndex);
		reg_func_nw("void AlignTextToFramePadding()", AlignTextToFramePadding);
		reg_func_nw("void BeginDisabled(bool = true)", BeginDisabled);
		reg_func_nw("void BeginGroup()", BeginGroup);
		reg_func_nw("void Bullet()", Bullet);
		reg_func_nw_ns("void BulletText(const string& in)", bullet_text);
		reg_func_nw("void CloseCurrentPopup()", CloseCurrentPopup);
		reg_func_nw("void ColorConvertHSVtoRGB(float, float, float, float& out, float& out, float& out)", ColorConvertHSVtoRGB);
		reg_func_nw("void ColorConvertRGBtoHSV(float, float, float, float& out, float& out, float& out)", ColorConvertRGBtoHSV);
		reg_func("void Columns(int, const string& in, bool= true)", Columns);
		reg_func_nw_ns("void Columns(int = 1)", static_cast<void (*)(int)>(([](int value) { ImGui::Columns(value); })));
		reg_func_nw("void DebugFlashStyleColor(int)", DebugFlashStyleColor);
		reg_func("void DebugTextEncoding(const string& in)", DebugTextEncoding);
		reg_func_nw("void Dummy(const ImVec2& in size)", Dummy);
		reg_func_nw("void EndDisabled()", EndDisabled);
		reg_func_nw("void EndDragDropSource()", EndDragDropSource);
		reg_func_nw("void EndDragDropTarget()", EndDragDropTarget);
		reg_func_nw("void EndGroup()", EndGroup);
		reg_func_nw("void EndListBox()", EndListBox);
		reg_func_nw("void EndMainMenuBar()", EndMainMenuBar);
		reg_func_nw("void EndMenu()", EndMenu);
		reg_func_nw("void EndMenuBar()", EndMenuBar);
		reg_func_nw("void EndPopup()", EndPopup);
		reg_func_nw("void EndTabBar()", EndTabBar);
		reg_func_nw("void EndTabItem()", EndTabItem);
		reg_func_nw("void EndTable()", EndTable);
		reg_func_nw("void EndTooltip()", EndTooltip);
		//        void  Image(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
		reg_func_nw("void Indent(float = 0.0f)", Indent);
		reg_func_nw_ns("void LabelText(const string& in, const string& in fmt)",
		               func_of<void(const String& label, const String& fmt)>([](const String& label, const String& fmt) {
			               ImGui::LabelText(label.c_str(), "%s", fmt.c_str());
		               }));

		reg_func("void LoadIniSettingsFromDisk(const string& in)", LoadIniSettingsFromDisk);
		reg_func("void LoadIniSettingsFromMemory(const string& in, uint64 =0)", LoadIniSettingsFromMemory);
		reg_func_nw("void  LogButtons()", LogButtons);
		reg_func_nw("void  LogFinish()", LogFinish);
		reg_func_nw("void NewLine()", NewLine);
		reg_func_nw("void NextColumn()", NextColumn);
		//        void  OpenPopup(const char* str_id, ImGuiPopupFlags popup_flags = 0);
		//        void  OpenPopup(ImGuiID id, ImGuiPopupFlags popup_flags = 0);
		//        void  OpenPopupOnItemClick(const char* str_id = NULL, ImGuiPopupFlags popup_flags = 1);
		//        void  PlotHistogram(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
		//        void  PlotHistogram(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));
		//        void  PlotLines(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float));
		//        void  PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0));
		reg_func_nw("void PopButtonRepeat()", PopButtonRepeat);
		reg_func_nw("void PopClipRect()", PopClipRect);
		reg_func_nw("void PopFont()", PopFont);
		reg_func_nw("void PopID()", PopID);
		reg_func_nw("void PopItemWidth()", PopItemWidth);
		reg_func_nw("void PopStyleColor(int = 1)", PopStyleColor);
		reg_func_nw("void PopStyleVar(int = 1)", PopStyleVar);
		reg_func_nw("void PopTabStop()", PopTabStop);
		reg_func_nw("void PopTextWrapPos()", PopTextWrapPos);

		//        void ProgressBar(float fraction, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0), const char* overlay = NULL);
		//        void PushButtonRepeat(bool repeat);
		//        void PushClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect);
		//        void PushFont(ImFont* font);
		//        void PushID(const char* str_id);
		//        void PushID(const char* str_id_begin, const char* str_id_end);
		//        void PushID(const void* ptr_id);
		//        void PushID(int int_id);
		//        void PushItemWidth(float item_width);
		//        void PushStyleColor(ImGuiCol idx, const ImVec4& col);
		//        void PushStyleColor(ImGuiCol idx, ImU32 col);
		//        void PushStyleVar(ImGuiStyleVar idx, const ImVec2& val);
		//        void PushStyleVar(ImGuiStyleVar idx, float val);
		//        void PushTabStop(bool tab_stop);
		//        void PushTextWrapPos(float wrap_local_pos_x = 0.0f);
		//        void ResetMouseDragDelta(ImGuiMouseButton button = 0);
		//        void SameLine(float offset_from_start_x=0.0f, float spacing=-1.0f);
		//        void SaveIniSettingsToDisk(const char* ini_filename);
		reg_func_nw("void Separator()", Separator);
		reg_func("void SeparatorText(const string& in)", SeparatorText);
		reg_func("void SetClipboardText(const string& in)", SetClipboardText);
		//        void SetColorEditOptions(ImGuiColorEditFlags flags);
		reg_func_nw("void SetColumnOffset(int, float)", SetColumnOffset);
		reg_func_nw("void SetColumnWidth(int, float)", SetColumnWidth);
		reg_func_nw("void SetCursorPos(const ImVec2&)", SetCursorPos);
		reg_func_nw("void SetCursorPosX(float)", SetCursorPosX);
		reg_func_nw("void SetCursorPosY(float)", SetCursorPosY);
		reg_func_nw("void SetCursorScreenPos(const ImVec2&)", SetCursorScreenPos);
		reg_func_nw("void SetItemDefaultFocus()", SetItemDefaultFocus);
		reg_func_nw_ns("void SetItemTooltip(const string& in)",
		               func_of<void(const String&)>([](const String& fmt) { ImGui::SetItemTooltip("%s", fmt.c_str()); }));
		reg_func_nw("void SetKeyboardFocusHere(int = 0)", SetKeyboardFocusHere);
		//        void SetMouseCursor(ImGuiMouseCursor cursor_type);
		reg_func_nw("void SetNextFrameWantCaptureKeyboard(bool)", SetNextFrameWantCaptureKeyboard);
		reg_func_nw("void SetNextFrameWantCaptureMouse(bool)", SetNextFrameWantCaptureMouse);

		reg_func_nw("void SetNextItemAllowOverlap()", SetNextItemAllowOverlap);
		reg_func_nw("void SetNextItemOpen(bool, ImGuiCond = 0)", SetNextItemOpen);
		reg_func_nw("void SetNextItemWidth(float)", SetNextItemWidth);
		reg_func_nw("void SetNextWindowBgAlpha(float)", SetNextWindowBgAlpha);
		reg_func_nw("void SetNextWindowCollapsed(bool, ImGuiCond = 0)", SetNextWindowCollapsed);
		reg_func_nw("void SetNextWindowContentSize(const ImVec2& size)", SetNextWindowContentSize);
		reg_func_nw("void SetNextWindowDockID(ImGuiID, ImGuiCond = 0)", SetNextWindowDockID);
		reg_func_nw("void SetNextWindowFocus()", SetNextWindowFocus);
		reg_func_nw("void SetNextWindowPos(const ImVec2& pos, ImGuiCond = 0, const ImVec2& pivot = ImVec2(0, 0))",
		            SetNextWindowPos);
		reg_func_nw("void SetNextWindowScroll(const ImVec2& scroll)", SetNextWindowScroll);
		reg_func_nw("void SetNextWindowSize(const ImVec2& size, int = 0)", SetNextWindowSize);
		//        void SetNextWindowSizeConstraints(const ImVec2& size_min, const ImVec2& size_max, ImGuiSizeCallback custom_callback = NULL, void* custom_callback_data = NULL);
		reg_func_nw("void SetScrollFromPosX(float, float = 0.5f)", SetScrollFromPosX);
		reg_func_nw("void SetScrollFromPosY(float, float = 0.5f)", SetScrollFromPosY);
		reg_func_nw("void SetScrollHereX(float = 0.5f)", SetScrollHereX);
		reg_func_nw("void SetScrollHereY(float = 0.5f)", SetScrollHereY);
		reg_func_nw("void SetScrollX(float)", SetScrollX);
		reg_func_nw("void SetScrollY(float)", SetScrollY);
		reg_func("void SetTabItemClosed(const string& in)", SetTabItemClosed);
		reg_func_nw_ns("void SetToolTip(const string& in)", set_tooltip);
		reg_func_nw_ns("void SetWindowCollapsed(bool collapsed, int= 0)", (func_of<void(bool, int)>(ImGui::SetWindowCollapsed)));
		reg_func_no_ns("void SetWindowCollapsed(const string& in, bool collapsed, int= 0)",
		               (func_of<void(bool, int)>(ImGui::SetWindowCollapsed)));
		reg_func_nw_ns("void  SetWindowFocus()", (func_of<void()>(ImGui::SetWindowFocus)));
		reg_func_no_ns("void  SetWindowFocus(const string& in)", (func_of<void(const char*)>(ImGui::SetWindowFocus)));
		reg_func_nw("void SetWindowFontScale(float)", SetWindowFontScale);
		reg_func_no_ns("void SetWindowPos(const string& in, const ImVec2& , int = 0)",
		               (func_of<void(const char*, const ImVec2&, int)>(ImGui::SetWindowPos)));
		reg_func_nw_ns("void SetWindowPos(const ImVec2& , int = 0)", (func_of<void(const ImVec2&, int)>(ImGui::SetWindowPos)));
		reg_func_no_ns("void SetWindowSize(const string& in, const ImVec2& , int = 0)",
		               (func_of<void(const char*, const ImVec2&, int)>(ImGui::SetWindowSize)));
		reg_func_nw_ns("void SetWindowSize(const ImVec2& , int = 0)", (func_of<void(const ImVec2&, int)>(ImGui::SetWindowSize)));
		reg_func_nw_ns("void ShowAboutWindow(Bool@ = null)", (result_wrapped_func<ImGui::ShowAboutWindow, void, Boolean*>) );
		reg_func_nw_ns("void ShowDebugLogWindow(Bool@ = null)",
		               (result_wrapped_func<ImGui::ShowDebugLogWindow, void, Boolean*>) );
		reg_func_nw_ns("void ShowDemoWindow(Bool@ = null)", (result_wrapped_func<ImGui::ShowDemoWindow, void, Boolean*>) );
		reg_func("void ShowFontSelector(const string& in)", ShowFontSelector);
		reg_func_nw_ns("void ShowIDStackToolWindow(Bool@ = null)",
		               (result_wrapped_func<ImGui::ShowIDStackToolWindow, void, Boolean*>) );
		reg_func_nw_ns("void ShowMetricsWindow(Bool@ = null)", (result_wrapped_func<ImGui::ShowMetricsWindow, void, Boolean*>) );
		reg_func_nw_ns("void ShowStyleEditor()", wrapped_show_style_editor);
		reg_func_nw("void ShowUserGuide()", ShowUserGuide);
		reg_func_nw("void Spacing()", Spacing);
		reg_func_nw("void TableAngledHeadersRow()", TableAngledHeadersRow);
		reg_func("void TableHeader(const string& in)", TableHeader);
		reg_func_nw("void TableHeadersRow()", TableHeadersRow);
		reg_func_nw("void TableNextRow(int= 0, float= 0.0f)", TableHeadersRow);
		reg_func_nw("void TableSetColumnEnabled(int, bool)", TableSetColumnEnabled);
		reg_func_nw("void TableSetBgColor(int target, uint color, int = -1)", TableSetBgColor);
		reg_func("void TableSetupColumn(const string& in, int = 0, float = 0.0f, ImGuiID = 0)", TableSetupColumn);
		reg_func("void TableSetupScrollFreeze(int, int)", TableSetupScrollFreeze);
		reg_func("void TextUnformatted(const string& in , const string& in)", TextUnformatted);
		reg_func_nw("void TreePop()", TreePop);

		reg_func_no_ns("void TreePush(const string& in)", (func_of<void(const char*)>(ImGui::TreePush)));
		ScriptEngine::register_function("void Unindent(float = 0.0)", make_wrap<ImGui::Unindent>());
		ScriptEngine::register_function("void Value(const string& in, bool)",
		                                make_wrap<func_of<void(const char*, bool)>(ImGui::Value)>());

		ScriptEngine::register_function("void Value(const string& in, float, const string& in)",
		                                make_wrap<func_of<void(const char*, float, const char*)>(ImGui::Value)>());

		ScriptEngine::register_function("void Value(const string& in, int)",
		                                make_wrap<func_of<void(const char*, int)>(ImGui::Value)>());
		ScriptEngine::register_function("void Value(const string& in, uint)",
		                                make_wrap<func_of<void(const char*, unsigned int)>(ImGui::Value)>());
		ScriptEngine::default_namespace("");
	}

	static ReflectionInitializeController initializer(on_init, "ImGui",
	                                                  {"Engine::Vector", "Engine::IntVector", "Engine::UIntVector"});
}// namespace Engine
