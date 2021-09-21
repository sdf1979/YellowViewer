#pragma once

#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include "wnd.h"
#include "text_data.h"
#include "directory_watcher.h"

namespace CWindows{

    enum class Token{
        String,
        Tab,
        NewLine
    };

    using data_out = std::pair<Token, std::wstring_view>;

    class Edit: public Wnd{
        TechLog1C::DirectoryWatcher* dw_;
        int width_;
        int height_;
        int bottom_margin_;
        int top_margin_;
        int hpos_;
        int vpos_;
        int max_hpos_;
        int max_vpos_;

        int height_string_;
        unsigned int num_line_;
        int offset_x_;
        int caret_column_;
        int caret_line_;
        bool caret_visible_;
        std::unique_ptr<TechLog1C::EventOut> current_event_;
        std::unique_ptr<TechLog1C::EventOut> current_event_on_rbutton_click_;

        HFONT font_;
        HBRUSH hbrush_grey_;
        size_t start_index_event_;
        std::vector<std::pair<uint64_t, std::wstring>> display_lines_;
                
        void DisplayCaret(HDC hdc);
        void MarkCurrentEvent(int x, int y);
        std::unique_ptr<TechLog1C::EventOut> GetCurrentEvent(int x, int y);
        std::unordered_map<UINT, std::wstring> selection_condition_;
        std::unordered_map<std::uint32_t, COLORREF> line_color_;
        
        LRESULT OnSize(WPARAM wparam, LPARAM lparam);
        LRESULT OnEraseBkgnd(WPARAM wparam, LPARAM lparam);
        LRESULT OnPaint(WPARAM wparam, LPARAM lparam);
        LRESULT OnHScroll(WPARAM wparam, LPARAM lparam);
        LRESULT OnVScroll(WPARAM wparam, LPARAM lparam);
        LRESULT OnSetFocus(WPARAM wparam, LPARAM lparam);
        LRESULT OnKillFocus(WPARAM wparam, LPARAM lparam);
        LRESULT OnMouseMove(WPARAM wparam, LPARAM lparam);
        LRESULT OnLButtonDown(WPARAM wparam, LPARAM lparam);
        LRESULT OnRButtonDown(WPARAM wparam, LPARAM lparam);
        LRESULT OnFind(WPARAM wparam, LPARAM lparam);
        LRESULT OnDestroy(WPARAM wparam, LPARAM lparam);
        LRESULT OnMouseWheel(WPARAM wparam, LPARAM lparam);
        LRESULT OnContextMenu(WPARAM wparam, LPARAM lparam);
        LRESULT OnCommand(WPARAM wparam, LPARAM lparam);

        void AppendMenuExt(HMENU h_pop_menu_, UINT id, std::wstring menu_str, std::wstring filter);
    public:
        Edit(TechLog1C::DirectoryWatcher* dw);
        ~Edit();
        bool Create(HWND parent, std::wstring caption, int top, int left, int width, int height, UINT id) override;
        void SetBottomMargin(int bottom_margin);
        void UpdateDisplay();
        void SetMaxVPos(int max_vpos);        
    };
}