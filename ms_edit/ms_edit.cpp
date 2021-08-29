#include "ms_edit.h"

using namespace std;

namespace CWindows{
    
    MS_Edit::MS_Edit(){
        font_ = CreateFontW(15, 0, 0, 0, 0, 0, 0 ,0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH|FF_DONTCARE, L"Courier New");

        NONCLIENTMETRICS metrics = {};
        metrics.cbSize = sizeof(metrics);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);
        font_caption_ = CreateFontIndirect(&metrics.lfMessageFont);

        AddMessage(WM_CHAR, this, &MS_Edit::OnChar);
    }

    MS_Edit::~MS_Edit(){
        DeleteObject(font_);
        DeleteObject(font_caption_);
    }

    bool MS_Edit::Create(HWND parent, wstring caption, int top, int left, int width, int height, UINT id){
        caption_ = caption;
        hwnd_caption_ = CreateWindowW(L"STATIC", &caption_[0], WS_CHILD|WS_VISIBLE|SS_LEFTNOWORDWRAP, 0, 0, 0, 0, parent, 0, GetModuleHandleW(0), NULL);
        HDC hdc_caption = GetDC(hwnd_caption_);
        SelectObject(hdc_caption, font_caption_);
        SIZE size_caption;
        GetTextExtentPointW(hdc_caption, &caption_[0], caption_.size(), &size_caption);
        ReleaseDC (hwnd_caption_, hdc_caption);
        MoveWindow(hwnd_caption_, left, top + (height - size_caption.cy)/2, size_caption.cx, size_caption.cy, TRUE);
        SendMessage(hwnd_caption_, WM_SETFONT, (WPARAM)font_caption_, TRUE);

        Wnd::Create(L"EDIT", parent, L"", WS_EX_CLIENTEDGE, WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, top, left + size_caption.cx, width, height, id);
        left_ -= size_caption.cx;
        width_ += size_caption.cx;
        wndproc_default_ = (WNDPROC)GetWindowLongPtr(hwnd_, GWLP_WNDPROC);
        Wnd* wnd = this;
        SetWindowLongPtrW(hwnd_, GWLP_USERDATA, LONG_PTR(wnd));
        SetWindowLongPtr(hwnd_, GWLP_WNDPROC, (LONG_PTR)WndProc);
        SendMessage(hwnd_, WM_SETFONT, (WPARAM)font_, TRUE);
        return 0;
    }

    void MS_Edit::Move(int top, int left, int width, int height){
        HDC hdc_caption = GetDC(hwnd_caption_);
        SelectObject(hdc_caption, font_caption_);
        SIZE size_caption;
        GetTextExtentPointW(hdc_caption, &caption_[0], caption_.size(), &size_caption);
        ReleaseDC (hwnd_caption_, hdc_caption);
        MoveWindow(hwnd_caption_, left, top + (height - size_caption.cy)/2, size_caption.cx, size_caption.cy, TRUE);

        MoveWindow(hwnd_, left + size_caption.cx, top, width - size_caption.cx, height, TRUE);
    }    

    LRESULT MS_Edit::OnChar(WPARAM wparam, LPARAM lparam){
        InputKeyData* kd = (InputKeyData*)&lparam;
        if(kd->scanCode == 28){
            LONG_PTR id = GetWindowLongPtr(hwnd_, GWLP_ID);
            SendMessageW(GetParent(hwnd_), WM_END_INPUT, (WPARAM)hwnd_, id);
        }
        return CallWindowProc(wndproc_default_, hwnd_, WM_CHAR, wparam, lparam);
    }
}