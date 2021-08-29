#include "wnd.h"

using namespace std;

namespace CWindows{

    void Wnd::CreateConsole(){
        if (AllocConsole())
        {
            int hCrt = _open_osfhandle((intptr_t)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
            *stdout = *(::fdopen(hCrt, "w"));
            ::setvbuf(stdout, NULL, _IONBF, 0);
            *stderr = *(::fdopen(hCrt, "w"));
            ::setvbuf(stderr, NULL, _IONBF, 0);
            _setmode( _fileno( stdout ), _O_WTEXT );
            std::ios::sync_with_stdio();
        }
    }

    bool Wnd::Create(wstring class_name, HWND parent, wstring caption, DWORD exstyle, DWORD style, int top, int left, int width, int height, UINT id){
        class_name_ = class_name;
        WNDCLASSW wc = {0};
        wc.hInstance = GetModuleHandleW(0);
        wc.lpszClassName = class_name.c_str();
        wc.lpfnWndProc = WNDPROC(&WndProc);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.lpszMenuName = NULL;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hbrBackground = HBRUSH(COLOR_WINDOW);
        
        if(!GetClassInfoW(GetModuleHandleW(0), class_name.c_str(), &wc)){
            if(!RegisterClassW(&wc)) return false;
        }
        top_ = top; left_ = left; width_ = width; height_ = height;
        hwnd_ = CreateWindowExW(exstyle, class_name.c_str(), caption.c_str(), style, left, top, width, height, parent, NULL, GetModuleHandleW(0), this);
        SetWindowLongPtr(hwnd_, GWLP_ID, id);
                
        return hwnd_;
    }

    RECT Wnd::Rect(){
        return {left_, top_, left_ + width_, top_ + height_};
    }

    void Wnd::OutConsole(std::wstring msg){
        wcout << msg << endl;
    }

    LRESULT CALLBACK Wnd::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
        Wnd* wnd = nullptr;
        if(msg == WM_NCCREATE){
            wnd = (Wnd*)LPCREATESTRUCTW(lparam)->lpCreateParams;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, LONG_PTR(LPCREATESTRUCT(lparam)->lpCreateParams));
            wnd->hwnd_ = hwnd;
        }        

        wnd = (Wnd*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);
        
        if(wnd){
            auto it = wnd->msgmap_.find(msg);
            if(it == wnd->msgmap_.end()){
                if(wnd->wndproc_default_){
                    return CallWindowProc(wnd->wndproc_default_, hwnd, msg, wparam, lparam);
                }
                else{
                    return DefWindowProcW(hwnd, msg, wparam, lparam);
                }
            }
            else{
                auto pointer = it->second;
                return (pointer.wnd->*pointer.func)(wparam, lparam);
            }
        }

        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}