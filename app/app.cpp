#include "app.h"

namespace CWindows{

    static void c_callback_message(void* obj, int part, std::string msg){
        MainWindow* mw = reinterpret_cast<MainWindow*>(obj);
        SendMessageW(mw->GetHWND(), SB_SETTEXTW, (WPARAM)part, (LPARAM)&Utf8ToWideChar(msg)[0]);
    }

    MainWindow::MainWindow():
        edit_(nullptr),
        path_(nullptr),
        status_bar_(nullptr){
        AddMessage(WM_CREATE, this, &MainWindow::OnCreate);
        AddMessage(WM_SIZE, this, &MainWindow::OnSize);
        AddMessage(WM_SETFOCUS, this, &MainWindow::OnSetFocus);
        AddMessage(WM_KILLFOCUS, this, &MainWindow::OnKillFocus);
        AddMessage(WM_CHAR, this, &MainWindow::OnChar);
        AddMessage(SB_SETTEXTW, this, &MainWindow::OnStatusBarSetText);
        AddMessage(WM_END_INPUT, this, &MainWindow::OnEndInput);
        AddMessage(WM_DESTROY, this, &MainWindow::OnDestroy);
        AddMessage(WM_MOUSEWHEEL, this, &MainWindow::OnMouseWheel);
        AddMessage(WM_MENU_SELECTION, this, &MainWindow::OnMenuSelection);
    }

    bool MainWindow::Create(HWND parent, std::wstring caption, int top, int left, int width, int height, UINT id){
        bool result =  Wnd::Create(L"_MainWindow", parent, caption, 0, WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN, top, left, width, height, id);
        // HDC hdc = GetWindowDC(hwnd_);
        // SetTextAlign(hdc, TA_LEFT);
        // ReleaseDC(hwnd_, hdc);
        // auto style = GetWindowLongPtr(hwnd_ ,GWL_STYLE);
        // style = style & ~ (SS_CENTER);
        // style = style | (SS_LEFT);
        // SetWindowLongPtr(hwnd_, GWL_STYLE, style);
        // SetWindowPos (hwnd_, NULL, 0, 0, 0, 0, TRUE);
        return result;
    }

    void MainWindow::LoadFiles(std::wstring path){
        {
            MEASUREMENT;
            directory_watcher_->ReadDirectory(path);
            directory_watcher_->ReadFiles();
            edit_->UpdateDisplay();
        }

        TechLog1C::TimerPerf* timer_perf = TechLog1C::TimerPerf::GetInstance();
        timer_perf->Print();
    }

    LRESULT MainWindow::OnCreate(WPARAM wparam, LPARAM lparam){
        directory_watcher_ = std::make_unique<TechLog1C::DirectoryWatcher>();
        directory_watcher_->AddCallbackMessage(this, c_callback_message);

        status_bar_ = std::make_unique<StatusBar>();
        status_bar_->Create(hwnd_, L"", 0, 0, 0, 0, ID_STATUS_BAR);
        
        edit_ = std::make_unique<Edit>(directory_watcher_.get());
        edit_->SetBottomMargin(status_bar_->Height());
        edit_->Create(hwnd_, L"", 0, 0, 0, 0, ID_EDIT);

        int height_element = 24;
        int indent = 30;

        find_ = std::make_unique<MS_Edit>();
        find_->Create(hwnd_, L"Найти: ", 4, 4, 400, height_element, ID_FIND);

        RECT rc_find = find_->Rect();
        path_ = std::make_unique<MS_Edit>();
        path_->Create(hwnd_, L"Каталог тех. журналов: ", rc_find.top, rc_find.right + indent, 400, height_element, ID_PATH);        

        return 0;
    }

    LRESULT MainWindow::OnSize(WPARAM wparam, LPARAM lparam){
        tagRECT r;
        GetClientRect(hwnd_, &r);

        SendMessageW(edit_->GetHWND(), WM_SIZE, wparam, lparam);
        SendMessageW(edit_->GetHWND(), WM_SETFOCUS, wparam, lparam);
        SendMessageW(status_bar_->GetHWND(), WM_SIZE, wparam, lparam);

        int height_element = 24;
        int indent = 30;
        find_->Move(4, 4, r.right - r.left - indent - 400, height_element);
        path_->Move(4, r.right - 400, 399, 24);
        
        return 0;
    }

    LRESULT MainWindow::OnSetFocus(WPARAM wparam, LPARAM lparam){
        //SendMessageW(edit->GetHWND(), WM_SETFOCUS, wparam, lparam);
        return 0;
    }

    LRESULT MainWindow::OnKillFocus(WPARAM wparam, LPARAM lparam){
        //SendMessageW(edit->GetHWND(), WM_KILLFOCUS, wparam, lparam);
        return 0;
    }

    LRESULT MainWindow::OnChar(WPARAM wparam, LPARAM lparam){
        InputKeyData* kd = (InputKeyData*)&lparam;
        bool vk_control = GetAsyncKeyState(VK_CONTROL);
        if(vk_control && kd->scanCode == 38){
            // directory_watcher_->ReadDirectory();
            // directory_watcher_->ReadFiles();
            // directory_watcher_->Sort();
            // edit->UpdateDisplay();
        }
        else if(vk_control && kd->scanCode == 33){
            //SendMessageW(edit->GetHWND(), WM_CHAR, wparam, lparam);
        }
        return 0;
    }

    LRESULT MainWindow::OnStatusBarSetText(WPARAM wparam, LPARAM lparam){
        SendMessageW(status_bar_->GetHWND(), SB_SETTEXTW, wparam, lparam);
        return 0;
    }

    static void LoadFileInThread(MainWindow* mw, std::wstring path){
        mw->LoadFiles(path);
    }

    LRESULT MainWindow::OnEndInput(WPARAM wparam, LPARAM lparam){
        HWND hwnd = (HWND)wparam;
        if(lparam == ID_PATH){
            int length = SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
            std::wstring str(length + 1, L'\0');
            GetWindowTextW(hwnd, &str[0], str.size());
            str.resize(length);
            std::thread thread(LoadFileInThread, this, str);
            thread.detach();
        }
        else if(lparam == ID_FIND){
            int length = SendMessageW(hwnd, WM_GETTEXTLENGTH, 0, 0);
            std::wstring str(length + 1, L'\0');
            GetWindowTextW(hwnd, &str[0], str.size());
            str.resize(length);
            SendMessageW(edit_->GetHWND(), WM_FIND, (WPARAM)&str[0], str.size());
        }
        return 0;
    }    

    LRESULT MainWindow::OnDestroy(WPARAM wparam, LPARAM lparam){
        PostQuitMessage(0);
        return 0;
    }

    LRESULT MainWindow::OnMouseWheel(WPARAM wparam, LPARAM lparam){
        SendMessageW(edit_->GetHWND(), WM_MOUSEWHEEL, wparam, lparam);
        return 0;
    }

    LRESULT MainWindow::OnMenuSelection(WPARAM wparam, LPARAM lparam){
        if(wparam = ID_MENU_ITEM_FIND){
            SendMessageW(find_->GetHWND(), WM_SETTEXT, 0, lparam);
            SendMessageW(find_->GetHWND(), WM_PAINT, 0, 0);            
            int length = SendMessageW(find_->GetHWND(), WM_GETTEXTLENGTH, 0, 0);
            std::wstring str(length + 1, L'\0');
            GetWindowTextW(find_->GetHWND(), &str[0], str.size());
            str.resize(length);

            SendMessageW(edit_->GetHWND(), WM_FIND, (WPARAM)&str[0], str.size());
        }
        return 0;
    }

    App::App():
        main_window(nullptr){}

    void App::Run(){
        if(!main_window){
            main_window = std::make_unique<MainWindow>();
            main_window->Create(0, L"Yellow viewer", 300, 300, 800, 600, 0);
            //TODO DEBUG
            //main_window->CreateConsole();
        }

        MSG msg;
        //HACCEL hAccel = LoadAcceleratorsW(GetModuleHandleW(0), MAKEINTRESOURCEW(IDR_ACCELERATOR));
        while(GetMessageW(&msg, NULL, 0, 0)){
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
}