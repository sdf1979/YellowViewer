#include "timer_perf.h"

using namespace std;

namespace TechLog1C{

    TimerPerf* TimerPerf::instance_ = nullptr;
    mutex TimerPerf::mutex_;

    TimerPerf::TimerPerf(){}

    TimerPerf* TimerPerf::GetInstance(){
        lock_guard<mutex> lock(mutex_);
        if(!instance_){
            instance_ = new TimerPerf();
        }
        return instance_;
    }

    void TimerPerf::Elapsed(string id, chrono::time_point<clock_> start){
        auto duration = clock_::now() - start;
        auto it = measurements_.insert({id, {duration, 1}});
        if(!it.second){
            it.first->second.first += duration;
            ++it.first->second.second;
        }
    }

    void TimerPerf::Print(){
        vector<pair<string, pair<chrono::nanoseconds, uint64_t>>> measurements;
        for(auto it = measurements_.begin(); it != measurements_.end(); ++it){
            measurements.push_back({it->first, {it->second.first, it->second.second}});
        }
        sort(measurements.begin(), measurements.end(), [](const pair<string, pair<chrono::nanoseconds, uint64_t>>& lhs, const pair<string, pair<chrono::nanoseconds, uint64_t>>& rhs){
            return lhs.second.first > rhs.second.first;
        });

        for(auto it = measurements.begin(); it != measurements.end(); ++it){
            wcout << Utf8ToWideChar(it->first) << " " << chrono::duration_cast<second_>(it->second.first).count() << " " << it->second.second << endl;
        }
    }


    Measurement::Measurement(string file, string func):
        timer_perf_(TimerPerf::GetInstance()),
        start_(clock_::now()){
        id_.resize(file.size() + func.size() + 1);
        char* ch = &id_[0];
        memcpy(ch, file.c_str(), file.size());
        ch += file.size();
        *ch = ':';
        ++ch;
        memcpy(ch, func.c_str(), func.size());
    }

    Measurement::~Measurement(){
        timer_perf_->Elapsed(move(id_), start_);
    }

}