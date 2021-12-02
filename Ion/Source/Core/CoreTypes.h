#pragma once

#include <stdint.h>

// Legacy types

using llong        = int64_t;
using ubyte        = uint8_t;
using uchar        = uint8_t;
using ushort       = uint16_t;
using uint         = uint32_t;
using ulong        = unsigned long;
using ullong       = uint64_t;

// New types

using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;



using wchar        = wchar_t;

using String       = std::string;
using WString      = std::wstring;

#ifdef UNICODE
using TString      = WString;
#else
using TString      = String;
#endif

using StringView   = std::string_view;
using WStringView  = std::wstring_view;

#ifdef UNICODE
using TStringView  = WStringView;
#else
using TStringView  = StringView;
#endif

template<typename T>
using TShared      = std::shared_ptr<T>;

template<typename T>
using TUnique      = std::unique_ptr<T>;

template<typename T>
using TWeak        = std::weak_ptr<T>;

// Data structures

// @TODO: Implement simplified versions of these in the future, also the pointers and String above ^^

template<typename T, typename Hasher = std::hash<T>, typename Allocator = std::allocator<T>>
using THashSet     = std::unordered_set<T, Hasher, std::equal_to<T>, Allocator>;

template<typename T, typename U, typename Hasher = std::hash<T>, typename Allocator = std::allocator<std::pair<const T, U>>>
using THashMap     = std::unordered_map<T, U, Hasher, std::equal_to<T>, Allocator>;

template<typename T, typename Allocator = std::allocator<T>>
using TArray       = std::vector<T, Allocator>;

template<typename T, size_t Size>
using TFixedArray  = std::array<T, Size>;

template<typename T, typename Container = std::deque<T>>
using TQueue       = std::queue<T, Container>;

template<typename T>
using TFunction    = std::function<T>;

// Thread

using Thread = std::thread;
using Mutex = std::mutex;
using ConditionVariable = std::condition_variable;
using LockGuard = std::lock_guard<Mutex>;
using ScopedLock = std::scoped_lock<Mutex>;
using UniqueLock = std::unique_lock<Mutex>;

template<typename T>
using TAtomic = std::atomic<T>;

// Debug

#if ION_DEBUG
#define TypeInfo(name) std::type_info name
#else
#define TypeInfo(name)
#endif
