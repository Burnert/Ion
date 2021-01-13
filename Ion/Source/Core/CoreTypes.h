#pragma once

using llong        = long long;
using byte         = unsigned char;
using uchar        = unsigned char;
using ushort       = unsigned short;
using uint         = unsigned int;
using ulong        = unsigned long;
using ullong       = unsigned long long;
			       
using wchar        = wchar_t;
			       
using CStr         = char*;
using WCStr        = wchar*;

template<typename T>
using Shared       = std::shared_ptr<T>;

template<typename T>
using Unique       = std::unique_ptr<T>;
