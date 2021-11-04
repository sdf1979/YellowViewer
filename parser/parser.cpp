#include "parser.h"

using namespace std;

namespace TechLog1C{
    
    EventData::EventData():
        offset_start_(0),
        offset_end_(0),
        minute_(0),
        second_(0),
        msecond_(0),
        count_lines_(1),
        t_connect_id_(0)
        // duration_(0),
        // level_(0),
        // memory_(0),
        // memory_peak_(0),
        // in_bytes_(0),
        // out_bytes_(0),
        // cpu_time_(0)
        {}

    EventData::EventData(EventData&& event_data):
        offset_start_(event_data.offset_start_),
        offset_end_(event_data.offset_end_),
        minute_(event_data.minute_),
        second_(event_data.second_),
        msecond_(event_data.msecond_),
        count_lines_(event_data.count_lines_),
        // duration_(event_data.duration_),
        name_(move(event_data.name_)),
        // level_(event_data.level_),
        // process_(move(event_data.process_)),
        p_process_name_(move(event_data.p_process_name_)),
        usr_(move(event_data.usr_)),
        session_id_(move(event_data.session_id_)),
        t_connect_id_(event_data.t_connect_id_),
        wait_connections_(move(event_data.wait_connections_))
        // memory_(event_data.memory_),
        // memory_peak_(event_data.memory_peak_),
        // in_bytes_(event_data.in_bytes_),
        // out_bytes_(event_data.out_bytes_),
        // cpu_time_(event_data.cpu_time_),
        // key_temp_(move(event_data.key_temp_)),
        // value_temp_(move(event_data.value_temp_)),
        // wait_connection_(move(event_data.wait_connection_)),
        // descr_(move(event_data.descr_)),
        // i_name_(move(event_data.i_name_))
        {
        event_data.offset_start_ = 0;
        event_data.offset_end_ = 0;
        event_data.minute_ = 0;
        event_data.second_ = 0;
        event_data.msecond_ = 0;
        event_data.count_lines_ = 1;
        event_data.t_connect_id_ = 0;
        // event_data.duration_ = 0;
        // event_data.level_ = 0;
        // event_data.memory_ = 0;
        // event_data.memory_peak_ = 0;
        // event_data.in_bytes_ = 0;
        // event_data.out_bytes_ = 0;
        // event_data.cpu_time_ = 0;
    }

    EventData& EventData::operator=(EventData&& event_data){
        if (&event_data == this) return *this;
        
        offset_start_ = event_data.offset_start_;
        offset_end_ = event_data.offset_end_;
        minute_ = event_data.minute_;
        second_ = event_data.second_;
        msecond_ = event_data.msecond_;
        count_lines_= event_data.count_lines_;
        // duration_ = event_data.duration_;
        name_ = move(event_data.name_);
        // level_ = event_data.level_;
        // process_ =move(event_data.process_);
        p_process_name_ = move(event_data.p_process_name_);
        usr_ = move(event_data.usr_);
        session_id_ = move(event_data.session_id_);
        t_connect_id_ = event_data.t_connect_id_;
        wait_connections_ = move(event_data.wait_connections_);
        // memory_ = event_data.memory_;
        // memory_peak_ = event_data.memory_peak_;
        // in_bytes_ = event_data.in_bytes_;
        // out_bytes_ = event_data.out_bytes_;
        // cpu_time_ = event_data.cpu_time_;
        // wait_connection_ = move(event_data.wait_connection_);
        // descr_ = move(event_data.descr_);
        // i_name_ = move(event_data.i_name_);

        // key_temp_ = move(event_data.key_temp_);
        // value_temp_ = move(event_data.value_temp_);

        event_data.offset_start_ = 0;
        event_data.offset_end_ = 0;
        event_data.minute_ = 0;
        event_data.second_ = 0;
        event_data.msecond_ = 0;
        event_data.count_lines_ = 1;
        event_data.t_connect_id_ = 0;
        // event_data.duration_ = 0;
        // event_data.level_ = 0;
        // event_data.memory_ = 0;
        // event_data.memory_peak_ = 0;
        // event_data.in_bytes_ = 0;
        // event_data.out_bytes_ = 0;
        // event_data.cpu_time_ = 0;

        return *this;        
    }

    bool IsGuid(const char* begin, const char* end){
        int size_guid = 0;
        for(;begin != end;++begin){
            char ch = *begin;
            if((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch == '-' && (size_guid == 8 || size_guid == 13 || size_guid == 18 || size_guid == 23))){
                ++size_guid;
            }
            else{
                return false;
            }
        }
        return true;
    }

    string ReplaceGuid(string value){
        size_t size = value.size();
        if(size < 37){
            return move(value);
        }
        else{
            const char* begin = value.c_str() + size - 36;
            if(IsGuid(begin, begin + 36)){
                string new_value;
                new_value.append(value.substr(0, size - 36));
                new_value.append("_guid");
                return new_value;
            }
            else{
                return(move(value));
            }
        }        
    }

    void EventData::EndValue(){
        // if(key_temp_ == "process") process_ = move(value_temp_);
        //if(key_temp_ == "p:processName") p_process_name_ = ReplaceGuid(move(value_temp_));
        if(key_temp_ == "p:processName") p_process_name_ = move(value_temp_);
        else if(key_temp_ == "Usr") usr_ = move(value_temp_);
        else if(key_temp_ == "t:connectID"){
            t_connect_id_ = stoll(value_temp_); 
            value_temp_ = "";
        }
        else if(key_temp_ == "SessionID"){
            session_id_ = move(value_temp_);
        }
        else if(key_temp_ == "WaitConnections"){
            if(!value_temp_.empty()){
                wait_connections_ = move(value_temp_);
            }
        }
        // else if(key_temp_ == "Memory"){
        //     memory_ = stoll(value_temp_);
        //     value_temp_ = "";
        // }
        // else if(key_temp_ == "MemoryPeak"){
        //     memory_peak_ = stoll(value_temp_);
        //     value_temp_ = "";
        // }
        // else if(key_temp_ == "InBytes"){
        //     in_bytes_ = stoll(value_temp_);
        //     value_temp_ = "";
        // }
        // else if(key_temp_ == "OutBytes"){
        //     out_bytes_ = stoll(value_temp_);
        //     value_temp_ = "";
        // }
        // else if(key_temp_ == "CpuTime"){
        //     cpu_time_ = stoll(value_temp_);
        //     value_temp_ = "";
        // }
        // else if(key_temp_ == "Descr") descr_ = move(value_temp_);
        // else if(key_temp_ == "IName") i_name_ = move(value_temp_);
        else value_temp_ = "";
        key_temp_ = "";
    }
    
    // ostream& operator<< (std::ostream &out, const EventData& event_data){
    //     out << event_data.minute_ << ':' << event_data.second_ << '.' << event_data.msecond_ << '-' << event_data.duration_ << ',' << event_data.name_;
    //     return out;
    // }

    TypeChar GetTypeChar(char ch, Parser* const parser){
        if(ch == -48 || ch == -47){
            return TypeChar::Char;
        }
        else if(ch >= -128 && ch <= -65){
            return TypeChar::Char;
        }
        else if(ch >= 'a' && ch <= 'z'){
            return TypeChar::Char;
        }
        else if(ch >= 'A' && ch <= 'Z'){
            return TypeChar::Char;
        }
        else if(ch >= '0' && ch <= '9'){
            return TypeChar::Digit;
        }
        else if(ch == ':'){
            return TypeChar::Colon;
        }
        else if(ch == '.'){
            return TypeChar::Dot;
        }
        else if(ch == '-'){
            return TypeChar::Minus;
        }
        else if(ch == ','){
            if(parser->stack_quotes.empty()){
                return TypeChar::Comma;
            }
            else{
                return TypeChar::CommaInQ;
            }
        }
        else if(ch == '='){
            return TypeChar::Equal;
        }
        else if(ch == '\''){
            return TypeChar::Quote;
        }
        else if(ch == '"'){
            return TypeChar::Dquote;
        }
        else if(ch == '\n'){
            if(parser->stack_quotes.empty()){
                return TypeChar::NewLine;
            }
            else{
                return TypeChar::NewLineInQ;
            }
        }
        else{
            return TypeChar::Char;
        }
    };

    enum class State{
        Start,
        Minute,
        Second,
        Msecond,
        Duration,
        Event,
        Level,
        Key,
        Value,
        EndValue,
        EndEvent,
        Mvalue,
        Error
    };

    void Start(const char** ch, Parser* const parser){
        parser->event_data = {};
        if(**ch == '\n'){
            ++*ch;
            ++parser->offset_;
        }
        parser->event_data.offset_start_ = parser->offset_;
    }

    void Minute(const char** ch, Parser* const parser){
        parser->event_data.minute_ = parser->event_data.minute_ * 10 + (**ch - '0');
        ++*ch;
        ++parser->offset_;
    }

    void Second(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != ':'){
            parser->event_data.second_ = parser->event_data.second_ * 10 + (ch_ - '0');
        }
        ++*ch;
        ++parser->offset_;
    }

    void Msecond(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != '.'){
            parser->event_data.msecond_ = parser->event_data.msecond_ * 10 + (ch_ - '0');
        }
        ++*ch;
        ++parser->offset_;
    }

    void Duration(const char** ch, Parser* const parser){
        // char ch_ = **ch;
        // if(ch_ != '-'){
        //     parser->event_data.duration_ = parser->event_data.duration_ * 10 + (ch_ - '0');
        // }
        ++*ch;
        ++parser->offset_;
    }

    void Event(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != ','){
            parser->event_data.name_.push_back(ch_);
        }
        ++*ch;
        ++parser->offset_;
    }

    void Level(const char** ch, Parser* const parser){
        // char ch_ = **ch;
        // if(ch_ != ','){
        //     parser->event_data.level_ = parser->event_data.level_ * 10 + (ch_ - '0');
        // }
        ++*ch;
        ++parser->offset_;
    }

    void Key(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != ','){
            parser->event_data.key_temp_.push_back(ch_);
        }
        ++*ch;
        ++parser->offset_;
    }

    void Value(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ != '=' && ch_ != '\r'){
            parser->event_data.value_temp_.push_back(ch_);
        }
        ++*ch;
        ++parser->offset_;
    }

    void EndValue(const char** ch, Parser* const parser){
        parser->event_data.EndValue();
    }

    void EndEvent(const char** ch, Parser* const parser){
        parser->event_data.offset_end_ = parser->offset_;
        parser->events_.push_back(move(parser->event_data));
    }

    void Mvalue(const char** ch, Parser* const parser){
        char ch_ = **ch;
        if(ch_ == '\'' || ch_ == '"'){
            if(parser->stack_quotes.empty()){
                parser->stack_quotes.push(ch_);
            }
            else{
                if(parser->stack_quotes.top() == ch_){
                    parser->stack_quotes.pop();
                }
            }
        }
        if(ch_ == '\n') ++parser->event_data.count_lines_;
        parser->event_data.value_temp_.push_back(ch_);
        ++*ch;
        ++parser->offset_;        
    }

    void Error(const char** ch, Parser* const parser){
        //TODO DEBUG
        //wcout << L"Parser error!";
        throw "Parser error!";
    }

    State GetState(PtrFcn ptr_fcn_){
        if(ptr_fcn_ == Key) return State::Key;
        else if(ptr_fcn_ == Value) return State::Value;
        else if(ptr_fcn_ == Mvalue) return State::Mvalue;
        else if(ptr_fcn_ == EndValue) return State::EndValue;
        else if(ptr_fcn_ == Minute) return State::Minute;
        else if(ptr_fcn_ == Second) return State::Second;
        else if(ptr_fcn_ == Msecond) return State::Msecond;
        else if(ptr_fcn_ == Duration) return State::Duration;
        else if(ptr_fcn_ == Event) return State::Event;
        else if(ptr_fcn_ == Level) return State::Level;
        else if(ptr_fcn_ == EndEvent) return State::EndEvent;
        else if(ptr_fcn_ == Start) return State::Start;
        else return State::Error;
    }

PtrFcn TransitionTable[12][12] = {
//              Digit     Colon   Dot      Minus     Comma     CommaInQ Equal   Quote   Dquote  NewLine   NewLineInQ Char       
/*Start*/     { Minute,   Error,  Error,   Error,    Error,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Minute*/    { Minute,   Second, Error,   Error,    Error,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Second*/    { Second,   Error,  Msecond, Error,    Error,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Msecond*/   { Msecond,  Error,  Error,   Duration, Error,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Duration*/  { Duration, Error,  Error,   Error,    Event,    Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Event*/     { Error,    Error,  Error,   Error,    Level,    Error,   Error,  Error,  Error,  Error,    Error,     Event  },
/*Level*/     { Level,    Error,  Error,   Error,    Key,      Error,   Error,  Error,  Error,  Error,    Error,     Error  },
/*Key*/       { Key,      Key,    Key,     Key,      Key,      Error,   Value,  Error,  Error,  Error,    Error,     Key    },
/*Value*/     { Value,    Value,  Value,   Value,    EndValue, Error,   Value,  Mvalue, Mvalue, EndValue, Error,     Value  },
/*EndValue*/  { Error,    Error,  Error,   Error,    Key,      Error,   Error,  Error,  Error,  EndEvent, Error,     Error  },
/*EndEvent*/  { Error,    Error,  Error,   Error,    Error,    Error,   Error,  Error,  Error,  Start,    Error,     Error  },
/*Mvalue*/    { Mvalue,   Mvalue, Mvalue,  Mvalue,   EndValue, Mvalue,  Mvalue, Mvalue, Mvalue, EndValue, Mvalue,    Mvalue }
};

    Parser::Parser():
        ptr_fcn(Start),
        ptr_fcn_break(nullptr),
        offset_(3){}

    void Parser::Parse(const char* ch, int size){
        MEASUREMENT;

        const char* end = ch + size - 1;

        //Обрабатываем первый символ
        if(ptr_fcn == Start){
            ptr_fcn(&ch, this);
        }
        ptr_fcn = TransitionTable[(int)GetState(ptr_fcn)][(int)GetTypeChar(*ch, this)];

        //Обрабатываем от второго до предпоследнего
        for(;ch != end;){
            ptr_fcn(&ch, this);
            ptr_fcn = TransitionTable[(int)GetState(ptr_fcn)][(int)GetTypeChar(*ch, this)];
        }

        //Обрабатываем крайний
        ++end;
        for(;;){
            ptr_fcn(&ch, this);
            if(ch == end){
                break;
            }
            ptr_fcn = TransitionTable[(int)GetState(ptr_fcn)][(int)GetTypeChar(*ch, this)];
        }
    };

    void Parser::AddEvent(EventData&& event_data){
        events_.push_back(move(event_data));
    }

    std::vector<EventData>&& Parser::MoveEvents(){
        return move(events_);
    }

    void Parser::Clear(){
        events_.resize(0);
    }

    Lock::Lock(const string& space, const string& type):
        lock_space_(space){
        if(type == "Shared")
            lock_type_ = LockType::SHARED;
        else if(type == "Exclusive")
            lock_type_ = LockType::EXCLUSIVE;
    }

    void Lock::AddBlockingElement(const string& field, const string& value){
        lock_values_[field].insert(value);
    }

    bool Lock::IsLocking(const Lock& lock) const{
        if(lock_space_ != lock.lock_space_) return false;
        if(lock_type_ == LockType::SHARED && lock.lock_type_ == LockType::SHARED) return false;
        
        unordered_set<string> fields;
        for(auto it = lock_values_.begin(); it != lock_values_.end(); ++it){
            fields.insert(it->first);
        }
        for(auto it = lock.lock_values_.begin(); it != lock.lock_values_.end(); ++it){
            fields.insert(it->first);
        }

        for(auto it_field = fields.begin(); it_field != fields.end(); ++it_field){
            auto it_field_lhs = lock_values_.find(*it_field);
            auto it_field_rhs = lock.lock_values_.find(*it_field);
            if(it_field_lhs != lock_values_.end() && it_field_rhs != lock.lock_values_.end()){
                bool value_is_locking = true;
                if(it_field_lhs->second.size() && it_field_rhs->second.size()){
                    value_is_locking = false;
                    for(auto it_value_lhs = it_field_lhs->second.begin(); it_value_lhs != it_field_lhs->second.end(); ++it_value_lhs){
                        if(it_field_rhs->second.find(*it_value_lhs) != it_field_rhs->second.end()){
                            value_is_locking = true;
                            break;
                        }
                    }
                    if(!value_is_locking) return false;
                }
            }
        }
        return true;
    }

    void Locks::AddLock(const Lock& lock){
        locks_.insert({lock.Space(), lock});
    }

    bool Locks::IsLocking(const Locks& locks) const{
        for(auto it_lhs_lock = locks_.begin(); it_lhs_lock != locks_.end(); ++it_lhs_lock){
            auto it_rhs_lock = locks.locks_.find(it_lhs_lock->first);
            if(it_rhs_lock != locks.locks_.end()){
                if(it_lhs_lock->second.IsLocking(it_rhs_lock->second)) return true;
            }
        }
        return false;        
    }

    TDeadLock::TDeadLock(uint32_t t_connect_id_first, uint32_t t_connect_id_second, Locks locks_first, Locks locks_second):
        t_connect_id_first_(t_connect_id_first),
        t_connect_id_second_(t_connect_id_second),
        locks_first_(locks_first),
        locks_second_(locks_second){}

    TLock::TLock(uint32_t t_connect_id):
        t_connect_id_(t_connect_id){}

    void TLock::AddWaitConnection(uint32_t connect_id){
        wait_connections_set_.insert(connect_id);
    }

    void TLock::AddLock(const Lock& lock){
        locks_.AddLock(lock);
    }

    bool TLock::CheckWaitConnection(uint32_t connect_id){
        return wait_connections_set_.find(connect_id) != wait_connections_set_.end();
    }

    // pair<string_view, string_view> split(const string_view& sv, const char* splitter){
    //     auto pos = sv.find(splitter);
    //     return {sv.substr(0, pos), sv.substr(pos + 1)};
    // }

    vector<string_view> split(string_view sv, const char* splitter){
        vector<string_view> result;
        for(;;){
            auto pos = sv.find(splitter);
            if(pos != string_view::npos){
                result.push_back(sv.substr(0, pos));
                sv.remove_prefix(pos + 1);
            }else{
                result.push_back(sv);
                break;
            }
        }
        return result;
    }

    string_view ValueByKey(string_view sv, const string& key){
        auto pos = sv.find(key);
        sv.remove_prefix(pos + key.size());
        string find = ",";
        if(sv.substr(0, 1) == "'"){
            find = "'";
            sv.remove_prefix(1);
        }
        pos = sv.find(find);
        if(pos != string_view::npos){
            sv.remove_suffix(sv.size() - pos);
        }
        return sv;
    }

    TDeadLock AnalayzerLock::ParseTDeadLock(const string& str){
        
        string_view deadlock_info = ValueByKey(str, "DeadlockConnectionIntersections=");
        
        //Получаем данные по блокировкам первого и второго соединения
        auto pos = deadlock_info.find(",");
        string lock_first_str(deadlock_info.substr(0, pos));
        deadlock_info.remove_prefix(pos + 1);
        string lock_second_str(deadlock_info);

        string delimiters_space = " ";
        string delimiters_equal = "=";

        //Получаем объект блокировки первого соединения
        uint32_t session_id_first = strtoul(strtok(&lock_first_str[0], delimiters_space.c_str()), nullptr, 0);
        strtok(nullptr, delimiters_space.c_str());
        char* lock_space = strtok(nullptr, delimiters_space.c_str());
        char* lock_type = strtok(nullptr, delimiters_space.c_str());
        char* lock_str = strtok(nullptr, delimiters_space.c_str());
        char* lock_field = strtok(lock_str, delimiters_equal.c_str());
        char* lock_value = strtok(nullptr, delimiters_equal.c_str());
        Lock lock_first(lock_space, lock_type);
        lock_first.AddBlockingElement(lock_field, lock_value);
        Locks locks_first;
        locks_first.AddLock(lock_first);
        
        //Получаем объект блокировки второго соединения
        uint32_t session_id_second = strtoul(strtok(&lock_second_str[0], delimiters_space.c_str()), nullptr, 0);
        strtok(nullptr, delimiters_space.c_str());
        lock_space = strtok(nullptr, delimiters_space.c_str());
        lock_type = strtok(nullptr, delimiters_space.c_str());
        lock_str = strtok(nullptr, delimiters_space.c_str());
        lock_field = strtok(lock_str, delimiters_equal.c_str());
        lock_value = strtok(nullptr, delimiters_equal.c_str());
        Lock lock_second(lock_space, lock_type);
        lock_second.AddBlockingElement(lock_field, lock_value);
        Locks locks_second;
        locks_second.AddLock(lock_second);
 
        return {session_id_first, session_id_second, locks_first, locks_second};
    }

    TLock AnalayzerLock::ParseTLock(const std::string& str){
        
        // if(str.find("alreadylocked") != string::npos){
        //     int a = 1;
        // }

        // if(str.find("escalating") != string::npos){
        //     int a = 1;
        // }

        string_view connect_id_sv = ValueByKey(str, "t:connectID="); 
        uint32_t connect_id = strtoul(string(connect_id_sv).c_str(), nullptr, 0);

        TLock tlock(strtoul(string(connect_id_sv).c_str(), nullptr, 0));

        string find = ",";
        string_view wait_connections_sv = ValueByKey(str, "WaitConnections=");
        for(;;){
            auto pos = wait_connections_sv.find(find);
            if(pos != string_view::npos){
                tlock.AddWaitConnection(strtoul(string(wait_connections_sv.substr(0, pos)).c_str(), nullptr, 0));
                wait_connections_sv.remove_prefix(pos + find.size());
            }
            else{
                tlock.AddWaitConnection(strtoul(string(wait_connections_sv).c_str(), nullptr, 0));
                break;               
            }
        }

        string_view locks_sv = ValueByKey(str, "Locks=");
        for(;;){
            find = " ";
            auto pos = locks_sv.find(find);
            string lock_space(locks_sv.substr(0, pos));
            locks_sv.remove_prefix(pos + find.size());
            pos = locks_sv.find(find);
            string lock_type(locks_sv.substr(0, pos));
            locks_sv.remove_prefix(pos + find.size());

            Lock lock(lock_space, lock_type);

            find = ",";
            for(;;){
                pos = locks_sv.find(find);
                if(pos != string_view::npos){
                    string_view lock_fields_value_sv = locks_sv.substr(0, pos);
                    vector<string_view> lock_fields_value = split(lock_fields_value_sv, " ");
                    for(auto it_field_value = lock_fields_value.begin(); it_field_value != lock_fields_value.end(); ++it_field_value){ 
                        vector<string_view> field_value = split(*it_field_value, "=");
                        if(field_value[1] != "Undefined"){
                            lock.AddBlockingElement(string(field_value[0]), string(field_value[1]));
                        }
                    }
                    locks_sv.remove_prefix(pos + find.size());
                    if(locks_sv.substr(0, 1) != " ") break;
                    locks_sv.remove_prefix(1);
                }
                else{
                    string_view lock_fields_value_sv = locks_sv.substr(0, pos);
                    vector<string_view> lock_fields_value = split(lock_fields_value_sv, " ");
                    for(auto it_field_value = lock_fields_value.begin(); it_field_value != lock_fields_value.end(); ++it_field_value){ 
                        vector<string_view> field_value = split(*it_field_value, "=");
                        if(field_value[1] != "Undefined"){
                            lock.AddBlockingElement(string(field_value[0]), string(field_value[1]));
                        }
                    }
                    break;
                } 
            }
            tlock.AddLock(lock);
            if(pos == string_view::npos) break;
        }

        return tlock;
    }

}