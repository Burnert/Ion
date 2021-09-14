#pragma once

using llong        = int64_t;
using ubyte        = uint8_t;
using uchar        = uint8_t;
using ushort       = uint16_t;
using uint         = uint32_t;
using ulong        = unsigned long;
using ullong       = uint64_t;
			       
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
