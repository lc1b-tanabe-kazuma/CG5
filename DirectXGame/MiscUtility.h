#pragma once
#include <string>

// stringからwstringへの変換関数
std::wstring StringToWString(const std::string& str);

// wstringからstringへの変換関数
std::string WStringToString(const std::wstring& wstr);