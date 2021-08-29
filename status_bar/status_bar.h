#pragma once

#include "wnd.h"
#include <string>
#include <vector>

namespace CWindows{

    class StatusBar: public Wnd{
        std::vector<std::wstring> text;
        void SetPart();
        LRESULT OnSize(WPARAM wparam, LPARAM lparam);
        LRESULT OnSetText(WPARAM wparam, LPARAM lparam);
    public:
        StatusBar();
        bool Create(HWND parent, std::wstring caption, int top, int left, int width, int height, UINT id) override;
        
    };

}