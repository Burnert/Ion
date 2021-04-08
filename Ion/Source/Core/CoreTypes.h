#pragma once

using llong        = long long;
using ubyte        = unsigned char;
using uchar        = unsigned char;
using ushort       = unsigned short;
using uint         = unsigned int;
using ulong        = unsigned long;
using ullong       = unsigned long long;
			       
using wchar        = wchar_t;

using String       = std::string;
using WString      = std::wstring;

#ifdef UNICODE
using TString      = WString;
#else
using TString      = String;
#endif

template<typename T>
using TShared      = std::shared_ptr<T>;

template<typename T>
using TUnique      = std::unique_ptr<T>;
