#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <fstream>
#include <vector>
#include <list>
#include "encoding_string.h"
#include "db.h"
#include "reader.h"

namespace TechLog1C{

    class Time{
        std::uint64_t time_;
        friend bool operator<(const Time& lhs, const Time& rhs);
        friend bool operator==(const Time& lhs, const Time& rhs);
    public:
        Time();
        Time(std::uint64_t time);
        Time(std::uint8_t year, std::uint8_t month, std::uint8_t day, std::uint8_t hour, std::uint8_t minute, std::uint8_t second, std::uint32_t mseconds);
        int Year(){ return time_/10000000000000000; }
        int Month(){ return (time_%10000000000000000)/100000000000000; }
        int Day(){ return (time_%100000000000000)/1000000000000; }
        int Hour(){ return (time_%1000000000000)/10000000000; }
        std::uint64_t GetTime() const { return time_; }
    };

    struct EventIn{
        Time time_;
        unsigned int file_id_;
        unsigned long long start_;
        unsigned int length_;
        unsigned int count_lines_;
        int process_id_;
        int event_id_;
        int p_process_name_id_;
        int usr_id_;
        unsigned int t_connect_id_;
        int session_id_;
        int wait_connections_id_;
    };

    struct EventOut{
        Reader* reader_;
        Time time_;
        unsigned long long start_;
        unsigned int length_;
        unsigned int count_lines_;
        unsigned int count_lines_agr_;
        std::string process_;
        unsigned int id_;

        EventOut();
        EventOut(const EventOut& event_out);
        EventOut(EventOut&& event_out);
        EventOut(Reader* reader, uint64_t start, uint32_t length); 
        EventOut(Reader* reader, Time time, uint64_t start, uint32_t length, uint32_t count_lines, uint32_t count_lines_agr, std::string process, uint32_t id);
        std::string AsString();
        std::wstring AsWString();
        std::vector<std::wstring> AsWStrings();
    };

    bool operator==(const EventOut& lhs, const EventOut& rhs);
    bool operator!=(const EventOut& lhs, const EventOut& rhs);

}