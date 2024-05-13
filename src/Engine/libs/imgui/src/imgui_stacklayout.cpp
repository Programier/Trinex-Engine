
// Stack Layout implementation from https://github.com/thedmd/imgui/commit/6f6052c3822ba163c024b0d2e30e686876db8aec

#include <cinttypes>
#include <imgui_internal.h>
#include <imgui_stacklayout.h>

namespace ImGui
{
    //-----------------------------------------------------------------------------
    // [SECTION] STACK LAYOUT
    //-----------------------------------------------------------------------------

    static ImGuiLayout* FindLayout(ImGuiID id, ImGuiLayoutType type);
    static ImGuiLayout* CreateNewLayout(ImGuiID id, ImGuiLayoutType type, ImVec2 size);
    static void BeginLayout(ImGuiID id, ImGuiLayoutType type, ImVec2 size, float align);
    static void EndLayout(ImGuiLayoutType type);
    static void PushLayout(ImGuiLayout* layout);
    static void PopLayout(ImGuiLayout* layout);
    static void BalanceLayoutSprings(ImGuiLayout& layout);
    static ImVec2 BalanceLayoutItemAlignment(ImGuiLayout& layout, ImGuiLayoutItem& item);
    static void BalanceLayoutItemsAlignment(ImGuiLayout& layout);
    static void BalanceChildLayouts(ImGuiLayout& layout);
    static ImVec2 CalculateLayoutSize(ImGuiLayout& layout, bool collapse_springs);
    static ImGuiLayoutItem* GenerateLayoutItem(ImGuiLayout& layout, ImGuiLayoutItemType type);
    static float CalculateLayoutItemAlignmentOffset(ImGuiLayout& layout, ImGuiLayoutItem& item);
    static void TranslateLayoutItem(ImGuiLayoutItem& item, const ImVec2& offset);
    static void BeginLayoutItem(ImGuiLayout& layout);
    static void EndLayoutItem(ImGuiLayout& layout);
    static void AddLayoutSpring(ImGuiLayout& layout, float weight, float spacing);
    static void SignedIndent(float indent);


    static ImGuiLayout* FindLayout(ImGuiID id, ImGuiLayoutType type)
    {
        IM_ASSERT(type == ImGuiLayoutType_Horizontal || type == ImGuiLayoutType_Vertical);

        ImGuiWindow* window = GetCurrentWindow();
        ImGuiLayout* layout = (ImGuiLayout*) window->DC.Layouts.GetVoidPtr(id);
        if (!layout)
            return NULL;

        if (layout->Type != type)
        {
            layout->Type        = type;
            layout->MinimumSize = ImVec2(0.0f, 0.0f);
            layout->Items.clear();
        }

        return layout;
    }

    static ImGuiLayout* CreateNewLayout(ImGuiID id, ImGuiLayoutType type, ImVec2 size)
    {
        IM_ASSERT(type == ImGuiLayoutType_Horizontal || type == ImGuiLayoutType_Vertical);

        ImGuiWindow* window = GetCurrentWindow();

        ImGuiLayout* layout = IM_NEW(ImGuiLayout)(id, type);
        layout->Size        = size;

        window->DC.Layouts.SetVoidPtr(id, layout);

        return layout;
    }

    static void BeginLayout(ImGuiID id, ImGuiLayoutType type, ImVec2 size, float align)
    {
        ImGuiWindow* window = GetCurrentWindow();

        PushID(id);

        // Find or create
        ImGuiLayout* layout = FindLayout(id, type);
        if (!layout)
            layout = CreateNewLayout(id, type, size);

        layout->Live = true;

        PushLayout(layout);

        if (layout->Size.x != size.x || layout->Size.y != size.y)
            layout->Size = size;

        if (align < 0.0f)
            layout->Align = -1.0f;
        else
            layout->Align = ImClamp(align, 0.0f, 1.0f);

        // Start capture
        layout->CurrentItemIndex = 0;

        layout->CurrentSize.x = layout->Size.x > 0.0f ? layout->Size.x : layout->MinimumSize.x;
        layout->CurrentSize.y = layout->Size.y > 0.0f ? layout->Size.y : layout->MinimumSize.y;

        layout->StartPos          = window->DC.CursorPos;
        layout->StartCursorMaxPos = window->DC.CursorMaxPos;

        if (type == ImGuiLayoutType_Vertical)
        {
            // Push empty item to recalculate cursor position.
            PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
            Dummy(ImVec2(0.0f, 0.0f));
            PopStyleVar();

            // Indent horizontal position to match edge of the layout.
            layout->Indent = layout->StartPos.x - window->DC.CursorPos.x;
            SignedIndent(layout->Indent);
        }

        BeginLayoutItem(*layout);
    }

    static void EndLayout(ImGuiLayoutType type)
    {
        ImGuiWindow* window = GetCurrentWindow();
        IM_ASSERT(window->DC.CurrentLayout);
        IM_ASSERT(window->DC.CurrentLayout->Type == type);
        IM_UNUSED(type);

        ImGuiLayout* layout = window->DC.CurrentLayout;

        EndLayoutItem(*layout);

        if (layout->CurrentItemIndex < layout->Items.Size)
            layout->Items.resize(layout->CurrentItemIndex);

        if (layout->Type == ImGuiLayoutType_Vertical)
            SignedIndent(-layout->Indent);

        PopLayout(layout);

        const bool auto_width  = layout->Size.x <= 0.0f;
        const bool auto_height = layout->Size.y <= 0.0f;

        ImVec2 new_size = layout->Size;
        if (auto_width)
            new_size.x = layout->CurrentSize.x;
        if (auto_height)
            new_size.y = layout->CurrentSize.y;

        ImVec2 new_minimum_size = CalculateLayoutSize(*layout, true);

        if (new_minimum_size.x != layout->MinimumSize.x || new_minimum_size.y != layout->MinimumSize.y)
        {
            layout->MinimumSize = new_minimum_size;

            // Shrink
            if (auto_width)
                new_size.x = new_minimum_size.x;
            if (auto_height)
                new_size.y = new_minimum_size.y;
        }

        if (!auto_width)
            new_size.x = layout->Size.x;
        if (!auto_height)
            new_size.y = layout->Size.y;

        layout->CurrentSize = new_size;

        ImVec2 measured_size = new_size;
        if ((auto_width || auto_height) && layout->Parent)
        {
            if (layout->Type == ImGuiLayoutType_Horizontal && auto_width && layout->Parent->CurrentSize.x > 0)
                layout->CurrentSize.x = layout->Parent->CurrentSize.x;
            else if (layout->Type == ImGuiLayoutType_Vertical && auto_height && layout->Parent->CurrentSize.y > 0)
                layout->CurrentSize.y = layout->Parent->CurrentSize.y;

            BalanceLayoutSprings(*layout);

            measured_size = layout->CurrentSize;
        }

        layout->CurrentSize = new_size;

        PopID();

        ImVec2 current_layout_item_max = ImVec2(0.0f, 0.0f);
        if (window->DC.CurrentLayoutItem)
            current_layout_item_max = ImMax(window->DC.CurrentLayoutItem->MeasuredBounds.Max, layout->StartPos + new_size);

        window->DC.CursorPos    = layout->StartPos;
        window->DC.CursorMaxPos = layout->StartCursorMaxPos;
        ItemSize(new_size);
        ItemAdd(ImRect(layout->StartPos, layout->StartPos + measured_size), 0);

        if (window->DC.CurrentLayoutItem)
            window->DC.CurrentLayoutItem->MeasuredBounds.Max = current_layout_item_max;

        if (layout->Parent == NULL)
            BalanceChildLayouts(*layout);

        //window->DrawList->AddRect(layout->StartPos, layout->StartPos + measured_size, IM_COL32(0,255,0,255));           // [DEBUG]
        //window->DrawList->AddRect(window->DC.LastItemRect.Min, window->DC.LastItemRect.Max, IM_COL32(255,255,0,255));   // [DEBUG]
    }

    static ImVec2 CalculateLayoutSize(ImGuiLayout& layout, bool collapse_springs)
    {
        ImVec2 bounds = ImVec2(0.0f, 0.0f);

        if (layout.Type == ImGuiLayoutType_Vertical)
        {
            for (int i = 0; i < layout.Items.Size; i++)
            {
                ImGuiLayoutItem& item = layout.Items[i];
                ImVec2 item_size      = item.MeasuredBounds.GetSize();

                if (item.Type == ImGuiLayoutItemType_Item)
                {
                    bounds.x = ImMax(bounds.x, item_size.x);
                    bounds.y += item_size.y;
                }
                else
                {
                    bounds.y += ImFloor(item.SpringSpacing);

                    if (!collapse_springs)
                        bounds.y += item.SpringSize;
                }
            }
        }
        else
        {
            for (int i = 0; i < layout.Items.Size; i++)
            {
                ImGuiLayoutItem& item = layout.Items[i];
                ImVec2 item_size      = item.MeasuredBounds.GetSize();

                if (item.Type == ImGuiLayoutItemType_Item)
                {
                    bounds.x += item_size.x;
                    bounds.y = ImMax(bounds.y, item_size.y);
                }
                else
                {
                    bounds.x += ImFloor(item.SpringSpacing);

                    if (!collapse_springs)
                        bounds.x += item.SpringSize;
                }
            }
        }

        return bounds;
    }

    static void PushLayout(ImGuiLayout* layout)
    {
        ImGuiWindow* window = GetCurrentWindow();

        if (layout)
        {
            layout->Parent = window->DC.CurrentLayout;
            if (layout->Parent != NULL)
                layout->ParentItemIndex = layout->Parent->CurrentItemIndex;
            if (window->DC.CurrentLayout)
            {
                layout->NextSibling                  = window->DC.CurrentLayout->FirstChild;
                layout->FirstChild                   = NULL;
                window->DC.CurrentLayout->FirstChild = layout;
            }
            else
            {
                layout->NextSibling = NULL;
                layout->FirstChild  = NULL;
            }
        }

        window->DC.LayoutStack.push_back(layout);
        window->DC.CurrentLayout     = layout;
        window->DC.CurrentLayoutItem = NULL;
    }

    static void PopLayout(ImGuiLayout* layout)
    {
        ImGuiWindow* window = GetCurrentWindow();

        IM_ASSERT(!window->DC.LayoutStack.empty());
        IM_ASSERT(window->DC.LayoutStack.back() == layout);
        IM_UNUSED(layout);

        window->DC.LayoutStack.pop_back();

        if (!window->DC.LayoutStack.empty())
        {
            window->DC.CurrentLayout     = window->DC.LayoutStack.back();
            window->DC.CurrentLayoutItem = &window->DC.CurrentLayout->Items[window->DC.CurrentLayout->CurrentItemIndex];
        }
        else
        {
            window->DC.CurrentLayout     = NULL;
            window->DC.CurrentLayoutItem = NULL;
        }
    }

    static void BalanceLayoutSprings(ImGuiLayout& layout)
    {
        // Accumulate amount of occupied space and springs weights
        float total_spring_weight = 0.0f;

        int last_spring_item_index = -1;
        for (int i = 0; i < layout.Items.Size; i++)
        {
            ImGuiLayoutItem& item = layout.Items[i];
            if (item.Type == ImGuiLayoutItemType_Spring)
            {
                total_spring_weight += item.SpringWeight;
                last_spring_item_index = i;
            }
        }

        // Determine occupied space and available space depending on layout type
        const bool is_horizontal   = (layout.Type == ImGuiLayoutType_Horizontal);
        const bool is_auto_sized   = ((is_horizontal ? layout.Size.x : layout.Size.y) <= 0.0f) && (layout.Parent == NULL);
        const float occupied_space = is_horizontal ? layout.MinimumSize.x : layout.MinimumSize.y;
        const float available_space =
                is_auto_sized ? occupied_space : (is_horizontal ? layout.CurrentSize.x : layout.CurrentSize.y);
        const float free_space = ImMax(available_space - occupied_space, 0.0f);

        float span_start     = 0.0f;
        float current_weight = 0.0f;
        for (int i = 0; i < layout.Items.Size; i++)
        {
            ImGuiLayoutItem& item = layout.Items[i];
            if (item.Type != ImGuiLayoutItemType_Spring)
                continue;

            float last_spring_size = item.SpringSize;

            if (free_space > 0.0f && total_spring_weight > 0.0f)
            {
                float next_weight = current_weight + item.SpringWeight;
                float span_end =
                        ImFloor((i == last_spring_item_index) ? free_space : (free_space * next_weight / total_spring_weight));
                float spring_size = span_end - span_start;
                item.SpringSize   = spring_size;
                span_start        = span_end;
                current_weight    = next_weight;
            }
            else
            {
                item.SpringSize = 0.0f;
            }

            // If spring changed its size, fix positioning of following items to avoid one frame visual bugs.
            if (last_spring_size != item.SpringSize)
            {
                float difference = item.SpringSize - last_spring_size;

                ImVec2 offset = is_horizontal ? ImVec2(difference, 0.0f) : ImVec2(0.0f, difference);

                item.MeasuredBounds.Max += offset;

                for (int j = i + 1; j < layout.Items.Size; j++)
                {
                    ImGuiLayoutItem& translated_item = layout.Items[j];

                    TranslateLayoutItem(translated_item, offset);

                    translated_item.MeasuredBounds.Min += offset;
                    translated_item.MeasuredBounds.Max += offset;
                }
            }
        }
    }

    static ImVec2 BalanceLayoutItemAlignment(ImGuiLayout& layout, ImGuiLayoutItem& item)
    {
        // Fixup item alignment if necessary.
        ImVec2 position_correction = ImVec2(0.0f, 0.0f);
        if (item.CurrentAlign > 0.0f)
        {
            float item_align_offset = CalculateLayoutItemAlignmentOffset(layout, item);
            if (item.CurrentAlignOffset != item_align_offset)
            {
                float offset = item_align_offset - item.CurrentAlignOffset;

                if (layout.Type == ImGuiLayoutType_Horizontal)
                    position_correction.y = offset;
                else
                    position_correction.x = offset;

                TranslateLayoutItem(item, position_correction);

                item.CurrentAlignOffset = item_align_offset;
            }
        }

        return position_correction;
    }

    static void BalanceLayoutItemsAlignment(ImGuiLayout& layout)
    {
        for (int i = 0; i < layout.Items.Size; ++i)
        {
            ImGuiLayoutItem& item = layout.Items[i];
            BalanceLayoutItemAlignment(layout, item);
        }
    }

    static bool HasAnyNonZeroSpring(ImGuiLayout& layout)
    {
        for (int i = 0; i < layout.Items.Size; ++i)
        {
            ImGuiLayoutItem& item = layout.Items[i];
            if (item.Type != ImGuiLayoutItemType_Spring)
                continue;
            if (item.SpringWeight > 0)
                return true;
        }
        return false;
    }

    static void BalanceChildLayouts(ImGuiLayout& layout)
    {
        for (ImGuiLayout* child = layout.FirstChild; child != NULL; child = child->NextSibling)
        {
            //ImVec2 child_layout_size = child->CurrentSize;

            // Propagate layout size down to child layouts.
            //
            // TODO: Distribution assume inner layout is only
            //       element inside parent item and assigns
            //       all available space to it.
            //
            //       Investigate how to split space between
            //       adjacent layouts.
            //
            //       Investigate how to measure non-layout items
            //       to treat them as fixed size blocks.
            //
            if (child->Type == ImGuiLayoutType_Horizontal && child->Size.x <= 0.0f)
                child->CurrentSize.x = layout.CurrentSize.x;
            else if (child->Type == ImGuiLayoutType_Vertical && child->Size.y <= 0.0f)
                child->CurrentSize.y = layout.CurrentSize.y;

            BalanceChildLayouts(*child);

            //child->CurrentSize = child_layout_size;

            if (HasAnyNonZeroSpring(*child))
            {
                // Expand item measured bounds to make alignment correct.
                ImGuiLayoutItem& item = layout.Items[child->ParentItemIndex];

                if (child->Type == ImGuiLayoutType_Horizontal && child->Size.x <= 0.0f)
                    item.MeasuredBounds.Max.x =
                            ImMax(item.MeasuredBounds.Max.x, item.MeasuredBounds.Min.x + layout.CurrentSize.x);
                else if (child->Type == ImGuiLayoutType_Vertical && child->Size.y <= 0.0f)
                    item.MeasuredBounds.Max.y =
                            ImMax(item.MeasuredBounds.Max.y, item.MeasuredBounds.Min.y + layout.CurrentSize.y);
            }
        }

        BalanceLayoutSprings(layout);
        BalanceLayoutItemsAlignment(layout);
    }

    static ImGuiLayoutItem* GenerateLayoutItem(ImGuiLayout& layout, ImGuiLayoutItemType type)
    {
        ImGuiContext& g = *GImGui;
        IM_ASSERT(layout.CurrentItemIndex <= layout.Items.Size);

        if (layout.CurrentItemIndex < layout.Items.Size)
        {
            ImGuiLayoutItem& item = layout.Items[layout.CurrentItemIndex];
            if (item.Type != type)
                item = ImGuiLayoutItem(type);
        }
        else
        {
            layout.Items.push_back(ImGuiLayoutItem(type));
        }

        g.CurrentWindow->DC.CurrentLayoutItem = &layout.Items[layout.CurrentItemIndex];

        return &layout.Items[layout.CurrentItemIndex];
    }

    // Calculate how many pixels from top/left layout edge item need to be moved to match
    // layout alignment.
    static float CalculateLayoutItemAlignmentOffset(ImGuiLayout& layout, ImGuiLayoutItem& item)
    {
        if (item.CurrentAlign <= 0.0f)
            return 0.0f;

        ImVec2 item_size = item.MeasuredBounds.GetSize();

        float layout_extent = (layout.Type == ImGuiLayoutType_Horizontal) ? layout.CurrentSize.y : layout.CurrentSize.x;
        float item_extent   = (layout.Type == ImGuiLayoutType_Horizontal) ? item_size.y : item_size.x;

        if (item_extent <= 0 /* || layout_extent <= item_extent*/)
            return 0.0f;

        float align_offset = ImFloor(item.CurrentAlign * (layout_extent - item_extent));

        return align_offset;
    }

    static void TranslateLayoutItem(ImGuiLayoutItem& item, const ImVec2& offset)
    {
        if ((offset.x == 0.0f && offset.y == 0.0f) || (item.VertexIndexBegin == item.VertexIndexEnd))
            return;

        //IMGUI_DEBUG_LOG("TranslateLayoutItem by %f,%f\n", offset.x, offset.y);
        ImDrawList* draw_list = GetWindowDrawList();

        ImDrawVert* begin = draw_list->VtxBuffer.Data + item.VertexIndexBegin;
        ImDrawVert* end   = draw_list->VtxBuffer.Data + item.VertexIndexEnd;

        for (ImDrawVert* vtx = begin; vtx < end; ++vtx)
        {
            vtx->pos.x += offset.x;
            vtx->pos.y += offset.y;
        }
    }

    static void SignedIndent(float indent)
    {
        if (indent > 0.0f)
            Indent(indent);
        else if (indent < 0.0f)
            Unindent(-indent);
    }

    static void BeginLayoutItem(ImGuiLayout& layout)
    {
        ImGuiContext& g       = *GImGui;
        ImGuiWindow* window   = g.CurrentWindow;
        ImGuiLayoutItem& item = *GenerateLayoutItem(layout, ImGuiLayoutItemType_Item);

        item.CurrentAlign = layout.Align;
        if (item.CurrentAlign < 0.0f)
            item.CurrentAlign = ImClamp(g.Style.LayoutAlign, 0.0f, 1.0f);

        // Align item according to data from previous frame.
        // If layout changes in current frame alignment will
        // be corrected in EndLayout() to it visualy coherent.
        item.CurrentAlignOffset = CalculateLayoutItemAlignmentOffset(layout, item);
        if (item.CurrentAlign > 0.0f)
        {
            if (layout.Type == ImGuiLayoutType_Horizontal)
            {
                window->DC.CursorPos.y += item.CurrentAlignOffset;
            }
            else
            {
                float new_position = window->DC.CursorPos.x + item.CurrentAlignOffset;

                // Make placement behave like in horizontal case when next
                // widget is placed at very same Y position. This indent
                // make sure for vertical layout placed widgets has same X position.
                SignedIndent(item.CurrentAlignOffset);

                window->DC.CursorPos.x = new_position;
            }
        }

        item.MeasuredBounds.Min = item.MeasuredBounds.Max = window->DC.CursorPos;
        item.VertexIndexBegin = item.VertexIndexEnd = window->DrawList->_VtxCurrentIdx;
    }

    static void EndLayoutItem(ImGuiLayout& layout)
    {
        ImGuiContext& g     = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        IM_ASSERT(layout.CurrentItemIndex < layout.Items.Size);

        ImGuiLayoutItem& item = layout.Items[layout.CurrentItemIndex];

        ImDrawList* draw_list = window->DrawList;
        item.VertexIndexEnd   = draw_list->_VtxCurrentIdx;

        if (item.CurrentAlign > 0.0f && layout.Type == ImGuiLayoutType_Vertical)
            SignedIndent(-item.CurrentAlignOffset);

        // Fixup item alignment in case item size changed in current frame.
        ImVec2 position_correction = BalanceLayoutItemAlignment(layout, item);

        item.MeasuredBounds.Min += position_correction;
        item.MeasuredBounds.Max += position_correction;

        if (layout.Type == ImGuiLayoutType_Horizontal)
            window->DC.CursorPos.y = layout.StartPos.y;
        else
            window->DC.CursorPos.x = layout.StartPos.x;

        layout.CurrentItemIndex++;
    }

    static void AddLayoutSpring(ImGuiLayout& layout, float weight, float spacing)
    {
        ImGuiContext& g                = *GImGui;
        ImGuiWindow* window            = g.CurrentWindow;
        ImGuiLayoutItem* previous_item = &layout.Items[layout.CurrentItemIndex];

        // Undo item padding, spring should consume all space between items.
        if (layout.Type == ImGuiLayoutType_Horizontal)
            window->DC.CursorPos.x = previous_item->MeasuredBounds.Max.x;
        else
            window->DC.CursorPos.y = previous_item->MeasuredBounds.Max.y;

        previous_item = NULL;// may be invalid after call to GenerateLayoutItem()

        EndLayoutItem(layout);

        ImGuiLayoutItem* spring_item = GenerateLayoutItem(layout, ImGuiLayoutItemType_Spring);

        spring_item->MeasuredBounds.Min = spring_item->MeasuredBounds.Max = window->DC.CursorPos;

        if (weight < 0.0f)
            weight = 0.0f;

        if (spring_item->SpringWeight != weight)
            spring_item->SpringWeight = weight;

        if (spacing < 0.0f)
        {
            ImVec2 style_spacing = g.Style.ItemSpacing;
            if (layout.Type == ImGuiLayoutType_Horizontal)
                spacing = style_spacing.x;
            else
                spacing = style_spacing.y;
        }

        if (spring_item->SpringSpacing != spacing)
            spring_item->SpringSpacing = spacing;

        if (spring_item->SpringSize > 0.0f || spacing > 0.0f)
        {
            ImVec2 spring_size, spring_spacing;
            if (layout.Type == ImGuiLayoutType_Horizontal)
            {
                spring_spacing = ImVec2(0.0f, g.Style.ItemSpacing.y);
                spring_size    = ImVec2(spacing + spring_item->SpringSize, layout.CurrentSize.y);
            }
            else
            {
                spring_spacing = ImVec2(g.Style.ItemSpacing.x, 0.0f);
                spring_size    = ImVec2(layout.CurrentSize.x, spacing + spring_item->SpringSize);
            }

            PushStyleVar(ImGuiStyleVar_ItemSpacing, ImFloor(spring_spacing));
            Dummy(ImFloor(spring_size));
            PopStyleVar();
        }

        layout.CurrentItemIndex++;

        BeginLayoutItem(layout);
    }

    void BeginHorizontal(const char* str_id, const ImVec2& size /* = ImVec2(0, 0)*/, float align /* = -1*/)
    {
        ImGuiWindow* window = GetCurrentWindow();
        BeginLayout(window->GetID(str_id), ImGuiLayoutType_Horizontal, size, align);
    }

    void BeginHorizontal(const void* ptr_id, const ImVec2& size /* = ImVec2(0, 0)*/, float align /* = -1*/)
    {
        ImGuiWindow* window = GetCurrentWindow();
        BeginLayout(window->GetID(ptr_id), ImGuiLayoutType_Horizontal, size, align);
    }

    void BeginHorizontal(int id, const ImVec2& size /* = ImVec2(0, 0)*/, float align /* = -1*/)
    {
        ImGuiWindow* window = GetCurrentWindow();
        BeginLayout(window->GetID((void*) (intptr_t) id), ImGuiLayoutType_Horizontal, size, align);
    }

    void EndHorizontal()
    {
        EndLayout(ImGuiLayoutType_Horizontal);
    }

    void BeginVertical(const char* str_id, const ImVec2& size /* = ImVec2(0, 0)*/, float align /* = -1*/)
    {
        ImGuiWindow* window = GetCurrentWindow();
        BeginLayout(window->GetID(str_id), ImGuiLayoutType_Vertical, size, align);
    }

    void BeginVertical(const void* ptr_id, const ImVec2& size /* = ImVec2(0, 0)*/, float align /* = -1*/)
    {
        ImGuiWindow* window = GetCurrentWindow();
        BeginLayout(window->GetID(ptr_id), ImGuiLayoutType_Vertical, size, align);
    }

    void BeginVertical(int id, const ImVec2& size /* = ImVec2(0, 0)*/, float align /* = -1*/)
    {
        ImGuiWindow* window = GetCurrentWindow();
        BeginLayout(window->GetID((void*) (intptr_t) id), ImGuiLayoutType_Vertical, size, align);
    }

    void EndVertical()
    {
        EndLayout(ImGuiLayoutType_Vertical);
    }

    // Inserts spring separator in layout
    //      weight <= 0     : spring will always have zero size
    //      weight > 0      : power of current spring
    //      spacing < 0     : use default spacing if pos_x == 0, no spacing if pos_x != 0
    //      spacing >= 0    : enforce spacing amount
    void Spring(float weight /* = 1.0f*/, float spacing /* = -1.0f*/)
    {
        ImGuiWindow* window = GetCurrentWindow();
        IM_ASSERT(window->DC.CurrentLayout);

        AddLayoutSpring(*window->DC.CurrentLayout, weight, spacing);
    }

    void SuspendLayout()
    {
        PushLayout(NULL);
    }

    void ResumeLayout()
    {
        ImGuiWindow* window = GetCurrentWindow();
        IM_ASSERT(!window->DC.CurrentLayout);
        IM_ASSERT(!window->DC.LayoutStack.empty());
        IM_UNUSED(window);
        PopLayout(NULL);
    }

}// namespace ImGui
