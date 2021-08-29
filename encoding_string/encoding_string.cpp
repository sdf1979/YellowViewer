#include "encoding_string.h"

std::wstring Utf8ToWideChar(const std::string& str){
    int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
    std::wstring wstr(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], count);
    return wstr;        
}

std::string WideCharToUtf8(const std::wstring& wstr){
    int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
    std::string str(count, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
    return str;
}

struct number_separator : std::numpunct<char> {
    char do_thousands_sep() const override { return ' '; }
    std::string do_grouping () const override { return "\003"; }
};

std::string Format(double value, int precision){
    std::stringstream ss;
    ss.imbue(std::locale(ss.getloc(), new number_separator));
    ss << std::fixed << std::setprecision(precision) << value;
    std::string str = ss.str();
    return str;    
}