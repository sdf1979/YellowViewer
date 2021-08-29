#pragma once

#include "sqlite3.h"
#include <string>
#include <filesystem>
#include <unordered_set>
#include "encoding_string.h"
#include <iostream>

namespace TechLog1C{

    static const uint32_t ALL_DATA_LOAD = 0;

    struct JornalRecord{
        uint32_t file_id_;
        uint64_t time_;
        uint64_t start_;
        uint32_t length_;
        uint32_t count_lines_;
        uint32_t count_lines_agr_;
        uint32_t process_id_;
        uint32_t event_id_;
        uint32_t p_process_name_id_;
        uint32_t usr_id_;
        uint32_t t_connect_id_;
        uint32_t session_id_;
        uint32_t wait_connections_;
    };

    struct JournalEntry{
        uint32_t file_id_;
        uint64_t time_;
        uint64_t start_;
        uint32_t length_;
        uint32_t count_lines_;
        uint32_t count_lines_agr_;
        const unsigned char* process_;
        uint32_t id_;
    };

    struct JournalEntryObject{
        uint32_t id_;
        uint32_t file_id_;
        uint64_t start_;
        uint32_t length_;
        std::string event_name_;
        std::string usr_;
        std::string session_id_;
        uint32_t tconnect_id_;
        std::string p_process_name_;
        std::string wait_connections_;
    }; 

    class Db{
        sqlite3* db_;
        sqlite3_stmt* res_insert_file_;
        sqlite3_stmt* res_insert_process_;
        sqlite3_stmt* res_insert_event_name_;
        sqlite3_stmt* res_insert_event_;
        sqlite3_stmt* res_load_journal_;
        sqlite3_stmt* res_load_journal_find_;
        sqlite3_stmt* res_insert_find_table_;
        sqlite3_stmt* res_insert_p_process_name_;
        sqlite3_stmt* res_insert_usr_name_;
        sqlite3_stmt* res_insert_session_ids_;
        sqlite3_stmt* res_insert_wait_connections_;
        sqlite3_stmt* res_load_entry_object_;
        const std::wstring DB_NAME_;
        bool new_db_;
    public:
        Db();
        ~Db();
        void Open(std::wstring path);
        void Close();
        void BeginTrunsaction();
        void CommitTrunsaction();
        void RollbackTrunsaction();
        bool IsNew(){ return new_db_; }
        void InsertFile(uint32_t id, std::string_view name);
        void InsertProcess(uint32_t id, std::string_view name);
        void InsertEventName(uint32_t id, std::string_view name);
        void InsertProcessName(uint32_t id, std::string_view name);
        void InsertUsrName(uint32_t id, std::string_view name);
        void InsertSessionId(uint32_t id, std::string_view name);
        void InsertWaitConnections(uint32_t id, std::string_view name);
        void CreateIndex();
        void InsertEvent(const JornalRecord& journal_record);
        void ClearFindTable();
        void InsertFindTable(uint32_t id, uint32_t count_lines_agr);
        uint32_t CountEvents();
        uint32_t CountEventsFind();
        uint32_t CountLines();
        uint32_t CountLinesFind();
        void LoadFiles(void* obj, int (*fcn)(void*, int, char**, char**));
        void LoadJournalAll(void* obj, int (*fcn)(void*, const JournalEntry*), uint64_t start_line, uint32_t count_line);
        void LoadJournalFind(void* obj, int (*fcn)(void*, const JournalEntry*), uint64_t start_line, uint32_t count_line);
        void Find(void* obj, int (*fcn)(void*, int, char**, char**), std::string find_str);
        JournalEntryObject LoadEntryObject(uint32_t id);
        JournalEntryObject Next(uint32_t id, std::string event_name, uint32_t t_connect_id);
        JournalEntryObject Prev(uint32_t id, std::string event_name, uint32_t t_connect_id);
        JournalEntryObject Prev(uint32_t id_first, uint32_t id_second, std::string event_name, uint32_t t_connect_id);
        std::vector<JournalEntryObject> PrevAll(uint32_t id_first, uint32_t id_second, std::string event_name, uint32_t t_connect_id);
        void InsertString(uint32_t id, const std::string& str);
        std::string GetString(uint32_t id);
    };

}

