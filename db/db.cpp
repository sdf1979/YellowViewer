#include "db.h"

using namespace std;

namespace TechLog1C{

    void Exec(sqlite3* db, string sql){
        char* err = nullptr;
        sqlite3_exec(db, sql.c_str(), 0, 0, &err);
        if(err){
            string msg = "Ошибка выполнения запроса!\n";
            msg.append(err).append("\n\nТекст запроса:\n").append(sql);
            MessageBoxW(NULL, Utf8ToWideChar(msg).c_str(), 0, 0);
            sqlite3_free(err);
            throw std::invalid_argument("Invalid argument");
        }        
    }

    void Exec(sqlite3* db, string sql, void* obj, int (*fcn)(void*, int, char**, char**)){
        char* err = nullptr;
        sqlite3_exec(db, sql.c_str(), fcn, obj, &err);
        if(err){
            string msg = "Ошибка выполнения запроса!\n";
            msg.append(err).append("\n\nТекст запроса:\n").append(sql);
            MessageBoxW(NULL, Utf8ToWideChar(msg).c_str(), 0, 0);
            sqlite3_free(err);
            throw std::invalid_argument("Invalid argument");
        }        
    }

    void Prepare(sqlite3* db, const char* sql, sqlite3_stmt** res){
        if(sqlite3_prepare_v2(db, sql, -1, res, 0) != SQLITE_OK){
            auto err = sqlite3_errmsg(db);
            MessageBoxW(NULL, Utf8ToWideChar(err).c_str(), 0, 0);
            throw;
        }        
    }

    void PrepareInsertEvent(sqlite3* db, sqlite3_stmt** res){
        if(!*res){
            string sql = R"(
            INSERT INTO journal
            (
                file_id,
                time,
                start,
                lenght,
                count_lines,
                count_lines_agr,
                process_id,
                event_id,
                p_process_name_id,
                usr_id,
                t_connect_id,
                session_id,
                wait_connections_id
            )
            VALUES
            (
                @file_id,
                @time,
                @start,
                @end,
                @count_lines,
                @count_lines_agr,
                @process_id,
                @event_id,
                @p_process_name_id,
                @usr_id,
                @t_connect_id,
                @session_id,
                @wait_connections_id
            );)";
            Prepare(db, sql.c_str(), res);
        }        
    }

    void InsertEntity(sqlite3_stmt* res, uint32_t id, string_view name){
        sqlite3_bind_int(res, 1, id);
        sqlite3_bind_text(res, 2, &name[0], -1, 0);
        
        sqlite3_step(res);
        sqlite3_clear_bindings(res);
        sqlite3_reset(res);
    }

    void CreateTableStrings(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS strings(
            id INTEGER(4) PRIMARY KEY,
            string TEXT 
        );
        )");
    }

    void CreateTableFiles(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS files(
            id INTEGER(4) PRIMARY KEY,
            name TEXT 
        );
        )");
    }

    void CreateTableProcess(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS processes(
            id INTEGER(4) PRIMARY KEY,
            name TEXT 
        );
        )");
    }

    void CreateTableProcessNames(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS p_process_names(
            id INTEGER(4) PRIMARY KEY,
            name TEXT 
        );
        )");
    }

    void CreateIndexForProcessNames(sqlite3* db){
        Exec(db, R"(
        CREATE INDEX idx_name_p_process_names ON p_process_names (
            name,
            id
        );
        )");
    }

    void CreateTableUserNames(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS user_names(
            id INTEGER(4) PRIMARY KEY,
            name TEXT 
        );
        )");
    }

    void CreateIndexForUserNames(sqlite3* db){
        Exec(db, R"(
        CREATE INDEX idx_name_user_names ON user_names (
            name,
            id
        );
        )");
    }

    void CreateTableEvents(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS events(
            id INTEGER(4) PRIMARY KEY,
            name TEXT 
        );
        )");
    }

    void CreateIndexForEvents(sqlite3* db){
        Exec(db, R"(
        CREATE INDEX idx_name_events ON events (
            name,
            id
        );
        )");
    }

    void CreateTableJournal(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS journal(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            file_id INTEGER(4),
            time INTEGER(8),
            start INTEGER(8),
            lenght INTEGER(4),
            count_lines INTEGER(4),
            count_lines_agr INTEGER(4),
            process_id INTEGER(4),
            event_id INTEGER(4),
            p_process_name_id INTEGER(4),
            usr_id INTEGER(4),
            t_connect_id INTEGER(4),
            session_id INTEGER(4),
            wait_connections_id INTEGER(4)
        );
        )");
    }

    void CreateIndexForJournal(sqlite3* db){
        Exec(db, R"(
        CREATE INDEX idx_event_id_journal ON journal (
            event_id,
            id
        );
        CREATE INDEX idx_p_process_name_id_journal ON journal (
            p_process_name_id,
            id
        );
        CREATE INDEX idx_usr_id_journal ON journal (
            usr_id,
            id
        );
        CREATE INDEX idx_count_lines_agr_journal ON journal (
            count_lines_agr ASC,
            id
        );
        CREATE INDEX idx_t_connect_id_id_journal ON journal (
            t_connect_id,
            id
        );
        CREATE INDEX idx_session_id_id_journal ON journal (
            session_id,
            id
        );
        CREATE INDEX idx_wait_connections_id_id_journal ON journal (
            wait_connections_id,
            id
        );
        CREATE INDEX idx_time_id_journal ON journal (
            time,
            id
        );
        )");
    }

    void CreateTableFindResult(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS mem.find_result(
            id INTEGER PRIMARY KEY,
            count_lines_agr INTEGER(4)            
        );)");
        Exec(db, R"(
        CREATE INDEX IF NOT EXISTS mem.idx_count_lines_agr_find_result ON find_result (
            count_lines_agr ASC,
            id
        );
        )");
    }

    void CreateTableSessionIds(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS session_ids(
            id INTEGER(4) PRIMARY KEY,
            name TEXT 
        );
        )");
    }

    void CreateIndexForSessionIds(sqlite3* db){
        Exec(db, R"(
        CREATE INDEX idx_name_session_ids ON session_ids (
            name,
            id
        );
        )");
    }

    void CreateTableWaitConnections(sqlite3* db){
        Exec(db, R"(
        CREATE TABLE IF NOT EXISTS wait_connections(
            id INTEGER(4) PRIMARY KEY,
            name TEXT 
        );
        )");
    }

    void CreateIndexForWaitConnections(sqlite3* db){
        Exec(db, R"(
        CREATE INDEX idx_name_wait_connections ON session_ids (
            name,
            id
        );
        )");
    }

    void CreateDb(sqlite3* db){
        CreateTableStrings(db);
        CreateTableFiles(db);
        CreateTableProcess(db);
        CreateTableProcessNames(db);
        CreateTableUserNames(db);
        CreateTableEvents(db);
        CreateTableJournal(db);
        CreateTableSessionIds(db);
        CreateTableWaitConnections(db);        
    }

    void CreateIndexDb(sqlite3* db){
        CreateIndexForProcessNames(db);
        CreateIndexForUserNames(db);
        CreateIndexForEvents(db);
        CreateIndexForJournal(db);
        CreateIndexForSessionIds(db);
        CreateIndexForWaitConnections(db);
    }

    void FinalizeRes(sqlite3_stmt* res){
        sqlite3_finalize(res);
        res = nullptr;
    }

    Db::Db():
        db_(nullptr),
        res_insert_file_(nullptr),
        res_insert_process_(nullptr),
        res_insert_event_name_(nullptr),
        res_insert_event_(nullptr),
        res_load_journal_(nullptr),
        res_load_journal_find_(nullptr),
        res_insert_find_table_(nullptr),
        res_insert_p_process_name_(nullptr),
        res_insert_usr_name_(nullptr),
        res_insert_session_ids_(nullptr),
        res_insert_wait_connections_(nullptr),
        res_load_entry_object_(nullptr),
        DB_NAME_(L"data.sqlite3"){}

    Db::~Db(){
        Close();
    }

    void Db::Open(wstring path){
        Close();
        wstring db_file = path.append(L"\\").append(DB_NAME_);
        new_db_ = !std::filesystem::exists(std::filesystem::path(db_file));
        sqlite3_open(WideCharToUtf8(db_file).c_str(), &db_);
        Exec(db_, "ATTACH DATABASE ':memory:' AS mem;");
        Exec(db_, "PRAGMA synchronous = OFF;");
        Exec(db_, "PRAGMA temp_store = MEMORY;");
        Exec(db_, "PRAGMA journal_mode = OFF;");
        Exec(db_, "PRAGMA cache_size = 7000;");
        
        if(new_db_){
            CreateDb(db_);
            CreateIndex();
        }
        CreateTableFindResult(db_);
        PrepareInsertEvent(db_, &res_insert_event_);
    }

    void Db::Close(){
        if(!res_insert_file_) FinalizeRes(res_insert_file_);
        if(!res_insert_process_) FinalizeRes(res_insert_process_);
        if(!res_insert_event_name_) FinalizeRes(res_insert_event_name_);
        if(!res_insert_event_) FinalizeRes(res_insert_event_);
        if(!res_load_journal_) FinalizeRes(res_load_journal_);
        if(!res_load_journal_find_) FinalizeRes(res_load_journal_find_);
        if(!res_insert_find_table_) FinalizeRes(res_insert_find_table_);
        if(!res_insert_p_process_name_) FinalizeRes(res_insert_p_process_name_);
        if(!res_insert_usr_name_) FinalizeRes(res_insert_usr_name_);
        if(!res_load_entry_object_) FinalizeRes(res_load_entry_object_);
        if(db_){
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    void Db::BeginTrunsaction(){
        Exec(db_, "BEGIN TRANSACTION;");
    }

    void Db::CommitTrunsaction(){
        Exec(db_, "COMMIT TRANSACTION;");        
    }

    void Db::RollbackTrunsaction(){
        Exec(db_, "ROLLBACK TRANSACTION;");        
    }

    void Db::InsertFile(uint32_t id, string_view name){
        if(!res_insert_file_){
            Prepare(db_, "INSERT INTO files VALUES(@id, @name);", &res_insert_file_);
        }
        InsertEntity(res_insert_file_, id, name);
    }

    void Db::InsertProcess(uint32_t id, string_view name){
        if(!res_insert_process_){
            Prepare(db_, "INSERT INTO processes VALUES(@id, @name);", &res_insert_process_);
        }
        InsertEntity(res_insert_process_, id, name);
    }

    void Db::InsertEventName(uint32_t id, string_view name){
        if(!res_insert_event_name_){
            Prepare(db_, "INSERT INTO events VALUES(@id, @name);", &res_insert_event_name_);
        }
        InsertEntity(res_insert_event_name_, id, name);
    }

    void Db::InsertProcessName(uint32_t id, std::string_view name){
        if(!res_insert_p_process_name_){
            Prepare(db_, "INSERT INTO p_process_names VALUES(@id, @name);", &res_insert_p_process_name_);
        }
        InsertEntity(res_insert_p_process_name_, id, name);      
    }

    void Db::InsertUsrName(uint32_t id, std::string_view name){
        if(!res_insert_usr_name_){
            Prepare(db_, "INSERT INTO user_names VALUES(@id, @name);", &res_insert_usr_name_);
        }
        InsertEntity(res_insert_usr_name_, id, name);
    }

    void Db::InsertSessionId(uint32_t id, std::string_view name){
        if(!res_insert_session_ids_){
            Prepare(db_, "INSERT INTO session_ids VALUES(@id, @name);", &res_insert_session_ids_);
        }
        InsertEntity(res_insert_session_ids_, id, name);
    }

    void Db::InsertWaitConnections(uint32_t id, std::string_view name){
        if(!res_insert_wait_connections_){
            Prepare(db_, "INSERT INTO wait_connections VALUES(@id, @name);", &res_insert_wait_connections_);
        }
        InsertEntity(res_insert_wait_connections_, id, name);
    }

    void Db::CreateIndex(){
        BeginTrunsaction();
        CreateIndexDb(db_);
        CommitTrunsaction();
    }

    void Db::InsertEvent(const JornalRecord& journal_record){
        sqlite3_bind_int(res_insert_event_, 1, journal_record.file_id_);
        sqlite3_bind_int64(res_insert_event_, 2, journal_record.time_);
        sqlite3_bind_int64(res_insert_event_, 3, journal_record.start_);
        sqlite3_bind_int64(res_insert_event_, 4, journal_record.length_);
        sqlite3_bind_int(res_insert_event_, 5, journal_record.count_lines_);
        sqlite3_bind_int(res_insert_event_, 6, journal_record.count_lines_agr_);
        sqlite3_bind_int(res_insert_event_, 7, journal_record.process_id_);
        sqlite3_bind_int(res_insert_event_, 8, journal_record.event_id_);
        sqlite3_bind_int(res_insert_event_, 9, journal_record.p_process_name_id_);
        sqlite3_bind_int(res_insert_event_, 10, journal_record.usr_id_);
        sqlite3_bind_int(res_insert_event_, 11, journal_record.t_connect_id_);
        sqlite3_bind_int(res_insert_event_, 12, journal_record.session_id_);
        sqlite3_bind_int(res_insert_event_, 13, journal_record.wait_connections_);

        sqlite3_step(res_insert_event_);
        sqlite3_clear_bindings(res_insert_event_);
        sqlite3_reset(res_insert_event_);
    }

    void Db::ClearFindTable(){
        Exec(db_, "DROP INDEX mem.idx_count_lines_agr_find_result;");
        Exec(db_, "DROP TABLE mem.find_result;");
        CreateTableFindResult(db_);        
    }

    void Db::InsertFindTable(uint32_t id, uint32_t count_lines_agr){
        if(!res_insert_find_table_){
            string sql = "INSERT INTO mem.find_result VALUES(@id, @count_lines_agr);";
            Prepare(db_, sql.c_str(), &res_insert_find_table_);
        }
        sqlite3_bind_int(res_insert_find_table_, 1, id);
        sqlite3_bind_int(res_insert_find_table_, 2, count_lines_agr);
                
        sqlite3_step(res_insert_find_table_);
        sqlite3_clear_bindings(res_insert_find_table_);
        sqlite3_reset(res_insert_find_table_);
    }

    uint32_t Db::CountEvents(){
        sqlite3_stmt* res;
        sqlite3_prepare_v2(db_, "SELECT MAX(id) FROM journal;", -1, &res, 0);
        sqlite3_step(res);
        uint32_t count = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
                
        return count;
    }

    uint32_t Db::CountEventsFind(){
        sqlite3_stmt* res;
        sqlite3_prepare_v2(db_, "SELECT COUNT(*) FROM mem.find_result;", -1, &res, 0);
        sqlite3_step(res);
        uint32_t count = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
                
        return count;
    }

    uint32_t Db::CountLines(){
        sqlite3_stmt* res;
        sqlite3_prepare_v2(db_, "SELECT count_lines_agr FROM journal ORDER BY id DESC LIMIT 1;", -1, &res, 0);
        sqlite3_step(res);
       uint32_t count = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        
        return count;
    }

    uint32_t Db::CountLinesFind(){
        sqlite3_stmt* res;
        sqlite3_prepare_v2(db_, "SELECT count_lines_agr FROM mem.find_result ORDER BY count_lines_agr DESC LIMIT 1;", -1, &res, 0);
        sqlite3_step(res);
        uint32_t count = sqlite3_column_int(res, 0);
        sqlite3_finalize(res);
        
        return count;
    }

    void Db::LoadFiles(void* obj, int (*fcn)(void*, int, char**, char**)){
        string sql = "SELECT id, name FROM files ORDER BY id;";
        Exec(db_, sql, obj, fcn);
    }

    void LoadJournalPrepare(sqlite3* db, sqlite3_stmt** res, const char* sql){
        if(!*res){
            Prepare(db, sql, res);
        }
    }

    void LoadJournal(sqlite3_stmt** res, void* obj, int (*fcn)(void*, const JournalEntry*), uint64_t start_line, uint32_t count_line){
        sqlite3_bind_int(*res, 1, start_line);
        sqlite3_bind_int(*res, 2, start_line + count_line);

        int step = sqlite3_step(*res);
        while(step == SQLITE_ROW){
            JournalEntry je = {
                static_cast<uint32_t>(sqlite3_column_int(*res, 0)),
                static_cast<uint64_t>(sqlite3_column_int64(*res, 1)),
                static_cast<uint64_t>(sqlite3_column_int64(*res, 2)),
                static_cast<uint32_t>(sqlite3_column_int(*res, 3)),
                static_cast<uint32_t>(sqlite3_column_int(*res, 4)),
                static_cast<uint32_t>(sqlite3_column_int(*res, 5)),
                sqlite3_column_text(*res, 6),
                static_cast<uint32_t>(sqlite3_column_int(*res, 7))
            };

            fcn(obj, &je);
            step = sqlite3_step(*res);            
        }
        sqlite3_reset(*res);
    }

    void Db::LoadJournalAll(void* obj, int (*fcn)(void*, const JournalEntry*), uint64_t start_line, uint32_t count_line){
        if(!db_) return;

        if(!res_load_journal_){
            string sql = R"(
            SELECT
                journal.file_id,
                journal.time,
                journal.start,
                journal.lenght,
                journal.count_lines,
                journal.count_lines_agr,
                processes.name,
                journal.id
            FROM
                journal
            INNER JOIN
                processes
            ON
                processes.id = journal.process_id
            WHERE
                journal.id >= (SELECT id FROM journal WHERE count_lines_agr >= @start ORDER BY count_lines_agr ASC LIMIT 1)
                and journal.id<= IFNULL((SELECT id FROM journal WHERE count_lines_agr >= @end ORDER BY count_lines_agr ASC LIMIT 1), 4294967296)
            ORDER BY
                journal.id
            )";
            LoadJournalPrepare(db_, &res_load_journal_, sql.c_str());
        }
        LoadJournal(&res_load_journal_, obj, fcn, start_line, count_line);
    }

    void Db::LoadJournalFind(void* obj, int (*fcn)(void*, const JournalEntry*), uint64_t start_line, uint32_t count_line){
        if(!db_) return;

        if(!res_load_journal_find_){
            string sql = R"(
            SELECT
                journal.file_id,
                journal.time,
                journal.start,
                journal.lenght,
                journal.count_lines,
                find_result.count_lines_agr,
                processes.name,
                journal.id
            FROM
                mem.find_result as find_result
            INNER JOIN
                journal
            ON
                journal.id = find_result.id
            LEFT JOIN
                processes
            ON
                processes.id = journal.process_id
            WHERE
                find_result.id >= (SELECT id FROM mem.find_result WHERE count_lines_agr >= @start ORDER BY count_lines_agr ASC LIMIT 1)
                and find_result.id<= IFNULL((SELECT id FROM mem.find_result WHERE count_lines_agr >= @end ORDER BY count_lines_agr ASC LIMIT 1), 4294967296)
            ORDER BY
                find_result.id
            )";
            LoadJournalPrepare(db_, &res_load_journal_find_, sql.c_str());
        }
        LoadJournal(&res_load_journal_find_, obj, fcn, start_line, count_line);
    }

    enum class KeyTable{
        ID_ROW,
        EVENT,
        P_PROCESS_NAME,
        USR,
        T_CONNECT_ID,
        SESSION_ID,
        WAIT_CONNECTIONS,
        TIME
    };

    unordered_set<KeyTable> KeysTable(string_view find_str){
        unordered_set<KeyTable> keys_table;

        if(find_str.find("IdRow") != string_view::npos) keys_table.insert(KeyTable::ID_ROW);
        if(find_str.find("Event") != string_view::npos) keys_table.insert(KeyTable::EVENT);
        if(find_str.find("p:processName") != string_view::npos) keys_table.insert(KeyTable::P_PROCESS_NAME);
        if(find_str.find("Usr") != string_view::npos) keys_table.insert(KeyTable::USR);
        if(find_str.find("t:connectID") != string_view::npos) keys_table.insert(KeyTable::T_CONNECT_ID);
        if(find_str.find("SessionID") != string_view::npos) keys_table.insert(KeyTable::SESSION_ID);
        if(find_str.find("WaitConnections") != string_view::npos) keys_table.insert(KeyTable::WAIT_CONNECTIONS);
        if(find_str.find("Time") != string_view::npos) keys_table.insert(KeyTable::TIME);
        
        return keys_table;
    }

    void Replace(string& source_str, const string& find_str, const string& replace_str){
        size_t pos = 0;
        while((pos = source_str.find(find_str, pos)) != string::npos) {
            source_str.replace(pos, find_str.length(), replace_str);
            pos += replace_str.length();
        }
    }

    void ReplaceTime(string& source_str){
        string find = "Time";
        string_view sv(source_str);
        size_t pos = 0;
        while((pos = sv.find(find)) != string_view::npos){ 
            sv.remove_prefix(pos);
            pos = sv.find("\"");
            pos = sv.find("\"", pos + 1);
            sv.remove_suffix(sv.size() - pos - 1);
            string find_str(sv);

            sv = find_str; 
            pos = sv.find(find);
            sv.remove_prefix(pos + find.size());

            pos = sv.find("\"");
            string replace_str = "journal.time";
            replace_str.append(sv.substr(0, pos));
            sv.remove_prefix(pos + 1);

            uint64_t time = 0;
            pos = sv.find(".");
            if(pos != string_view::npos){
                time = atoi(string(sv.substr(0, pos)).c_str()) * 1000000000000LL;
                sv.remove_prefix(pos + 1);
                pos = sv.find(".");
                if(pos != string_view::npos){
                    time += atoi(string(sv.substr(0, pos)).c_str()) * 100000000000000LL;
                    sv.remove_prefix(pos + 1);
                    pos = sv.find(" ");
                    if(pos != string_view::npos){
                        time += atoi(string(sv.substr(0, pos)).c_str()) * 10000000000000000LL;
                        sv.remove_prefix(pos + 1);
                        pos = sv.find(":");
                        if(pos != string_view::npos){
                            time += atoi(string(sv.substr(0, pos)).c_str()) * 10000000000LL;
                            sv.remove_prefix(pos + 1);
                            pos = sv.find(":");
                            if(pos != string_view::npos){
                                time += atoi(string(sv.substr(0, pos)).c_str()) * 100000000LL;
                                sv.remove_prefix(pos + 1);
                                pos = sv.find(".");
                                if(pos != string_view::npos){
                                    time += atoi(string(sv.substr(0, pos)).c_str()) * 1000000LL;
                                    sv.remove_prefix(pos + 1);
                                    time += atoll(string(sv).c_str());
                                }
                                else{
                                    time += atoi(string(sv).c_str()) * 1000000LL;                                    
                                }
                            }
                        }
                    }                    
                }
            }
            replace_str.append(to_string(time));
            Replace(source_str, find_str, replace_str);
            sv = source_str;
        }
        
    }

    void Db::Find(void* obj, int (*fcn)(void*, int, char**, char**), string find_str){
        unordered_set<KeyTable> keys_table = KeysTable(find_str);
        if(keys_table.empty()) return; 

        string cmd = "SELECT journal.id, journal.count_lines FROM journal";

        if(keys_table.find(KeyTable::ID_ROW) != keys_table.end()){
            Replace(find_str, "IdRow", "journal.id");
        }

        if(keys_table.find(KeyTable::EVENT) != keys_table.end()){
            cmd.append(" LEFT JOIN events ON events.id = journal.event_id");
            Replace(find_str, "Event", "events.name");
        }

        if(keys_table.find(KeyTable::P_PROCESS_NAME) != keys_table.end()){
            cmd.append(" LEFT JOIN p_process_names ON p_process_names.id = journal.p_process_name_id");
            Replace(find_str, "p:processName", "p_process_names.name");
        }

        if(keys_table.find(KeyTable::USR) != keys_table.end()){
            cmd.append(" LEFT JOIN user_names ON user_names.id = journal.usr_id");
            Replace(find_str, "Usr", "user_names.name");
        }

        if(keys_table.find(KeyTable::T_CONNECT_ID) != keys_table.end()){
            Replace(find_str, "t:connectID", "journal.t_connect_id");
        }

        if(keys_table.find(KeyTable::SESSION_ID) != keys_table.end()){
            cmd.append(" LEFT JOIN session_ids ON session_ids.id = journal.session_id");
            Replace(find_str, "SessionID", "session_ids.name");
        }

        if(keys_table.find(KeyTable::WAIT_CONNECTIONS) != keys_table.end()){
            cmd.append(" LEFT JOIN wait_connections ON wait_connections.id = journal.wait_connections_id");
            Replace(find_str, "WaitConnections", "wait_connections.name");
        }

        if(keys_table.find(KeyTable::TIME) != keys_table.end()){
            ReplaceTime(find_str);
        }

        if(keys_table.size()){
            cmd.append(" WHERE ").append(find_str);
        }

        cmd.append(" ORDER BY journal.id");
        //TODO DEBUG
        //std::wcout << Utf8ToWideChar(cmd) << std::endl;
        Exec(db_, cmd, obj, fcn); 
    }

    JournalEntryObject JournalEntryObjectFromRes(sqlite3_stmt* res){
        return
        {
            static_cast<uint32_t>(sqlite3_column_int(res, 0)),
            static_cast<uint32_t>(sqlite3_column_int(res, 1)),
            static_cast<uint64_t>(sqlite3_column_int(res, 2)),
            static_cast<uint32_t>(sqlite3_column_int(res, 3)),
            string(reinterpret_cast<const char*>(sqlite3_column_text(res, 4))),
            string(reinterpret_cast<const char*>(sqlite3_column_text(res, 5))),
            string(reinterpret_cast<const char*>(sqlite3_column_text(res, 6))),
            static_cast<uint32_t>(sqlite3_column_int(res, 7)),
            string(reinterpret_cast<const char*>(sqlite3_column_text(res, 8))),
            string(reinterpret_cast<const char*>(sqlite3_column_text(res, 9)))
        };
    }

    JournalEntryObject Db::LoadEntryObject(uint32_t id){
        if(!res_load_entry_object_){
            string sql = 
            R"(SELECT
                journal.id,
                journal.file_id,
                journal.start,
                journal.lenght,
                events.name,
                user_names.name,
                session_ids.name,
                journal.t_connect_id,
                p_process_names.name,
                wait_connections.name
            FROM
                journal
            INNER JOIN
                events ON events.id = journal.event_id
            INNER JOIN
                user_names ON user_names.id = journal.usr_id
            INNER JOIN
                session_ids ON session_ids.id = journal.session_id
            INNER JOIN
                p_process_names ON p_process_names.id = journal.p_process_name_id
            INNER JOIN
                wait_connections ON wait_connections.id = journal.wait_connections_id
            WHERE
                journal.id = @id;
            
            )";
            Prepare(db_, sql.c_str(), &res_load_entry_object_);            
        }

        sqlite3_bind_int(res_load_entry_object_, 1, id);

        int step = sqlite3_step(res_load_entry_object_);
        if(step == SQLITE_ROW){
            JournalEntryObject entry_object = JournalEntryObjectFromRes(res_load_entry_object_);
            sqlite3_reset(res_load_entry_object_);
            return entry_object;
        }
        else{
            sqlite3_reset(res_load_entry_object_);
            return {};
        }
    }

    JournalEntryObject Db::Next(uint32_t id, string event_name, uint32_t t_connect_id){
        sqlite3_stmt* res = nullptr;
        if(!res){
            string sql = 
            R"(SELECT
                journal.id,
                journal.file_id,
                journal.start,
                journal.lenght,
                events.name,
                user_names.name,
                session_ids.name,
                journal.t_connect_id,
                p_process_names.name,
                wait_connections.name
            FROM
                journal
            INNER JOIN
                events ON events.id = journal.event_id and events.name = @event_name 
            INNER JOIN
                user_names ON user_names.id = journal.usr_id
            INNER JOIN
                session_ids ON session_ids.id = journal.session_id
            INNER JOIN
                p_process_names ON p_process_names.id = journal.p_process_name_id
            INNER JOIN
                wait_connections ON wait_connections.id = journal.wait_connections_id
            WHERE
                journal.id > @id
                and journal.t_connect_id = @t_connect_id
            ORDER BY
                journal.id
            LIMIT
                1
            )";
            Prepare(db_, sql.c_str(), &res);            
        }

        sqlite3_bind_text(res, 1, &event_name[0], -1, 0);
        sqlite3_bind_int(res, 2, id);
        sqlite3_bind_int(res, 3, t_connect_id);

        int step = sqlite3_step(res);
        if(step == SQLITE_ROW){
            JournalEntryObject entry_object = JournalEntryObjectFromRes(res);
            sqlite3_reset(res);
            sqlite3_finalize(res);
            return entry_object;
        }
        else{
            sqlite3_reset(res_load_entry_object_);
            sqlite3_finalize(res);
            return {};
        }
    }

    JournalEntryObject Db::Prev(uint32_t id, string event_name, uint32_t t_connect_id){
        sqlite3_stmt* res = nullptr;
        if(!res){
            string sql = 
            R"(SELECT
                journal.id,
                journal.file_id,
                journal.start,
                journal.lenght,
                events.name,
                user_names.name,
                session_ids.name,
                journal.t_connect_id,
                p_process_names.name,
                wait_connections.name
            FROM
                journal
            INNER JOIN
                events ON events.id = journal.event_id and events.name = @event_name 
            INNER JOIN
                user_names ON user_names.id = journal.usr_id
            INNER JOIN
                session_ids ON session_ids.id = journal.session_id
            INNER JOIN
                p_process_names ON p_process_names.id = journal.p_process_name_id
            INNER JOIN
                wait_connections ON wait_connections.id = journal.wait_connections_id
            WHERE
                journal.id < @id
                and journal.t_connect_id = @t_connect_id
            ORDER BY
                journal.id DESC
            LIMIT
                1
            )";
            Prepare(db_, sql.c_str(), &res);            
        }

        sqlite3_bind_text(res, 1, &event_name[0], -1, 0);
        sqlite3_bind_int(res, 2, id);
        sqlite3_bind_int(res, 3, t_connect_id);

        int step = sqlite3_step(res);
        if(step == SQLITE_ROW){
            JournalEntryObject entry_object = JournalEntryObjectFromRes(res);
            sqlite3_reset(res);
            sqlite3_finalize(res);
            return entry_object;
        }
        else{
            sqlite3_reset(res_load_entry_object_);
            sqlite3_finalize(res);
            return {};
        }
    }

    JournalEntryObject Db::Prev(uint32_t id_first, uint32_t id_second, string event_name, uint32_t t_connect_id){
        sqlite3_stmt* res = nullptr;
        if(!res){
            string sql = 
            R"(SELECT
                journal.id,
                journal.file_id,
                journal.start,
                journal.lenght,
                events.name,
                user_names.name,
                session_ids.name,
                journal.t_connect_id,
                p_process_names.name,
                wait_connections.name
            FROM
                journal
            INNER JOIN
                events ON events.id = journal.event_id and events.name = @event_name 
            INNER JOIN
                user_names ON user_names.id = journal.usr_id
            INNER JOIN
                session_ids ON session_ids.id = journal.session_id
            INNER JOIN
                p_process_names ON p_process_names.id = journal.p_process_name_id
            INNER JOIN
                wait_connections ON wait_connections.id = journal.wait_connections_id
            WHERE
                journal.id > @id_first
                and journal.id < @id_second
                and journal.t_connect_id = @t_connect_id
            ORDER BY
                journal.id DESC
            LIMIT
                1
            )";
            Prepare(db_, sql.c_str(), &res);            
        }

        sqlite3_bind_text(res, 1, &event_name[0], -1, 0);
        sqlite3_bind_int(res, 2, id_first);
        sqlite3_bind_int(res, 3, id_second);
        sqlite3_bind_int(res, 4, t_connect_id);

        int step = sqlite3_step(res);
        if(step == SQLITE_ROW){
            JournalEntryObject entry_object = JournalEntryObjectFromRes(res);
            sqlite3_reset(res);
            sqlite3_finalize(res);
            return entry_object;
        }
        else{
            sqlite3_reset(res_load_entry_object_);
            sqlite3_finalize(res);
            return {};
        }
    }

    vector<JournalEntryObject> Db::PrevAll(uint32_t id_first, uint32_t id_second, std::string event_name, uint32_t t_connect_id){
        sqlite3_stmt* res = nullptr;
        if(!res){
            string sql = 
            R"(SELECT
                journal.id,
                journal.file_id,
                journal.start,
                journal.lenght,
                events.name,
                user_names.name,
                session_ids.name,
                journal.t_connect_id,
                p_process_names.name,
                wait_connections.name
            FROM
                journal
            INNER JOIN
                events ON events.id = journal.event_id and events.name = @event_name 
            INNER JOIN
                user_names ON user_names.id = journal.usr_id
            INNER JOIN
                session_ids ON session_ids.id = journal.session_id
            INNER JOIN
                p_process_names ON p_process_names.id = journal.p_process_name_id
            INNER JOIN
                wait_connections ON wait_connections.id = journal.wait_connections_id
            WHERE
                journal.id > @id_first
                and journal.id < @id_second
                and journal.t_connect_id = @t_connect_id
            ORDER BY
                journal.id DESC
            )";
            Prepare(db_, sql.c_str(), &res);            
        }

        sqlite3_bind_text(res, 1, &event_name[0], -1, 0);
        sqlite3_bind_int(res, 2, id_first);
        sqlite3_bind_int(res, 3, id_second);
        sqlite3_bind_int(res, 4, t_connect_id);

        vector<JournalEntryObject> result;

        int step = sqlite3_step(res);
        while(step == SQLITE_ROW){
            result.push_back(JournalEntryObjectFromRes(res));
            step = sqlite3_step(res);
        }
        sqlite3_reset(res);
        sqlite3_finalize(res);

        return result;
    }

    void Db::InsertString(uint32_t id, const string& str){
        sqlite3_stmt* res = nullptr;
        if(!res){
            Prepare(db_, "INSERT INTO strings VALUES(@id, @str);", &res);
        }
        sqlite3_bind_int(res, 1, id);
        sqlite3_bind_text(res, 2, &str[0], -1, 0);
        
        sqlite3_step(res);
        sqlite3_clear_bindings(res);
        sqlite3_reset(res);
    }

    string Db::GetString(uint32_t id){
        string result;
        sqlite3_stmt* res = nullptr;
        if(!res){
            Prepare(db_, "SELECT string FROM strings WHERE id = @id;", &res);
        }
        sqlite3_bind_int(res, 1, id);
        int step = sqlite3_step(res);
        if(step == SQLITE_ROW){
            result = string(reinterpret_cast<const char*>(sqlite3_column_text(res, 0)));
        }
        sqlite3_reset(res);
        sqlite3_finalize(res);
        return result;
    }

}