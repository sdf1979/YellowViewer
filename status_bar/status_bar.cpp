#include "status_bar.h"

using namespace std;

namespace CWindows{

    StatusBar::StatusBar(){
        AddMessage(WM_SIZE, this, &StatusBar::OnSize);
        AddMessage(SB_SETTEXTW, this, &StatusBar::OnSetText);
    }

    bool StatusBar::Create(HWND parent, wstring caption, int top, int left, int width, int height, UINT id){
        bool result = Wnd::Create(STATUSCLASSNAMEW, parent, caption, 0, WS_CHILD|WS_VISIBLE, top, left, width, height, id);
        RECT rc;
        GetWindowRect(hwnd_, &rc);
        height_ = rc.bottom - rc.top;

        SetPart();
        
        wndproc_default_ = (WNDPROC)GetWindowLongPtr(hwnd_, GWLP_WNDPROC);
        Wnd* wnd = this;
        SetWindowLongPtrW(hwnd_, GWLP_USERDATA, LONG_PTR(wnd));
        SetWindowLongPtr(hwnd_, GWLP_WNDPROC, (LONG_PTR)WndProc);

        return result;
    }

    LRESULT StatusBar::OnSize(WPARAM wparam, LPARAM lparam){
        SetPart();
        return CallWindowProc(wndproc_default_, hwnd_, WM_SIZE, wparam, lparam);
    }

    LRESULT StatusBar::OnSetText(WPARAM wparam, LPARAM lparam){
        return CallWindowProc(wndproc_default_, hwnd_, SB_SETTEXTW, wparam, lparam);
    }

    void StatusBar::SetPart(){
        HLOCAL hloc = LocalAlloc(LHND, sizeof(int) * 2);
        PINT pa_parts = (PINT)LocalLock(hloc);
        RECT rc;
        GetClientRect(GetParent(hwnd_), &rc);
        int width = (rc.right - rc.left)/2;
        pa_parts[0] = width;
        pa_parts[1] = rc.right - rc.left;
        SendMessageW(hwnd_, SB_SETPARTS, (WPARAM)2, (LPARAM)pa_parts);
        LocalUnlock(hloc);
        LocalFree(hloc); 
    }
}