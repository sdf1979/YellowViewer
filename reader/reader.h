#pragma once

#include <fstream>
#include <cstdint>
#include <string>
#include <time.h>
#include "timer_perf.h"

namespace TechLog1C{

    class Reader{
        unsigned int id_;
        std::ifstream fs_;
        std::uint64_t pos_;
        std::uint64_t pos_end_;
        char* buffer_;
        std::uint64_t size_read_;
        std::wstring file_name_;
        int size_buffer_;
        //time_t file_time_;
        //const time_t MAX_FILE__PROCESSING_TIME;

        std::uint8_t year_;
        std::uint8_t month_;
        std::uint8_t day_;
        std::uint8_t hour_;
        unsigned int time_;
        std::string process_;

        void FindStartPosition();

        friend bool operator<(const Reader& lhs, const Reader& rhs);
    public:
        Reader(unsigned int id, std::wstring file_name, unsigned int size_buffer);
        unsigned int Id(){ return id_; }
        std::ifstream* FS(){ return &fs_; }
        std::uint8_t Year(){ return year_; }
        std::uint8_t Month(){ return month_; }
        std::uint8_t Day(){ return day_; }
        std::uint8_t Hour(){ return hour_; }
        unsigned int Time(){ return time_; }
        static unsigned int Time(std::wstring file_name);
        const std::string& Process(){ return process_; }
        bool Open();
        bool Close();
        bool Read();
        bool Read(char* ch, std::uint64_t start, std::uint64_t size);
        bool Next();
        bool IsWorkingFile();
        std::uint64_t GetFileSize();
        const std::wstring& GetFileName();
        std::pair<const char*, int> GetBuffer();
        void ClearBuffer();
        ~Reader();
        bool operator==(const Reader& reader) const;
    };

}