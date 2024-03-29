#pragma once

#include <stdint.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <forward_list>
#include <deque>
#include <queue>
#include <stack>
#include <functional>
#include <optional>
#include <variant>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

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
#ifdef UNICODE
using tchar        = wchar_t;
#else
using tchar        = char;
#endif

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

// Use the string literals globally
using namespace std::string_literals;

// Memory

using std::align_val_t;

// Data structures

// @TODO: Implement simplified versions of these in the future, also the pointers and String above ^^

template<typename T>
using THash = std::hash<T>;

template<typename T, typename Hasher = std::hash<T>, typename Allocator = std::allocator<T>>
using THashSet = std::unordered_set<T, Hasher, std::equal_to<T>, Allocator>;

template<typename T, typename U, typename Hasher = std::hash<T>, typename Allocator = std::allocator<std::pair<const T, U>>>
using THashMap = std::unordered_map<T, U, Hasher, std::equal_to<T>, Allocator>;

template<typename T, typename Allocator = std::allocator<T>>
using TArray = std::vector<T, Allocator>;

template<typename T, typename Allocator = std::allocator<T>>
using TLinkedList = std::forward_list<T, Allocator>;

template<typename T, typename Allocator = std::allocator<T>>
using TDoubleLinkedList = std::list<T, Allocator>;

template<typename T, typename Allocator = std::allocator<T>>
using TDeque = std::deque<T, Allocator>;

template<typename T, size_t Size>
using TFixedArray = std::array<T, Size>;

template<typename T, typename Container = TDeque<T>>
using TQueue = std::queue<T, Container>;

template<typename T, typename Container = TDeque<T>>
using TStack = std::stack<T, Container>;

template<typename T, typename Container = TArray<T>, typename Compare = std::less<T>>
using TPriorityQueue = std::priority_queue<T, Container, Compare>;

template<typename T>
using TFunction = std::function<T>;

template<typename T>
using TOptional = std::optional<T>;
constexpr inline std::nullopt_t NullOpt = std::nullopt;

template<typename... Types>
using TVariant = std::variant<Types...>;

template<typename T, typename V>
NODISCARD constexpr T VariantCast(V& variant)
{
	return std::get<T>(variant);
}

template<typename T, typename V>
NODISCARD constexpr T VariantCast(V&& variant)
{
	return std::get<T>(Move(variant));
}

template<typename T, typename V>
NODISCARD constexpr T VariantCast(const V& variant)
{
	return std::get<T>(variant);
}

template<typename T, typename V>
NODISCARD constexpr T VariantCast(const V&& variant)
{
	return std::get<T>(Move(variant)); 
}

// Thread

using Thread = std::thread;
using Mutex = std::mutex;
using ConditionVariable = std::condition_variable;
using LockGuard = std::lock_guard<Mutex>;
using ScopedLock = std::scoped_lock<Mutex>;
using UniqueLock = std::unique_lock<Mutex>;
namespace LockProp
{
	inline constexpr std::defer_lock_t DeferLock = std::defer_lock;
	inline constexpr std::try_to_lock_t TryToLock = std::try_to_lock;
}

template<typename T>
using TAtomic = std::atomic<T>;
