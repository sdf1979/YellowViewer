#pragma once

#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <memory>
#include <time.h>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

#include "reader.h"
#include "parser.h"
#include "text_data.h"
#include "db.h"
#include "timer_perf.h"

namespace TechLog1C{
    using FcnMsg = void (*)(void* obj, int part, std::string); 
    using FcnInsertEntity = void (Db::*)(unsigned int id, std::string_view name);  

    using File = std::pair<std::unique_ptr<Reader>, std::unique_ptr<Parser>>;
    using Entities = std::vector<std::pair<uint32_t, std::string>>;
    using EntitiesSave = std::vector<std::pair<Entities, FcnInsertEntity>>;    

    class DirectoryWatcher{
        std::filesystem::path directory_;
        std::vector<File> files_;
        std::unique_ptr<Db> db_;
        unsigned int count_lines_;
        std::vector<EventOut> events_;
        std::vector<EventOut>* events_load_;
        std::string find_str_;
        unsigned int count_lines_agr_find_;
        std::atomic<bool> files_is_read_;
        std::uint64_t all_files_size_;

        void* obj_msg_;
        FcnMsg fcn_msg_;
              
        void AddFiles(std::filesystem::path path);
        void LoadJournal(std::vector<EventOut>* events, unsigned long long start_line, unsigned int count_line);
    public:
        DirectoryWatcher();
        ~DirectoryWatcher();
        void Close();
        bool ReadDirectory(std::wstring directory_name);
        void ReadFiles();
        EventOut GetEvent(unsigned long long line);
        //pair<id event, string line>
        std::vector<std::pair<std::uint64_t, std::wstring>> GetLines(unsigned long long start_line, unsigned int count_line);
        unsigned int CountLines();
        int CallbackLoadFiles(int argc, char **argv, char **azColName);
        int CallbackLoadJournal(const JournalEntry* journal_entry);
        int CallbackFind(int argc, char **argv, char **azColName);

        void AddCallbackMessage(void* obj, FcnMsg fcn);

        void Find(std::wstring find_str);
        JournalEntryObject LoadEntryObject(unsigned int id);
        std::vector<std::pair<uint32_t, uint8_t>> AnalyzeTDeadlock(EventOut* event_out);
        std::vector<std::pair<uint32_t, uint8_t>> AnalyzeTTimeOut(EventOut* event_out);
        std::vector<std::pair<uint32_t, uint8_t>> AnalyzeTLock(EventOut* event_out);
    };

}