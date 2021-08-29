#pragma once

#include <string>
#include <windows.h>
#include <locale>
#include <sstream>
#include <iomanip>

std::wstring Utf8ToWideChar(const std::string& str);
std::string WideCharToUtf8(const std::wstring& wstr);
std::string Format(double value, int precision);