#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
// #include <set>
// #include <map>
#include <stack>
#include <functional>
#include "timer_perf.h"

namespace TechLog1C{

    class Parser;
    using PtrFcn = void(*)(const char**, Parser* const);

    enum class TypeChar{
        Digit,
        Colon,
        Dot,
        Minus,
        Comma,
        CommaInQ,
        Equal,
        Quote,
        Dquote,
        NewLine,
        NewLineInQ,
        Char
    };

    class EventData{
        //ВНИМАНИЕ!!!
        //При добавлении новой переменной добавить в конструктор копирования и перемещения
        std::uint64_t offset_start_;
        std::uint64_t offset_end_;

        std::uint8_t minute_;
        std::uint8_t second_;
        std::uint32_t msecond_;
        std::uint32_t count_lines_;
        // std::size_t duration_;
        std::string name_;
        // int level_;
        // std::string process_;
        std::string p_process_name_;
        std::string usr_;
        // std::int64_t memory_;
        // std::int64_t memory_peak_;
        // std::int64_t in_bytes_;
        // std::int64_t out_bytes_;
        // std::int64_t cpu_time_;
        std::uint32_t t_connect_id_;
        std::string session_id_;
        std::string wait_connections_;
        // std::string descr_;
        // std::string i_name_;

        std::string key_temp_;
        std::string value_temp_;
        

        void EndValue();

        friend void Start(const char** ch, Parser* const parser);
        friend void Minute(const char** ch, Parser* const parser);
        friend void Second(const char** ch, Parser* const parser);
        friend void Msecond(const char** ch, Parser* const parser);
        friend void Duration(const char** ch, Parser* const parser);
        friend void Event(const char** ch, Parser* const parser);
        friend void Level(const char** ch, Parser* const parser);
        friend void Key(const char** ch, Parser* const parser);
        friend void Value(const char** ch, Parser* const parser);
        friend void EndValue(const char** ch, Parser* const parser);
        friend void EndEvent(const char** ch, Parser* const parser);
        friend void Mvalue(const char** ch, Parser* const parser);

        friend std::ostream& operator<< (std::ostream &out, const EventData& event_data);
    public:
        EventData();
        EventData(EventData&& event_data);
        EventData& operator=(EventData&& event_data);

        std::uint64_t OffsetStart() { return offset_start_; }
        std::uint64_t OffsetEnd() { return offset_end_; }
        std::uint32_t Lenght() { return offset_end_ - offset_start_; }
        std::uint8_t Minute() const { return minute_; }
        std::uint8_t Second() const { return second_; }
        std::uint32_t Msecond() const { return msecond_; }
        unsigned int CountLines() const { return count_lines_; }
        const std::string& Name() {return name_; }
        // std::size_t Duration() const { return duration_; }
        // const std::string& Name() const { return name_; }
        // int Level() const { return level_; }
        // const std::string& Process() const { return process_; }
        const std::string& ProcessName() const {return p_process_name_; }
        const std::string& Usr() const { return usr_; }
        // std::int64_t Memory() const { return memory_; }
        // std::int64_t MemoryPeak() const { return memory_peak_; }
        // std::int64_t InBytes() const { return in_bytes_; }
        // std::int64_t OutBytes() const { return out_bytes_; }
        // std::int64_t CpuTime() const { return cpu_time_; }
        std::uint32_t ConnectId() const { return t_connect_id_; }
        const std::string& SessionId() const { return session_id_; }
        const std::string& WaitConnections() const { return wait_connections_; }
        // const std::string& Descr() const { return descr_; }
        // const std::string& IName() const { return i_name_; }
    };

    class Parser{
        std::vector<EventData> events_;
        std::stack<char> stack_quotes;
        EventData event_data;
        PtrFcn ptr_fcn;
        PtrFcn ptr_fcn_break;
        std::uint64_t offset_;
        
        friend void Start(const char** ch, Parser* const parser);
        friend void Minute(const char** ch, Parser* const parser);
        friend void Second(const char** ch, Parser* const parser);
        friend void Msecond(const char** ch, Parser* const parser);
        friend void Duration(const char** ch, Parser* const parser);
        friend void Event(const char** ch, Parser* const parser);
        friend void Level(const char** ch, Parser* const parser);
        friend void Key(const char** ch, Parser* const parser);
        friend void Value(const char** ch, Parser* const parser);
        friend void EndValue(const char** ch, Parser* const parser);
        friend void EndEvent(const char** ch, Parser* const parser);
        friend void Mvalue(const char** ch, Parser* const parser);

        friend TypeChar GetTypeChar(char ch, Parser* const parser);
    public:
        Parser();
        void Parse(const char* ch, int size);
        void AddEvent(EventData&& event_data);
        std::vector<EventData>&& MoveEvents();
        void Clear();
    };

    enum class LockType{
        SHARED,
        EXCLUSIVE
    };

    class Lock{
    private:
        std::string lock_space_;
        LockType lock_type_;
        std::unordered_map<std::string, std::unordered_set<std::string>> lock_values_;
    public:
        Lock(const std::string& space, const std::string& type);
        const std::string& Space() const{ return lock_space_; }
        LockType LockType() const{ return lock_type_; }
        void AddBlockingElement(const std::string& field, const std::string& value);
        bool IsLocking(const Lock& lock) const;
    };

    class Locks{
    private:
        std::unordered_map<std::string, Lock> locks_;
    public:
        void AddLock(const Lock& lock);
        bool IsLocking(const Locks& locks) const;
    };

    class TDeadLock{
    private:
        uint32_t t_connect_id_first_;
        uint32_t t_connect_id_second_;
        Locks locks_first_;
        Locks locks_second_;
    public:
        TDeadLock(uint32_t t_connect_id_first, uint32_t t_connect_id_second, Locks locks_first, Locks locks_second);
        uint32_t ConnectIdFirst(){ return t_connect_id_first_; }
        uint32_t ConnectIdSecond(){ return t_connect_id_second_; }
        const Locks& LocksFirst() const{ return locks_first_; }
        const Locks& LocksSecond() const{ return locks_second_; }
    };

    class TLock{
    private:
        uint32_t t_connect_id_;
        std::unordered_set<uint32_t> wait_connections_set_;
        Locks locks_;
    public:
        TLock():t_connect_id_(0){};
        TLock(uint32_t t_connect_id);
        void AddWaitConnection(uint32_t connect_id);
        void AddLock(const Lock& lock);
        bool CheckWaitConnection(uint32_t connect_id);
        const Locks& Locks() const{ return locks_; }
        const std::unordered_set<uint32_t>& WaitConnection(){ return wait_connections_set_; }
    };

    class AnalayzerLock{
    public:
        static TDeadLock ParseTDeadLock(const std::string& str);
        static TLock ParseTLock(const std::string& str);
    };

}