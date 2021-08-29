#pragma once

#include <windows.h>
#include <commctrl.h>
#include <unordered_map>
#include <string>
#include <fcntl.h>
#include <io.h>
#include <iostream>

namespace CWindows{

    static const UINT ID_STATUS_BAR = 1;
    static const UINT ID_EDIT = 2;
    static const UINT ID_PATH = 3;
    static const UINT ID_FIND = 4;
    static const UINT ID_EDIT_COPY = 5;
    static const UINT ID_MENU_ITEM_FIND = 6;
    static const UINT ID_EXCP = 7;
    static const UINT ID_QERR = 8;
    static const UINT ID_TTIMEOUT = 9;
    static const UINT ID_TDEADLOCK = 10;
    static const UINT ID_TLOCK = 11;
    static const UINT ID_CUSTOM_EVENT = 12;
    static const UINT ID_USR = 13;
    static const UINT ID_SESSION_ID = 14;
    static const UINT ID_CONNECT_ID = 15;
    static const UINT ID_P_PROCESS_NAME = 16;
    static const UINT ID_ANALIZE_TDEADLOCK = 17;
    static const UINT ID_ANALIZE_TTIMEOUT = 18;
    static const UINT ID_ANALIZE_TLOCK = 19;

    static const UINT WM_END_INPUT = WM_USER + 1;
    static const UINT WM_FIND = WM_USER + 2;
    static const UINT WM_MENU_SELECTION = WM_USER + 2;

    struct InputKeyData
    {
        unsigned repeatCount : 16;
        unsigned scanCode : 8;
        unsigned isExtended : 1;
        unsigned reserved : 4;
        unsigned contextCode : 1;
        unsigned previousState : 1;
        unsigned transitionState : 1;
    };

    class Wnd{
        typedef LRESULT (Wnd::*FuncPointer)(WPARAM, LPARAM);
        struct POINTER
        {
            Wnd* wnd;
            FuncPointer func;
        };
        std::wstring class_name_;
    protected:
        HWND hwnd_ = nullptr;
        WNDPROC wndproc_default_ = nullptr;
        int top_;
        int left_;
        int width_;
        int height_;
        std::unordered_map<UINT, POINTER> msgmap_;
        static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
        bool Create(std::wstring class_name, HWND parent, std::wstring caption, DWORD exstyle, DWORD style, int top, int left, int width, int height, UINT id);
        void OutConsole(std::wstring msg);
    public:
        static void CreateConsole();
        virtual bool Create(HWND parent, std::wstring caption, int top, int left, int width, int height, UINT id) = 0;
        RECT Rect();
        HWND GetHWND() { return hwnd_; }
        int Height(){ return height_; }
        
        template<typename T>
        bool AddMessage(UINT msg, Wnd* wnd, LRESULT (T::*funcpointer)(WPARAM, LPARAM))
        {
            if(!wnd || !funcpointer) return false;
            msgmap_.insert({msg, {wnd, reinterpret_cast<FuncPointer>(funcpointer)}});

            return true;
        }
    };

}