#pragma once

#include <memory>
#include <string.h>
#include "wnd.h"
#include "edit.h"
#include "ms_edit.h"
#include "status_bar.h"
#include "directory_watcher.h"
#include <thread>
#include "timer_perf.h"

namespace CWindows{

    class MainWindow: public Wnd
    {
        std::unique_ptr<TechLog1C::DirectoryWatcher> directory_watcher_;
        //std::unique_ptr<TechLog1C::Events> events;

        std::unique_ptr<Edit> edit_;
        std::unique_ptr<MS_Edit> path_;
        std::unique_ptr<MS_Edit> find_;

        std::unique_ptr<StatusBar> status_bar_;
        HWND status_bar2;
        
        LRESULT OnCreate(WPARAM wparam, LPARAM lparam);
        LRESULT OnSize(WPARAM wparam, LPARAM lparam);
        LRESULT OnSetFocus(WPARAM wparam, LPARAM lparam);
        LRESULT OnKillFocus(WPARAM wparam, LPARAM lparam);
        LRESULT OnChar(WPARAM wparam, LPARAM lparam);
        LRESULT OnStatusBarSetText(WPARAM wparam, LPARAM lparam);
        LRESULT OnEndInput(WPARAM wparam, LPARAM lparam);
        LRESULT OnDestroy(WPARAM wparam, LPARAM lparam);
        LRESULT OnMouseWheel(WPARAM wparam, LPARAM lparam);
        LRESULT OnMenuSelection(WPARAM wparam, LPARAM lparam);
    public:
        MainWindow();
        bool Create(HWND parent, std::wstring caption, int top, int left, int width, int height, UINT id) override;
        void LoadFiles(std::wstring path);
    };

    class App{
        std::unique_ptr<MainWindow> main_window;
    public:
        App();
        void Run();
    };

}