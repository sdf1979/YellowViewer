#pragma once

#include "wnd.h"
#include <string.h>

namespace CWindows{

    class MS_Edit: public Wnd{
        HFONT font_;
        HFONT font_caption_;
        LRESULT OnChar(WPARAM wparam, LPARAM lparam);
        std::wstring caption_;
        HWND hwnd_caption_;
    public:
        MS_Edit();
        ~MS_Edit();
        bool Create(HWND parent, std::wstring caption, int top, int left, int width, int height, UINT id) override;
        void Move(int top, int left, int width, int height);
        //typedef LRESULT (MS_Edit::*FuncPointer)(HWND, UINT, WPARAM, LPARAM);
        //LRESULT CALLBACK EditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    }; 
}