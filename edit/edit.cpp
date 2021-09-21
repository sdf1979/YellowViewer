#include "edit.h"

using namespace std;

namespace CWindows{

    static std::wstring tab_ = L"    ";
    static std::wstring new_line_ = L"\n";
    static const int MAX_SIZE_EVAL_TEXT_PROPERTY = 4000;

    void TextProperty(HDC hdc, wstring_view wstr, int* dx, SIZE* size){
        if(!wstr.size()) return;

        SIZE size_cur;
        int fit_cur;
        wstring_view wstr_cur;
        while(wstr.size() > 0){
            if(wstr.size() > MAX_SIZE_EVAL_TEXT_PROPERTY){
                wstr_cur = wstr.substr(0, MAX_SIZE_EVAL_TEXT_PROPERTY);
                wstr.remove_prefix(MAX_SIZE_EVAL_TEXT_PROPERTY);
            }
            else{
                wstr_cur = wstr;
                wstr = L"";
            }
            GetTextExtentExPointW(hdc, &wstr_cur[0], wstr_cur.size(), wstr_cur.size() * 10, &fit_cur, dx, &size_cur);
            size->cx = size_cur.cx;
            size->cy += size_cur.cy;

            dx += wstr_cur.size();
        }
    }

    void TextProperty(HDC hdc, wstring_view wstr, SIZE* size){
        vector<int> dx(wstr.size());
        TextProperty(hdc, wstr, &dx[0], size);
    }

    size_t MaxSizeString(HDC hdc, const vector<pair<uint64_t, wstring>>& wstrings){
        size_t max_size = 0;
        SIZE size = {0, 0};
        for(auto it = wstrings.begin(); it != wstrings.end(); ++it){
            GetTextExtentPointW(hdc, &(it->second)[0], it->second.size(), &size);
            TextProperty(hdc, it->second, &size);
            if(max_size < size.cx) max_size = size.cx;
        }        
        return max_size;
    }

    void SetVScrollInfo(HWND hwnd, int vpos, int max_vpos){
        SCROLLINFO scrInfo;
        scrInfo.cbSize = sizeof(SCROLLINFO);
        scrInfo.nPage = 10;
        scrInfo.nMin = 0;
        scrInfo.nMax = max_vpos;
        scrInfo.nPos = vpos;
        scrInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        SetScrollInfo(hwnd, SB_VERT, &scrInfo, TRUE);        
    }

    void SetHScrollInfo(HWND hwnd, int max_hpos){
        SCROLLINFO scrInfoH;
        scrInfoH.cbSize = sizeof(SCROLLINFO);
        scrInfoH.nPage = 10;
        scrInfoH.nMin = 0;
        scrInfoH.nMax = max_hpos;
        scrInfoH.nPos = 0;
        scrInfoH.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        SetScrollInfo(hwnd, SB_HORZ, &scrInfoH, TRUE);        
    }

    void Msg(HWND hwnd, wstring msg){
        SendMessageW(hwnd, SB_SETTEXTW, (WPARAM)0, (LPARAM)&msg[0]);
    }

    vector<int> TextPoint(HDC hdc, wstring_view wstr){
        vector<int> dx(wstr.size());
        SIZE size = {0, 0};
        
        TextProperty(hdc, wstr, &dx[0], &size);

        //Вычисляем новые границы символов с учетом tab
        int addition = 0;
        int width_tab = 32;
        for(int i = 0; i < wstr.size(); ++i){
            dx[i] += addition;
            if(wstr[i] == L'\t'){
                int delta_add = (dx[i]/width_tab + 1) * width_tab - dx[i];
                dx[i] += delta_add;
                addition += delta_add;                    
            }                
        }
        return dx;
    }

    void ExtStringOut(HDC hdc, const wstring& wstr, int x, int y){
        if(!wstr.size()) return;

        HBRUSH hbrush_red = CreateSolidBrush(RGB(200, 0, 0));

        wstring_view wstr_view(wstr);
        wstring_view wstr_cur;
        while(wstr_view.size() > 0){
            if(wstr_view.size() > MAX_SIZE_EVAL_TEXT_PROPERTY){
                wstr_cur = wstr_view.substr(0, MAX_SIZE_EVAL_TEXT_PROPERTY);
                wstr_view.remove_prefix(MAX_SIZE_EVAL_TEXT_PROPERTY);
            }
            else{
                wstr_cur = wstr_view;
                wstr_view = L"";
            }
            vector<int> dx = TextPoint(hdc, wstr_cur);
            //Подготавливаем массив расстояний между символами
            vector<int> dx_tab(wstr_cur.size());
            dx_tab[0] = dx[0];
            for(int i = 1; i < dx.size(); ++i){
                dx_tab[i] = dx[i] - dx[i-1];
            }
            ExtTextOutW(hdc, x, y, 0, nullptr, &wstr_cur[0], wstr_cur.size(), &dx_tab[0]);
            x += dx.back();
        }
    }

    void ExtStringOut(HDC hdc, const wstring& wstr, int x, int y, COLORREF font_color){
        COLORREF cur_color = SetTextColor(hdc, font_color);
        ExtStringOut(hdc, wstr, x, y);
        SetTextColor(hdc, cur_color);
    }

    void CopyClipboard(const wstring& str){
        HGLOBAL hMem;
        hMem =  GlobalAlloc(GMEM_MOVEABLE, str.length() * sizeof(str[0]));
        memcpy(GlobalLock(hMem), str.c_str(), str.length() * sizeof(str[0]));
        GlobalUnlock(hMem);
        OpenClipboard(0);
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, hMem);
        CloseClipboard();
    }

    Edit::Edit(TechLog1C::DirectoryWatcher* dw):
        dw_(dw),
        width_(0),
        height_(0),
        bottom_margin_ (0),
        top_margin_(35),
        hpos_(0),
        vpos_(0),
        max_hpos_(100),
        max_vpos_(100),
        height_string_(-1),
        num_line_(1),
        offset_x_(0),
        caret_column_(0),
        caret_line_(1),
        caret_visible_(false),
        start_index_event_(0),
        current_event_(nullptr),
        current_event_on_rbutton_click_(nullptr){
        font_ = CreateFontW(16, 0, 0, 0, 0, 0, 0 ,0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH|FF_DONTCARE, L"Courier New");
        hbrush_grey_ = CreateSolidBrush(RGB(200, 200, 200));
        AddMessage(WM_SIZE, this, &Edit::OnSize);
        AddMessage(WM_ERASEBKGND, this, &Edit::OnEraseBkgnd);
        AddMessage(WM_PAINT, this, &Edit::OnPaint);
        AddMessage(WM_HSCROLL, this, &Edit::OnHScroll);
        AddMessage(WM_VSCROLL, this, &Edit::OnVScroll);
        AddMessage(WM_SETFOCUS, this, &Edit::OnSetFocus);
        AddMessage(WM_KILLFOCUS, this, &Edit::OnKillFocus);
        AddMessage(WM_MOUSEMOVE, this, &Edit::OnMouseMove);
        AddMessage(WM_LBUTTONDOWN, this, &Edit::OnLButtonDown);
        AddMessage(WM_RBUTTONDOWN, this, &Edit::OnRButtonDown);
        AddMessage(WM_FIND, this, &Edit::OnFind);
        AddMessage(WM_DESTROY, this, &Edit::OnDestroy);
        AddMessage(WM_MOUSEWHEEL, this, &Edit::OnMouseWheel);
        AddMessage(WM_CONTEXTMENU, this, &Edit::OnContextMenu);
        AddMessage(WM_COMMAND, this, &Edit::OnCommand);
    }

    Edit::~Edit(){
        DeleteObject(font_);
        DeleteObject(hbrush_grey_);
    }

    bool Edit::Create(HWND parent, wstring caption, int top, int left, int width, int height, UINT id){
        width_ = width;
        height_ = height;
        bool result = Wnd::Create(L"_Edit", parent, caption, WS_EX_CLIENTEDGE, WS_CHILD|WS_VISIBLE, top, left, width, height, id);
        SetClassLongPtrW(hwnd_, GCLP_HBRBACKGROUND, (LONG_PTR)(HBRUSH(COLOR_WINDOWFRAME)));

        HDC hdc = GetDC (hwnd_);
        SelectObject (hdc, font_);
        TEXTMETRICW text_metric;
        GetTextMetricsW(hdc, &text_metric);
        height_string_ = text_metric.tmHeight + text_metric.tmExternalLeading;
        ReleaseDC (hwnd_, hdc);

        return result;
    }

    void Edit::SetMaxVPos(int max_vpos){
        max_vpos_ = max_vpos;
        SetVScrollInfo(hwnd_, vpos_, max_vpos_);
    }

    void Edit::UpdateDisplay(){
        MEASUREMENT;
        hpos_ = 0;
        vpos_ = 0;
        num_line_ = 1;
        offset_x_ = 0;
        caret_column_ = 0;
        caret_line_ = 1;
        start_index_event_ = 0;
        max_vpos_ = dw_->CountLines();
        current_event_ = nullptr;
        SetHScrollInfo(hwnd_, max_hpos_);
        SetVScrollInfo(hwnd_, vpos_, max_vpos_);
        InvalidateRect(hwnd_, NULL, false);
    }

    void Edit::SetBottomMargin(int bottom_margin){
        bottom_margin_ = bottom_margin;
    }

    LRESULT Edit::OnSize(WPARAM wparam, LPARAM lparam){
        tagRECT r;
        //Получаем размер клиентской части главного окна
        GetClientRect(GetParent(hwnd_), &r);
        MoveWindow(hwnd_, r.left, r.top + top_margin_, r.right - r.left, r.bottom - r.top - top_margin_ - bottom_margin_, true);

        //Получаем размер клиентской области для вывода текста
        GetClientRect(hwnd_, &r);
        width_ = r.right - r.left;
        height_ = r.bottom - r.top;

        SCROLLINFO scrInfoH;
        scrInfoH.cbSize = sizeof(SCROLLINFO);
        scrInfoH.nPage = 10;
        scrInfoH.nMin = 0;
        scrInfoH.nMax = max_hpos_;
        scrInfoH.fMask = SIF_RANGE | SIF_PAGE;
        SetScrollInfo(hwnd_, SB_HORZ, &scrInfoH, TRUE);

        SetVScrollInfo(hwnd_, vpos_, max_vpos_);
        InvalidateRect(hwnd_, NULL, false);

        return 0;
    }

    LRESULT Edit::OnEraseBkgnd(WPARAM wparam, LPARAM lparam){
        return 1;
    }

    void Edit::DisplayCaret(HDC hdc){
        bool display_caret = false;
        if(display_lines_.size() && caret_line_ >= num_line_ && caret_line_ < num_line_ + display_lines_.size()){
            vector<int> dx = TextPoint(hdc, display_lines_[caret_line_ - num_line_].second);
            int caret_x = 0;
            if(caret_column_ > 0){
                if(caret_column_ < dx.size())
                    caret_x = dx[caret_column_ - 1];
                else
                    caret_x = dx.back();
            }
            if(caret_x >= offset_x_){
                SetCaretPos(caret_x - offset_x_, (caret_line_  - num_line_) * height_string_);
                display_caret = true;
            }
        }
        if(caret_visible_ && !display_caret){
            HideCaret(hwnd_);
            caret_visible_ = false;
        }
        else if(!caret_visible_ && display_caret){
            ShowCaret(hwnd_);
            caret_visible_ = true;
        }
    }

    void Edit::MarkCurrentEvent(int x, int y){
        int display_line = y/height_string_;
               
        if(display_line < display_lines_.size()){
            caret_line_ = num_line_ + y/height_string_;
            current_event_ = make_unique<TechLog1C::EventOut>(dw_->GetEvent(caret_line_));
            HDC hdc = GetDC(hwnd_);
            SelectObject(hdc, font_);
            vector<int> dx = TextPoint(hdc, display_lines_[display_line].second);
            caret_column_ = lower_bound(dx.begin(), dx.end(), x + offset_x_) - dx.begin();
            ReleaseDC(hwnd_, hdc);
            InvalidateRect(hwnd_, NULL, false);
        }
    }

    unique_ptr<TechLog1C::EventOut> Edit::GetCurrentEvent(int x, int y){
        int display_line = y/height_string_;
        if(display_line < display_lines_.size()){
            int caret_line = num_line_ + y/height_string_;
            return move(make_unique<TechLog1C::EventOut>(dw_->GetEvent(caret_line)));
        }
        else{
            return nullptr;
        }
    }

    LRESULT Edit::OnPaint(WPARAM wparam, LPARAM lparam){

        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd_, &ps);
        HDC hdc_mem = CreateCompatibleDC(hdc);
        SetBkMode(hdc_mem, TRANSPARENT);
        HBITMAP hbitmap = CreateCompatibleBitmap(hdc, width_, height_);
        SelectObject(hdc_mem, hbitmap);
        SelectObject(hdc_mem, font_);
        FillRect(hdc_mem, &ps.rcPaint, HBRUSH(COLOR_WINDOWFRAME));

        display_lines_ = dw_->GetLines(num_line_, height_/height_string_);
        if(!display_lines_.empty()){
            if(!current_event_){
                MarkCurrentEvent(0, 0);
            }
            pair<unsigned int, unsigned int> cur_event_lines = {current_event_->count_lines_agr_ - current_event_->count_lines_ + 1, current_event_->count_lines_agr_ };

            int max_width = MaxSizeString(hdc_mem, display_lines_);
            offset_x_ = hpos_ * max_width/max_hpos_;
            int y = 0;
            bool process_out = false;
            for(int i = 0; i < display_lines_.size(); ++i){
                if(num_line_ + i >= cur_event_lines.first && num_line_ + i <= cur_event_lines.second){
                    RECT rect = {0, y, ps.rcPaint.right, y + height_string_};
                    FillRect(hdc_mem, &rect, hbrush_grey_);
                    if(!process_out){
                        Msg(GetParent(hwnd_), Utf8ToWideChar(current_event_->process_));
                    }                
                }
                if(line_color_.size()){
                    auto it = line_color_.find(display_lines_[i].first);
                    if(it != line_color_.end()){
                        ExtStringOut(hdc_mem, display_lines_[i].second, -offset_x_, y, it->second);
                    }
                    else{
                        ExtStringOut(hdc_mem, display_lines_[i].second, -offset_x_, y);                        
                    }
                }
                else{
                    ExtStringOut(hdc_mem, display_lines_[i].second, -offset_x_, y);
                }
                y += height_string_;
            }
        }
        DisplayCaret(hdc_mem);
        
        BitBlt(hdc, 0, 0, width_, height_, hdc_mem, 0, 0, SRCCOPY);
        DeleteObject(hbitmap);
        DeleteDC(hdc_mem);
        EndPaint(hwnd_, &ps);

        return 0;
    }

    LRESULT Edit::OnHScroll(WPARAM wparam, LPARAM lparam){
        SCROLLINFO scrInfo;
        scrInfo.cbSize = sizeof(SCROLLINFO);
        scrInfo.fMask = SIF_ALL; //получаем текущие параметры scrollbar-а
        GetScrollInfo(hwnd_, SB_HORZ, &scrInfo);

        int currentPos = scrInfo.nPos; //запоминаем текущее положение содержимого

        switch (LOWORD(wparam)) {
            case SB_LINERIGHT:
                ++scrInfo.nPos;
                break; 
            case SB_LINELEFT:
                --scrInfo.nPos;
                break;
            case SB_THUMBTRACK:
                scrInfo.nPos = scrInfo.nTrackPos;
                break;
            default:
                return 0; //все прочие действия (например нажатие PageUp/PageDown) игнорируем
        }   

        scrInfo.fMask = SIF_POS; //пробуем применить новое положение
        SetScrollInfo(hwnd_, SB_HORZ, &scrInfo, TRUE);
        GetScrollInfo(hwnd_, SB_HORZ, &scrInfo);
        hpos_ = scrInfo.nPos;

        tagRECT r;
        GetClientRect(hwnd_, &r);
        InvalidateRect(hwnd_, &r, true);
        UpdateWindow(hwnd_);

        return 0;        
    }

    LRESULT Edit::OnVScroll(WPARAM wparam, LPARAM lparam){
        SCROLLINFO scrInfo;
        scrInfo.cbSize = sizeof(SCROLLINFO);
        scrInfo.fMask = SIF_ALL; //получаем текущие параметры scrollbar-а
        GetScrollInfo(hwnd_, SB_VERT, &scrInfo);

        int currentPos = scrInfo.nPos; //запоминаем текущее положение содержимого

        switch (LOWORD(wparam)) {
            case SB_LINEDOWN:
                ++scrInfo.nPos;
                break; 
            case SB_LINEUP:
                --scrInfo.nPos;
                break;
            case SB_THUMBTRACK:
                scrInfo.nPos = scrInfo.nTrackPos;
                break;
            default:
                return 0; //все прочие действия (например нажатие PageUp/PageDown) игнорируем
        }   

        scrInfo.fMask = SIF_POS; //пробуем применить новое положение
        SetScrollInfo(hwnd_, SB_VERT, &scrInfo, TRUE);
        GetScrollInfo(hwnd_, SB_VERT, &scrInfo);
        vpos_ = scrInfo.nPos;
        num_line_ = vpos_ + 1;

        tagRECT r;
        GetClientRect(hwnd_, &r);
        InvalidateRect(hwnd_, &r, true);
        UpdateWindow(hwnd_);

        return 0;
    }

    LRESULT Edit::OnSetFocus(WPARAM wparam, LPARAM lparam){
        CreateCaret(hwnd_, (HBITMAP) NULL, 1, 16);
        SetCaretPos(0, 0);
        ShowCaret(hwnd_);
        caret_visible_ = true;
        
        HDC hdc = GetDC(hwnd_);
        SelectObject(hdc, font_);
        DisplayCaret(hdc);
        ReleaseDC (hwnd_, hdc);
        
        return 0;
    }

    LRESULT Edit::OnKillFocus(WPARAM wparam, LPARAM lparam){
        HideCaret(hwnd_);
        DestroyCaret();
        caret_visible_ = false;
        return 0;
    }

    LRESULT Edit::OnMouseMove(WPARAM wparam, LPARAM lparam){
        // int x = LOWORD(lparam);
        // int y = HIWORD(lparam);

        // if(display_lines_.size()){
        //     int display_line = y/height_string_;
        //     TechLog1C::Event* event = events_->GetEvent(events_->IndexEvent(display_line));
        //     SendMessageW(GetParent(hwnd_), SB_SETTEXTW, SB_SIMPLEID, (LPARAM)&(event->Process())[0]);
        // }
        return 0;
    }

    LRESULT Edit::OnLButtonDown(WPARAM wparam, LPARAM lparam){
        SetFocus(hwnd_);
        MarkCurrentEvent(LOWORD(lparam), HIWORD(lparam));
        return 0;
    }

    LRESULT Edit::OnRButtonDown(WPARAM wparam, LPARAM lparam){
        SetFocus(hwnd_);
        return 0;
    }

    LRESULT Edit::OnFind(WPARAM wparam, LPARAM lparam){
        wstring str((wchar_t*)wparam, lparam);
        dw_->Find(str);
        UpdateDisplay();
        return 0;
    }

    LRESULT Edit::OnDestroy(WPARAM wparam, LPARAM lparam){
        PostQuitMessage(0);
        return 0;
    }

    LRESULT Edit::OnMouseWheel(WPARAM wparam, LPARAM lparam){
        short delta = GET_WHEEL_DELTA_WPARAM(wparam)/WHEEL_DELTA;
        
        if(delta){
            vpos_ -= delta;
            if(vpos_ < 0) vpos_ = 0;
            if(vpos_ > max_vpos_) vpos_ = max_vpos_;
            num_line_ = vpos_ + 1;

            SCROLLINFO scrInfo;
            scrInfo.cbSize = sizeof(SCROLLINFO);
            scrInfo.fMask = SIF_ALL; //получаем текущие параметры scrollbar-а
            GetScrollInfo(hwnd_, SB_VERT, &scrInfo);

            scrInfo.fMask = SIF_POS; //пробуем применить новое положение
            scrInfo.nPos = vpos_;
            SetScrollInfo(hwnd_, SB_VERT, &scrInfo, TRUE);
            
            tagRECT r;
            GetClientRect(hwnd_, &r);
            InvalidateRect(hwnd_, &r, true);
            UpdateWindow(hwnd_);
        }

        return 0;
    }

    void Edit::AppendMenuExt(HMENU h_pop_menu_, UINT id, wstring menu_str, wstring filter){
        AppendMenuW(h_pop_menu_, MF_STRING, id, menu_str.c_str());
        selection_condition_[id] = filter;
    }

    LRESULT Edit::OnContextMenu(WPARAM wparam, LPARAM lparam){
        if(display_lines_.size()){
            int x = LOWORD(lparam);
            int y = HIWORD(lparam);
            POINT pt = {x, y};
            MapWindowPoints(HWND_DESKTOP, hwnd_, &pt, 1);
            
            current_event_on_rbutton_click_ = GetCurrentEvent(pt.x, pt.y); 
            bool is_current_event = current_event_on_rbutton_click_ && (*current_event_on_rbutton_click_ == *current_event_);

            HMENU h_pop_menu = CreatePopupMenu();
            
            if(is_current_event){
                AppendMenuW(h_pop_menu, MF_STRING, ID_EDIT_COPY, L"Копировать текущее событие");
                AppendMenuW(h_pop_menu, MFT_SEPARATOR, 0, L"");
            }                    
            
            if(current_event_on_rbutton_click_){
                TechLog1C::JournalEntryObject entry_object = dw_->LoadEntryObject(current_event_on_rbutton_click_->id_);
                wstring event_name = Utf8ToWideChar(entry_object.event_name_);
                wstring usr = Utf8ToWideChar(entry_object.usr_);
                wstring session_id = Utf8ToWideChar(entry_object.session_id_);
                unsigned int t_connect_id = entry_object.tconnect_id_;
                wstring p_process_name = Utf8ToWideChar(entry_object.p_process_name_);

                if(event_name == L"TDEADLOCK"){
                    AppendMenuW(h_pop_menu, MF_STRING, ID_ANALIZE_TDEADLOCK, L"Анализ взаимоблокировки");
                    AppendMenuW(h_pop_menu, MFT_SEPARATOR, 0, L"");                
                }
                else if(event_name == L"TTIMEOUT"){
                    AppendMenuW(h_pop_menu, MF_STRING, ID_ANALIZE_TTIMEOUT, L"Анализ таймаута ожидания");
                    AppendMenuW(h_pop_menu, MFT_SEPARATOR, 0, L"");                
                }
                else if(event_name == L"TLOCK" && !entry_object.wait_connections_.empty()){
                    AppendMenuW(h_pop_menu, MF_STRING, ID_ANALIZE_TLOCK, L"Анализ ожидания");
                    AppendMenuW(h_pop_menu, MFT_SEPARATOR, 0, L"");                    
                }
                
                AppendMenuExt(h_pop_menu, ID_CUSTOM_EVENT, wstring(L"Отбор по имени события ").append(event_name), wstring(L"Event=\"").append(event_name).append(L"\""));   
                if(!usr.empty()){
                    AppendMenuExt(h_pop_menu, ID_USR, wstring(L"Отбор по пользователю Usr = \"").append(usr).append(L"\""), wstring(L"Usr=\"").append(usr).append(L"\""));
                }
                if(!session_id.empty()){
                    AppendMenuExt(h_pop_menu, ID_SESSION_ID, wstring(L"Отбор по сеансу SessionID = \"").append(session_id).append(L"\""), wstring(L"SessionID=\"").append(session_id).append(L"\""));
                }
                if(t_connect_id){
                    AppendMenuExt(h_pop_menu, ID_CONNECT_ID, wstring(L"Отбор по соединению t:connectID = ").append(to_wstring(t_connect_id)), wstring(L"t:connectID=").append(to_wstring(t_connect_id)));
                }
                if(!p_process_name.empty()){
                    AppendMenuExt(h_pop_menu, ID_P_PROCESS_NAME, wstring(L"Отбор по серверному контексту p:processName = \"").append(p_process_name).append(L"\""), wstring(L"p:processName=\"").append(p_process_name).append(L"\""));
                }
                AppendMenuW(h_pop_menu, MFT_SEPARATOR, 0, L"");
            }

            AppendMenuExt(h_pop_menu, ID_EXCP, L"EXCP - отбор исключений", L"Event=\"EXCP\"");
            AppendMenuExt(h_pop_menu, ID_QERR, L"QERR - отбор ошибок компиляции запросов", L"Event=\"QERR\"");
            AppendMenuExt(h_pop_menu, ID_TTIMEOUT, L"TTIMEOUT - отбор таймаутов управляемых блокировок", L"Event=\"TTIMEOUT\"");
            AppendMenuExt(h_pop_menu, ID_TDEADLOCK, L"TDEADLOCK - отбор управляемых взаимоблокировок", L"Event=\"TDEADLOCK\"");
            AppendMenuExt(h_pop_menu, ID_TLOCK, L"TLOCK - отбор ожиданий управляемых блокировках", L"Event=\"TLOCK\" AND WaitConnections <> \"\"");
            
            TrackPopupMenu(h_pop_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, hwnd_, NULL);
            DestroyMenu(h_pop_menu);
        }
        return 0;
    }

    LRESULT Edit::OnCommand(WPARAM wparam, LPARAM lparam){
        UINT id = LOWORD(wparam); 
        if(id == ID_EDIT_COPY){
            CopyClipboard(current_event_->AsWString());
        }
        else if(
            line_color_.clear();
            id == ID_CUSTOM_EVENT
            || id == ID_USR
            || id == ID_SESSION_ID
            || id == ID_CONNECT_ID
            || id == ID_P_PROCESS_NAME
            || id == ID_EXCP
            || id == ID_QERR
            || id == ID_TTIMEOUT
            || id == ID_TDEADLOCK
            || id == ID_TLOCK)
            {
                SendMessageW(GetParent(hwnd_), WM_MENU_SELECTION, (WPARAM)ID_MENU_ITEM_FIND, (LPARAM)selection_condition_[id].c_str());
            }
        else if(id == ID_ANALIZE_TDEADLOCK || id == ID_ANALIZE_TTIMEOUT || id == ID_ANALIZE_TLOCK){
            line_color_.clear();
            vector<pair<uint32_t, uint8_t>> id_rows;
            if(id == ID_ANALIZE_TDEADLOCK){
               id_rows = dw_->AnalyzeTDeadlock(current_event_on_rbutton_click_.get());
            }
            else if(id == ID_ANALIZE_TTIMEOUT){
                id_rows = dw_->AnalyzeTTimeOut(current_event_on_rbutton_click_.get());
            }
            else if(id == ID_ANALIZE_TLOCK){
                id_rows = dw_->AnalyzeTLock(current_event_on_rbutton_click_.get());
            }
            wstring find = L"IdRow IN (";
            wstring delimetr = L"";
            for(auto it = id_rows.begin(); it != id_rows.end(); ++it){
                find.append(delimetr).append(to_wstring(it->first));
                if(delimetr.empty()) delimetr = L",";
                if(it->second == 0){
                    line_color_.insert({it->first, RGB(200, 0, 0)});
                }
                else if(it->second == 1){
                    line_color_.insert({it->first, RGB(0, 0, 250)});
                }
            }
            find.append(L")");
            SendMessageW(GetParent(hwnd_), WM_MENU_SELECTION, (WPARAM)ID_MENU_ITEM_FIND, (LPARAM)find.c_str());
        }

        return 0;
    }

}