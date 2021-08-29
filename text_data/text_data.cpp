#include "text_data.h"
#include <algorithm>

using namespace std;

namespace TechLog1C{

    Time::Time():
        time_(0){}

    Time::Time(std::uint64_t time):
        time_(time){}

    Time::Time(std::uint8_t year, std::uint8_t month, std::uint8_t day, std::uint8_t hour, std::uint8_t minute, std::uint8_t second, std::uint32_t mseconds)
    {
        time_ = year * 10000000000000000ULL
        + month * 100000000000000ULL
        + day * 1000000000000ULL
        + hour * 10000000000ULL
        + minute * 100000000ULL
        + second * 1000000ULL
        + mseconds;
    }

    bool operator<(const Time& lhs, const Time& rhs){
        return lhs.time_ < rhs.time_;
    }

    bool operator==(const Time& lhs, const Time& rhs){
        return lhs.time_ == rhs.time_;
    }

    EventOut::EventOut():
        reader_(nullptr),
        start_(0),
        length_(0),
        count_lines_(0),
        count_lines_agr_(0),
        id_(0){}
    
    EventOut::EventOut(const EventOut& event_out):
        reader_(event_out.reader_),
        time_(event_out.time_),
        start_(event_out.start_),
        length_(event_out.length_),
        count_lines_(event_out.count_lines_),
        count_lines_agr_(event_out.count_lines_agr_),
        process_(event_out.process_),
        id_(event_out.id_){}

    EventOut::EventOut(EventOut&& event_out):
        reader_(event_out.reader_),
        time_(event_out.time_),
        start_(event_out.start_),
        length_(event_out.length_),
        count_lines_(event_out.count_lines_),
        count_lines_agr_(event_out.count_lines_agr_),
        process_(move(event_out.process_)),
        id_(event_out.id_){}
    
    EventOut::EventOut(Reader* reader, uint64_t start, uint32_t length):
        reader_(reader),
        time_(0),
        start_(start),
        length_(length),
        count_lines_(0),
        count_lines_agr_(0),
        process_(""),
        id_(0){}

    EventOut::EventOut(Reader* reader, Time time, uint64_t start, uint32_t length, uint32_t count_lines, uint32_t count_lines_agr, std::string process, uint32_t id):
        reader_(reader),
        time_(time),
        start_(start),
        length_(length),
        count_lines_(count_lines),
        count_lines_agr_(count_lines_agr),
        process_(process),
        id_(id){}

    string EventOut::AsString(){
        if(!reader_) return "";

        string str(length_ + 12, '0');

        string part_date = to_string(time_.Day());
        part_date.copy(&str[2] - part_date.size(), part_date.size(), 0);
        str[2] = '.';

        part_date = to_string(time_.Month());
        part_date.copy(&str[5] - part_date.size(), part_date.size(), 0);
        str[5] = '.'; 

        part_date = to_string(time_.Year());
        part_date.copy(&str[8] - part_date.size(), part_date.size(), 0);
        str[8] = ' ';

        part_date = to_string(time_.Hour());
        part_date.copy(&str[11] - part_date.size(), part_date.size(), 0);
        str[11] = ':';       

        reader_->Read(&str[12], start_, str.size() - 12);

        return str;
    }

    wstring EventOut::AsWString(){
        if(!reader_) return L"";
        return Utf8ToWideChar(AsString());
    }

    vector<wstring> EventOut::AsWStrings(){
        vector<wstring> strings;
        if(count_lines_ == 1){
            strings.push_back(AsWString());
        }
        else{
            wstring wstr = AsWString();
            wstring_view wstrv(wstr);
            auto pos = wstrv.find(L'\n');
            while(pos != wstring_view::npos){
                strings.push_back(wstring(&wstrv[0], pos));
                wstrv.remove_prefix(pos + 1);
                pos = wstrv.find(L'\n');
            }
            strings.push_back(wstring(&wstrv[0], wstrv.size()));
        }
        return strings;
    }

    bool operator==(const EventOut& lhs, const EventOut& rhs){
        return lhs.reader_ == rhs.reader_ && lhs.time_ == rhs.time_ && lhs.start_ == rhs.start_ && lhs.length_ == rhs.length_;        
    }

    bool operator!=(const EventOut& lhs, const EventOut& rhs){
        return !(lhs == rhs);        
    }

}