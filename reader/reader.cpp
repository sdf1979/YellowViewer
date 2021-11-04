#include "reader.h"
#include <filesystem>
#include <cstring>

namespace TechLog1C{

    using namespace std;

    tm GetFileTime(const wstring& file_name){
        filesystem::path path = file_name;
        wstring file = path.stem().wstring();
        tm tm = {};
        tm.tm_year = stoi(file.substr(0, 2));
        tm.tm_mon = stoi(file.substr(2, 2));
        tm.tm_mday = stoi(file.substr(4, 2));
        tm.tm_hour = stoi(file.substr(6, 2));
        return tm;
    }

    string GetProcessFromFile(const wstring& file_name){
        
        string process;

        filesystem::path path = file_name;

        if(path.has_parent_path()){
            path = path.parent_path();
            process = path.stem().string();

            if(path.has_parent_path()){
                path = path.parent_path();
                if(path.has_parent_path()){
                    path = path.parent_path();
                    process = path.stem().string().append("\\").append(process);
                }
            }
        }
        
        return process;
    }

    int FindStartPositionBuffer(char* buffer, uint64_t size){
        char* ch = buffer + size - 1;
        int pos = 0;
        for(;;){
            if(pos == 0 && *ch == '-'){
                ++pos;
            }
            else if(pos >=1 && pos <= 6){
                if(*ch >= '0' && *ch <= '9') ++pos; else pos = 0;
            }
            else if(pos == 7){
                if(*ch == '.') ++pos; else pos = 0;
            }
            else if(pos >=8 && pos <= 9){
                if(*ch >= '0' && *ch <= '9') ++pos; else pos = 0;
            }
            else if(pos == 10){
                if(*ch == ':') ++pos; else pos = 0;
            }
            else if(pos >=11 && pos <= 12){
                if(*ch >= '0' && *ch <= '9') ++pos; else pos = 0;
            }
            else if(pos == 13){
                if(*ch == '\n' || *ch == -65){
                    return (buffer + size - 1- ch)/sizeof(char);
                }
            }
            else{
                pos = 0;
            }

            if(ch == buffer){
                return -1;
            }

            --ch;
        }

        return -1;
    }

    bool operator<(const Reader& lhs, const Reader& rhs){
        return lhs.time_ < rhs.time_;
    }

    unsigned int Reader::Time(wstring file_name){
        tm file_time = GetFileTime(file_name);
        return file_time.tm_year * 1000000 + file_time.tm_mon * 10000 + file_time.tm_mday * 100 + file_time.tm_hour;
    }

    Reader::Reader(unsigned int id, wstring file_name, unsigned int size_buffer):
        id_(id),
        pos_(0),
        pos_end_(0),
        buffer_(nullptr),
        size_read_(0),
        file_name_(file_name),
        size_buffer_(size_buffer){
        tm file_time = GetFileTime(file_name);
        year_ = file_time.tm_year;
        month_ = file_time.tm_mon;
        day_ = file_time.tm_mday;
        hour_ = file_time.tm_hour;
        time_ = Reader::Time(file_name);
        process_ = GetProcessFromFile(file_name);    
        }

    void Reader::FindStartPosition(){
        fs_.seekg (0, ios::end);
        pos_end_ = fs_.tellg();
        uint64_t size = 0;
        for(;;){
            if(pos_end_ > size_buffer_) {
                pos_ = pos_end_ - size_buffer_;
            }
            else {
                pos_ = 0;
            }

            size = pos_end_ - pos_;            
            fs_.seekg( pos_, ios::beg);
            fs_.read(buffer_, size);
            
            int end_offset = FindStartPositionBuffer(buffer_, size);
            if(end_offset != -1){
                pos_ = pos_end_ - end_offset;
                break;
            }

            pos_end_ -= size;

            if(pos_ == 0){
                break;
            }
        }
    }

    uint64_t Reader::GetFileSize(){
        std::ifstream file_stream(file_name_.c_str(), ios::binary);
        bool is_open = file_stream.is_open(); 
        if(is_open){
            const auto begin = file_stream.tellg();
            file_stream.seekg (0, ios::end);
            const auto end = file_stream.tellg();
            file_stream.close();
            return end - begin;
        }
        else{
            throw L"Bad file";
        }
    }

    bool Reader::Open(){
        if (!fs_.is_open()){
            fs_.open(file_name_.c_str(), ios::binary);
            if(fs_.is_open()){
                buffer_ = new char[size_buffer_];
                fs_.seekg( pos_, ios::beg);
                fs_.read(buffer_, 3);
                if(!(buffer_[0] == (char)0xEF && buffer_[1] == (char)0xBB && buffer_[2] == (char)0xBF)){
                    fs_.close();
                    return false;
                }
                pos_ = 3;
            }
            else{
                throw L"Bad file";                
            }
        }
        return fs_.is_open();
    }

    bool Reader::Close(){
        if(buffer_){
            delete[] buffer_;
            buffer_ = nullptr;
        }
        if(fs_.is_open()){
            fs_.close();
        }
        return true;        
    }

    bool Reader::Read(){
        fs_.seekg (0, ios::end);
        pos_end_ = fs_.tellg();
        fs_.seekg(pos_, ios::beg);
        
        return pos_ != pos_end_;      
    }

    bool Reader::Read(char* ch, uint64_t start, uint64_t size){
        if (!fs_.is_open()){
            fs_.open(file_name_.c_str(), ios::binary);
            fs_.seekg(start, ios_base::beg);
            fs_.read(ch, size);
            fs_.close();
            return true;
        }
        return false;
    }

    bool Reader::Next(){
        MEASUREMENT;
        size_read_ = pos_end_ - pos_;
        size_read_ = (size_read_ > size_buffer_) ? size_buffer_ : size_read_;

        fs_.seekg( pos_, ios::beg);
        fs_.read(buffer_, size_read_);
        pos_ += size_read_;

        return size_read_;
    }

    bool Reader::IsWorkingFile(){
        return GetFileSize() > 3;
    }    

    const wstring& Reader::GetFileName(){
        return file_name_;
    }

    pair<const char*, int> Reader::GetBuffer(){
        return {buffer_, size_read_};
    }

    void Reader::ClearBuffer(){
        memset(buffer_, '\0', size_buffer_);
    }

    Reader::~Reader(){
        Close();
    }

    bool Reader::operator==(const Reader& reader) const{
        return this == &reader;
    }
}