#include "MiscUtility.h"
#include <Windows.h>

// stringからwstringへの変換関数
std::wstring StringToWString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	// UTF-8エンコードされたstringをUTF-16エンコードされたwstringに変換するためのサイズを取得
	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), nullptr, 0);

	// 変換後のwstringのサイズが0の場合は空のwstringを返す
	if (sizeNeeded == 0) {
		return std::wstring();
	}

	// 変換後のwstringを格納するためのバッファを確保
	std::wstring result(sizeNeeded, 0);

	// UTF-8エンコードされたstringをUTF-16エンコードされたwstringに変換する
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

// wstringからstringへの変換関数
std::string WStringToString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	// UTF-16エンコードされたwstringをUTF-8エンコードされたstringに変換するためのサイズを取得
	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);

	// 変換後のstringのサイズが0の場合は空のstringを返す
	if (sizeNeeded == 0) {
		return std::string();
	}

	// 変換後のstringを格納するためのバッファを確保
	std::string result(sizeNeeded, 0);

	// UTF-16エンコードされたwstringをUTF-8エンコードされたstringに変換する
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &result[0], sizeNeeded, nullptr, nullptr);
	return result;
}