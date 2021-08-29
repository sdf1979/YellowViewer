#include "directory_watcher.h"

namespace TechLog1C{

    using namespace std;

    mutex mutex_read_files_commit_trunsaction;

    static int c_callback_load_files(void *param, int argc, char **argv, char **azColName){
        DirectoryWatcher* dw = reinterpret_cast<DirectoryWatcher*>(param);
        return dw->CallbackLoadFiles(argc, argv, azColName);
    }

    static int c_callback_load_journal(void *param, const JournalEntry* journal_entry){
        DirectoryWatcher* dw = reinterpret_cast<DirectoryWatcher*>(param);
        return dw->CallbackLoadJournal(journal_entry);
    }

    static int c_callback_find(void *param, int argc, char **argv, char **azColName){
        DirectoryWatcher* dw = reinterpret_cast<DirectoryWatcher*>(param);
        return dw->CallbackFind(argc, argv, azColName);
    }

    void SaveEntity(Db* db, EntitiesSave& entities_save){
        for(auto it_entities_save = entities_save.begin(); it_entities_save != entities_save.end(); ++it_entities_save){
            FcnInsertEntity fcn_insert_entity = it_entities_save->second;
            for(auto it_entities = it_entities_save->first.begin(); it_entities != it_entities_save->first.end(); ++it_entities){
                (db->*fcn_insert_entity)(it_entities->first, it_entities->second);                
            }            
        }
    }

    void ReadFilesCommitTrunsaction(TechLog1C::Db* db, vector<JornalRecord> journal_records, uint32_t count_lines_agr, EntitiesSave entities_save){
        MEASUREMENT;
        // std::sort(journal_records.begin(), journal_records.end(), [](const JornalRecord& lhs, const JornalRecord& rhs)
        // {
        //     return lhs.time_ < rhs.time_;
        // });

        db->BeginTrunsaction();

        SaveEntity(db, entities_save);
        for(auto it = journal_records.begin();it != journal_records.end();++it){
            count_lines_agr += it->count_lines_;
            it->count_lines_agr_ = count_lines_agr; 
            db->InsertEvent(*it);
        }
        db->CommitTrunsaction();
        mutex_read_files_commit_trunsaction.unlock();
    }

    //static auto LOGGER = YellowWatcher::Logger::getInstance();
    DirectoryWatcher::DirectoryWatcher():
        count_lines_(0),
        files_is_read_(false),
        all_files_size_(0),
        obj_msg_(nullptr),
        fcn_msg_(nullptr),
        db_(make_unique<Db>()){}

    DirectoryWatcher::~DirectoryWatcher(){
        db_->Close();
    }

    int DirectoryWatcher::CallbackLoadFiles(int argc, char **argv, char **azColName){
        int file_id = std::atoi(argv[0]);
        wstring file_name = directory_.wstring();
        file_name.append(L"\\").append(Utf8ToWideChar(string(argv[1])));

        unique_ptr<Reader> new_file = make_unique<Reader>(file_id, file_name, 65535);
        files_.push_back({ move(new_file), make_unique<Parser>() });

        return 0;
    }

    int DirectoryWatcher::CallbackLoadJournal(const JournalEntry* journal_entry){
        events_load_->push_back(
            {
                files_[journal_entry->file_id_].first.get(),
                Time(journal_entry->time_),
                journal_entry->start_,
                journal_entry->length_,
                journal_entry->count_lines_,
                journal_entry->count_lines_agr_,
                string(reinterpret_cast<const char*>(journal_entry->process_)),
                journal_entry->id_
            });
        return 0;
    }

    int DirectoryWatcher::CallbackFind(int argc, char **argv, char **azColName){
        int id = std::atoi(argv[0]);
        int count_lines = std::atoll(argv[1]);
        count_lines_agr_find_ += count_lines;
        db_->InsertFindTable(id, count_lines_agr_find_) ;

        return 0;
    }

    void DirectoryWatcher::AddFiles(filesystem::path path){
        wstring file_name = path.wstring();
        int size_directory = directory_.wstring().size() + 1; 
        string file_name_short = WideCharToUtf8(file_name.substr(size_directory, file_name.size() - size_directory));
        int file_id = files_.size();
        db_->InsertFile(file_id, file_name_short);

        unique_ptr<Reader> new_file = make_unique<Reader>(file_id, file_name, 65535);
          all_files_size_ += new_file->GetFileSize();
        files_.push_back({ move(new_file), make_unique<Parser>() });
    }    

    void DirectoryWatcher::LoadJournal(std::vector<EventOut>* events, unsigned long long start_line, unsigned int count_line){
        events_load_ = events;
        events_load_->clear();
        if(find_str_.size()){
            db_->LoadJournalFind(this, c_callback_load_journal, start_line, count_line);
        }
        else{
            db_->LoadJournalAll(this, c_callback_load_journal, start_line, count_line);
        }
    }

    void DirectoryWatcher::ReadDirectory(wstring directory_name){
        MEASUREMENT;
        time_t cur_time = chrono::system_clock::to_time_t(chrono::system_clock::now());
        directory_ = filesystem::path(directory_name);

        if(!filesystem::exists(directory_)){
            wstring err = L"Каталог '";
            err.append(directory_name).append(L"' не найден или нет доступа!");

            MessageBoxW(NULL, err.c_str(), 0, 0);            
            return;
        }

        files_.clear();

        db_->Open(directory_);
        db_->BeginTrunsaction();        
        
        if(db_->IsNew()){
            if(obj_msg_ && fcn_msg_){
                stringstream ss;
                ss << "Подсчет данных:";
                fcn_msg_(obj_msg_, 1, ss.str());
                ss.clear();
            }

            vector<pair<filesystem::path, unsigned int>> log_files;
            try{
                for(const std::filesystem::directory_entry& dir: std::filesystem::recursive_directory_iterator(directory_)){
                    if(dir.is_regular_file() && dir.path().extension().string() == ".log"){
                        log_files.push_back({dir, Reader::Time(dir.path().wstring())});
                    }
                }
            }
            catch(std::system_error error){
                // string msg(directory_.string());
                // msg.append(";").append(error.what());
                // LOGGER->Print(msg, Logger::Type::Error);
                throw error;
            }
            std::sort(log_files.begin(), log_files.end(), [](const pair<filesystem::path, unsigned int>& lhs, const pair<filesystem::path, unsigned int>& rhs){
                return lhs.second < rhs.second;
            });
            for(auto it = log_files.begin();it != log_files.end(); ++it){
                AddFiles(it->first);
            }
            if(obj_msg_ && fcn_msg_){
                stringstream ss;
                ss << "Всего данных: " << Format(all_files_size_/1024.0/1024.0, 2) << " Мб";
                fcn_msg_(obj_msg_, 1, ss.str());
                ss.clear();
            }
        }
        else{
            db_->LoadFiles(this, c_callback_load_files);
        }

        db_->CommitTrunsaction();
    }

    unordered_map<string, uint32_t>::iterator InsertEntity(uint32_t& id, const string& name,  unordered_map<string, uint32_t>& entity_map, vector<pair<uint32_t, string>>& entity_new){
        auto res = entity_map.insert({name, id});
        if(res.second){
            entity_new.push_back({id, name});
            ++id;
        }
        return res.first;
    }

    void DirectoryWatcher::ReadFiles(){
        MEASUREMENT;
        if(db_->IsNew()){            
            uint32_t process_id = 0;          unordered_map<string, uint32_t> processes_map;            Entities processes_new;
            uint32_t event_id = 0;            unordered_map<string, uint32_t> events_map;               Entities events_new;
            uint32_t p_process_name_id = 0;   unordered_map<string, uint32_t> p_process_names_map;      Entities p_process_names_new;
            uint32_t usr_id = 0;              unordered_map<string, uint32_t> usr_map;                  Entities usr_new;
            uint32_t session_id = 0;          unordered_map<string, uint32_t> session_id_map;           Entities session_id_new;
            uint32_t wait_connections_id = 0; unordered_map<string, uint32_t> wait_connections_id_map;  Entities wait_connections_id_new;
            
            uint32_t time = 0;
            count_lines_ = 0;
            uint32_t count_lines_cur = 0;
            vector<JornalRecord> journal_records;
            journal_records.reserve(10000000);
            uint64_t load_size = 0;
            
            for(vector<File>::iterator it = files_.begin(); it != files_.end(); ++it){
                Reader* reader = it->first.get();
                Parser* parser = it->second.get();

                if(time != reader->Time()){
                    if(time){
                        EntitiesSave entities_save;
                        entities_save.push_back({move(processes_new), &Db::InsertProcess});
                        entities_save.push_back({move(events_new), &Db::InsertEventName});
                        entities_save.push_back({move(p_process_names_new), &Db::InsertProcessName});
                        entities_save.push_back({move(usr_new), &Db::InsertUsrName});
                        entities_save.push_back({move(session_id_new), &Db::InsertSessionId});
                        entities_save.push_back({move(wait_connections_id_new), &Db::InsertWaitConnections});
                        
                        std::sort(journal_records.begin(), journal_records.end(), [](const JornalRecord& lhs, const JornalRecord& rhs)
                        {
                            return lhs.time_ < rhs.time_;
                        });
                        mutex_read_files_commit_trunsaction.lock();
                        thread thread(ReadFilesCommitTrunsaction, db_.get(), move(journal_records), count_lines_, move(entities_save));
                        thread.detach();
                        
                        count_lines_ = count_lines_cur;
                        journal_records.reserve(10000000);
                    }
                    time = reader->Time();
                }

                if(reader->IsWorkingFile() && reader->Open()){
                    if(reader->Read()){
                        //Ищем процесс, если нет, добавляем
                        auto process_it = InsertEntity(process_id, reader->Process(), processes_map, processes_new);

                        while(reader->Next()){
                            auto buffer = reader->GetBuffer();
                            parser->Parse(buffer.first, buffer.second);
                            vector<EventData> events_temp = parser->MoveEvents();
                            EventData* event_data = &events_temp[0];
                            EventData* event_end = event_data + events_temp.size();
                            for(;event_data != event_end;++event_data){

                                //Ищем имя события, если нет, добавляем
                                auto event_it = InsertEntity(event_id, event_data->Name(), events_map, events_new);
                                //Ищем имя p:process_name, если нет, добавляем
                                auto p_process_name_it = InsertEntity(p_process_name_id, event_data->ProcessName(), p_process_names_map, p_process_names_new);
                                //Ищем имя Usr, если нет, добавляем
                                auto usr_it = InsertEntity(usr_id, event_data->Usr(), usr_map, usr_new);
                                //Ищем имя SessionID, если нет, добавляем
                                auto session_id_it = InsertEntity(session_id, event_data->SessionId(), session_id_map, session_id_new);
                                //Ищем имя WaitConnections, если нет, добавляем
                                auto wait_connections_it = InsertEntity(wait_connections_id, event_data->WaitConnections(), wait_connections_id_map, wait_connections_id_new);

                                journal_records.push_back(
                                    {
                                        reader->Id(),
                                        reader->Year() * 10000000000000000ULL
                                        + reader->Month() * 100000000000000ULL
                                        + reader->Day() * 1000000000000ULL
                                        + reader->Hour() * 10000000000ULL
                                        + event_data->Minute() * 100000000ULL
                                        + event_data->Second() * 1000000ULL
                                        + event_data->Msecond(),
                                        event_data->OffsetStart(),
                                        event_data->Lenght(),
                                        event_data->CountLines(),
                                        0,
                                        process_it->second,
                                        event_it->second,
                                        p_process_name_it->second,
                                        usr_it->second,
                                        event_data->ConnectId(),
                                        session_id_it->second,
                                        wait_connections_it->second
                                    });
                                count_lines_cur +=  event_data->CountLines();
                            }
                        }
                        parser->Clear();
                    }
                }
                reader->Close();
                load_size += reader->GetFileSize();
                if(obj_msg_ && fcn_msg_){
                    stringstream ss;
                    ss << "Всего данных: " << Format(all_files_size_/1024.0/1024.0, 2) << " Мб. ";
                    ss << "Обработано: " << Format(load_size/1024.0/1024.0, 2) << " Мб.";
                    fcn_msg_(obj_msg_, 1, ss.str());
                }

            }
            if(time){
                EntitiesSave entities_save;
                entities_save.push_back({move(processes_new), &Db::InsertProcess});
                entities_save.push_back({move(events_new), &Db::InsertEventName});
                entities_save.push_back({move(p_process_names_new), &Db::InsertProcessName});
                entities_save.push_back({move(usr_new), &Db::InsertUsrName});
                entities_save.push_back({move(session_id_new), &Db::InsertSessionId});
                entities_save.push_back({move(wait_connections_id_new), &Db::InsertWaitConnections});

                std::sort(journal_records.begin(), journal_records.end(), [](const JornalRecord& lhs, const JornalRecord& rhs)
                {
                    return lhs.time_ < rhs.time_;
                });
                mutex_read_files_commit_trunsaction.lock();
                ReadFilesCommitTrunsaction(db_.get(), move(journal_records), count_lines_, move(entities_save));
            }
            stringstream ss;
            ss << "Всего данных: " << Format(all_files_size_/1024.0/1024.0, 2) << " Мб. ";
            ss << "Событий: " << Format(db_->CountEvents(), 0) << ". Строк: " << Format(db_->CountLines(), 0) << ".";
            fcn_msg_(obj_msg_, 1, ss.str());
            db_->InsertString(ALL_DATA_LOAD, ss.str());
        }
        else{
            fcn_msg_(obj_msg_, 1, db_->GetString(ALL_DATA_LOAD));            
        }
        files_is_read_ = true;
    }

    EventOut DirectoryWatcher::GetEvent(unsigned long long line){
        if(!files_is_read_) return {};

        vector<EventOut> events;
        LoadJournal(&events, line, 0);
        if(events.size() == 1){
            return events[0];
        }
        return {};
    }

    std::vector<wstring> DirectoryWatcher::GetLines(unsigned long long start_line, unsigned int count_line){
         if(!files_is_read_) return {};

        LoadJournal(&events_, start_line, count_line);
                
        if(!events_.size()) return {};
        
        vector<wstring> lines;
        unsigned long long cur_line = events_[0].count_lines_agr_ - events_[0].count_lines_ + 1;
        unsigned long long end_line = start_line + count_line;
        for(auto it_events = events_.begin();it_events != events_.end();++it_events){
            vector<wstring> event_lines = it_events->AsWStrings();
            for(auto it_lines = event_lines.begin();it_lines != event_lines.end();++it_lines){
                if(cur_line >= start_line && cur_line <= end_line){
                    lines.push_back(move(*it_lines));
                }
                ++cur_line;
                if(cur_line > end_line) break;
            }
            if(cur_line > end_line) break;
        }
        return lines;
    }

    unsigned int DirectoryWatcher::CountLines(){
        if(find_str_.size()){
            count_lines_ = db_->CountLinesFind();            
        }
        else{
            count_lines_ = db_->CountLines();
        }
        return count_lines_;
    }

    void DirectoryWatcher::Find(std::wstring find_str){
        find_str_ = WideCharToUtf8(find_str);
        count_lines_agr_find_ = 0;
        db_->BeginTrunsaction();
        try{
            db_->ClearFindTable();
            if(find_str_.size()) db_->Find(this, c_callback_find, find_str_);

            db_->CommitTrunsaction();
        }
        catch(exception& ex){
            db_->RollbackTrunsaction();
        }
        if(find_str.empty()){
            fcn_msg_(obj_msg_, 1, db_->GetString(ALL_DATA_LOAD));
        }
        else{
            stringstream ss;
            ss << "Событий: " << Format(db_->CountEventsFind(), 0) << ". Строк: " << Format(db_->CountLinesFind(), 0) << ".";
            fcn_msg_(obj_msg_, 1, ss.str());            
        }     
    }

    void DirectoryWatcher::AddCallbackMessage(void* obj, FcnMsg fcn){
        obj_msg_ = obj;
        fcn_msg_ = fcn;
    }

    JournalEntryObject DirectoryWatcher::LoadEntryObject(unsigned int id){
        return db_->LoadEntryObject(id);
    }

    vector<uint32_t> DirectoryWatcher::AnalyzeTDeadlock(EventOut* event_out){

        //Формируем массив идентификаторов строк для вывода
        vector<uint32_t> id_rows;
        id_rows.push_back(event_out->id_);

        JournalEntryObject event_obj = LoadEntryObject(event_out->id_);
        string event_str = event_out->AsString();
        TDeadLock t_deadlock = AnalayzerLock::ParseTDeadLock(event_str);

        uint32_t id;

        //Блокировка первого соединения, которую не удалось установить
        JournalEntryObject connect_fail_first;
        id = event_obj.id_;
        TLock tlock_fail_first;
        for(;;){
            connect_fail_first = db_->Next(id, "TLOCK", t_deadlock.ConnectIdFirst());
            EventOut event_tmp(files_[connect_fail_first.file_id_].first.get(), connect_fail_first.start_, connect_fail_first.length_);
            tlock_fail_first = AnalayzerLock::ParseTLock(event_tmp.AsString());
            if(tlock_fail_first.CheckWaitConnection(t_deadlock.ConnectIdSecond()) || !connect_fail_first.id_){
                break;
            }
            id = connect_fail_first.id_; 
        }
        //Добавляем в массив идентификатор строки для вывода
        if(connect_fail_first.id_) id_rows.push_back(connect_fail_first.id_);

        //Блокировка первого соединения, которую не удалось установить
        JournalEntryObject connect_fail_second;
        id = event_obj.id_;
        TLock tlock_fail_second;
        for(;;){
            connect_fail_second = db_->Next(id, "TLOCK", t_deadlock.ConnectIdSecond());
            EventOut event_tmp(files_[connect_fail_second.file_id_].first.get(), connect_fail_second.start_, connect_fail_second.length_);
            tlock_fail_second = AnalayzerLock::ParseTLock(event_tmp.AsString());
            if(tlock_fail_second.CheckWaitConnection(t_deadlock.ConnectIdFirst()) || !connect_fail_second.id_){
                break;
            }
            id = connect_fail_second.id_; 
        }
        //Добавляем в массив идентификатор строки для вывода
        if(connect_fail_second.id_) id_rows.push_back(connect_fail_second.id_);
        
        //Начало транзакции первого соединения
        JournalEntryObject begin_trun_first;
        id = event_obj.id_; 
        for(;;){
            begin_trun_first = db_->Prev(id, "SDBL", t_deadlock.ConnectIdFirst());
            if(begin_trun_first.id_){
                EventOut event_tmp(files_[begin_trun_first.file_id_].first.get(), begin_trun_first.start_, begin_trun_first.length_);
                if(event_tmp.AsString().find("Func=BeginTransaction") != string::npos) break; 
                id = begin_trun_first.id_; 
            }
            else{
                break;
            }
        }
        //Добавляем в массив идентификатор строки для вывода
        if(begin_trun_first.id_) id_rows.push_back(begin_trun_first.id_);

        //Начало транзакции второго соединения
        JournalEntryObject begin_trun_second;
        id = event_obj.id_; 
        for(;;){
            begin_trun_second = db_->Prev(id, "SDBL", t_deadlock.ConnectIdSecond());
            if(begin_trun_second.id_){
                EventOut event_tmp(files_[begin_trun_second.file_id_].first.get(), begin_trun_second.start_, begin_trun_second.length_);
                if(event_tmp.AsString().find("Func=BeginTransaction") != string::npos) break;
                id = begin_trun_second.id_;
            }
            else{
                break;
            }
        }
        //Добавляем в массив идентификатор строки для вывода
        if(begin_trun_second.id_) id_rows.push_back(begin_trun_second.id_);

        //Успешные несовместимые блокировки первого соединения
        vector<JournalEntryObject> connects_succes = db_->PrevAll(begin_trun_first.id_, id, "TLOCK", t_deadlock.ConnectIdFirst());
        for(auto connect_succes = connects_succes.begin(); connect_succes != connects_succes.end(); ++connect_succes){
            EventOut event_tmp(files_[connect_succes->file_id_].first.get(), connect_succes->start_, connect_succes->length_);
            TLock tlock_success = AnalayzerLock::ParseTLock(event_tmp.AsString());
            if(tlock_success.Locks().IsLocking(t_deadlock.LocksSecond())){
                //Добавляем в массив идентификатор строки для вывода
                id_rows.push_back(connect_succes->id_);
            }
        }
        
        //Успешные несовместимые блокировки второго соединения
        connects_succes = db_->PrevAll(begin_trun_first.id_, id, "TLOCK", t_deadlock.ConnectIdSecond());
        for(auto connect_succes = connects_succes.begin(); connect_succes != connects_succes.end(); ++connect_succes){
            EventOut event_tmp(files_[connect_succes->file_id_].first.get(), connect_succes->start_, connect_succes->length_);
            TLock tlock_success = AnalayzerLock::ParseTLock(event_tmp.AsString());
            if(tlock_success.Locks().IsLocking(t_deadlock.LocksFirst())){
                //Добавляем в массив идентификатор строки для вывода
                id_rows.push_back(connect_succes->id_);
            }
        }
        
        sort(id_rows.begin(), id_rows.end());

        return id_rows;
    }

    vector<uint32_t> DirectoryWatcher::AnalyzeTTimeOut(EventOut* event_out){
        
        //Формируем массив идентификаторов строк для вывода
        vector<uint32_t> id_rows;
        id_rows.push_back(event_out->id_);

        JournalEntryObject event_obj = LoadEntryObject(event_out->id_);

        uint32_t id;

        //Ищем начало транзакции таймаута
        JournalEntryObject begin_trun_fail;
        id = event_obj.id_; 
        for(;;){
            begin_trun_fail = db_->Prev(id, "SDBL", event_obj.tconnect_id_);
            if(begin_trun_fail.id_){
                EventOut event_tmp(files_[begin_trun_fail.file_id_].first.get(), begin_trun_fail.start_, begin_trun_fail.length_);
                if(event_tmp.AsString().find("Func=BeginTransaction") != string::npos){
                    //Добавляем в массив идентификатор строки для вывода
                    id_rows.push_back(begin_trun_fail.id_);
                    break;
                }
                else{
                    id = begin_trun_fail.id_;
                }                
            }
            else{
                break;
            }
        }

        //Блокировка, которую не удалось установить
        JournalEntryObject connect_fail;
        id = event_obj.id_;
        TLock tlock_fail;
        for(;;){
            connect_fail = db_->Next(id, "TLOCK", event_obj.tconnect_id_);
            if(connect_fail.id_){
                if(event_obj.wait_connections_ == connect_fail.wait_connections_){
                    EventOut event_tmp(files_[connect_fail.file_id_].first.get(), connect_fail.start_, connect_fail.length_);
                    tlock_fail = AnalayzerLock::ParseTLock(event_tmp.AsString());
                    //Добавляем в массив идентификатор строки для вывода
                    id_rows.push_back(connect_fail.id_);
                    break;
                }
                else{
                    id = connect_fail.id_; 
                }                
            }
            else{
                break;
            }
        }
        
        //Для каждого соединения, из-за которого таймаут ищем начало транзакции и блокировки
        for(auto it_wait = tlock_fail.WaitConnection().begin(); it_wait != tlock_fail.WaitConnection().end(); ++it_wait){
            
            //Ищем начало транзакции
            JournalEntryObject begin_trun;
            id = event_obj.id_; 
            for(;;){
                begin_trun = db_->Prev(id, "SDBL", *it_wait);
                if(begin_trun.id_){
                    EventOut event_tmp(files_[begin_trun.file_id_].first.get(), begin_trun.start_, begin_trun.length_);
                    if(event_tmp.AsString().find("Func=BeginTransaction") != string::npos){
                        //Добавляем в массив идентификатор строки для вывода
                        id_rows.push_back(begin_trun.id_);
                        break; 
                    }
                    else{
                        id = begin_trun.id_; 
                    }                     
                }
                else{
                    break;
                }
            }
            
            //Ищем не совместимые блокировки
            id = event_obj.id_;
            vector<JournalEntryObject> connects_succes = db_->PrevAll(begin_trun.id_, id, "TLOCK", *it_wait);
            for(auto connect_succes = connects_succes.begin(); connect_succes != connects_succes.end(); ++connect_succes){
                EventOut event_tmp(files_[connect_succes->file_id_].first.get(), connect_succes->start_, connect_succes->length_);
                TLock tlock_success = AnalayzerLock::ParseTLock(event_tmp.AsString());
                if(tlock_success.Locks().IsLocking(tlock_fail.Locks())){
                    //Добавляем в массив идентификатор строки для вывода
                    id_rows.push_back(connect_succes->id_);
                }
            }
        }

        sort(id_rows.begin(), id_rows.end());

        return id_rows;
    }

    vector<uint32_t> DirectoryWatcher::AnalyzeTLock(EventOut* event_out){
        
        //Формируем массив идентификаторов строк для вывода
        vector<uint32_t> id_rows;
        id_rows.push_back(event_out->id_);

        uint32_t id;

        JournalEntryObject event_obj = LoadEntryObject(event_out->id_);
        EventOut event_tmp(files_[event_obj.file_id_].first.get(), event_obj.start_, event_obj.length_);
        TLock tlock = AnalayzerLock::ParseTLock(event_tmp.AsString());

        //Ищем начало транзакции блокировки с ожиданием
        JournalEntryObject begin_trun;
        id = event_obj.id_; 
        for(;;){
            begin_trun = db_->Prev(id, "SDBL", event_obj.tconnect_id_);
            if(begin_trun.id_){
                EventOut event_tmp(files_[begin_trun.file_id_].first.get(), begin_trun.start_, begin_trun.length_);
                if(event_tmp.AsString().find("Func=BeginTransaction") != string::npos){
                    //Добавляем в массив идентификатор строки для вывода
                    id_rows.push_back(begin_trun.id_);
                    break;
                }
                else{
                    id = begin_trun.id_;
                }                
            }
            else{
                break;
            }
        }

        //Для каждого соединения, из-за которого таймаут ищем начало транзакции и блокировки
        for(auto it_wait = tlock.WaitConnection().begin(); it_wait != tlock.WaitConnection().end(); ++it_wait){
            
            //Ищем начало транзакции
            JournalEntryObject begin_trun;
            id = event_obj.id_; 
            for(;;){
                begin_trun = db_->Prev(id, "SDBL", *it_wait);
                if(begin_trun.id_){
                    EventOut event_tmp(files_[begin_trun.file_id_].first.get(), begin_trun.start_, begin_trun.length_);
                    if(event_tmp.AsString().find("Func=BeginTransaction") != string::npos) break; 
                    id = begin_trun.id_; 
                }
                else{
                    break;
                }
            }
            //Добавляем в массив идентификатор строки для вывода
            if(begin_trun.id_) id_rows.push_back(begin_trun.id_);

            //Ищем не совместимые блокировки
            vector<JournalEntryObject> connects_succes = db_->PrevAll(begin_trun.id_, id, "TLOCK", *it_wait);
            for(auto connect_succes = connects_succes.begin(); connect_succes != connects_succes.end(); ++connect_succes){
                EventOut event_tmp(files_[connect_succes->file_id_].first.get(), connect_succes->start_, connect_succes->length_);
                TLock tlock_success = AnalayzerLock::ParseTLock(event_tmp.AsString());
                if(tlock_success.Locks().IsLocking(tlock.Locks())){
                    //Добавляем в массив идентификатор строки для вывода
                    id_rows.push_back(connect_succes->id_);
                }
            }
        }

        sort(id_rows.begin(), id_rows.end());

        return id_rows;
    }
}