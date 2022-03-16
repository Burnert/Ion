#pragma once
#pragma warning(disable:4251)

#include <iostream>
#include <memory>
#include <new>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <chrono>
#include <algorithm>
#include <optional>
#include <type_traits>
#include <filesystem>
#include <random>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include <queue>
#include <stack>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#if ION_DEBUG
#include <typeinfo>
#endif

#ifdef ION_PLATFORM_WINDOWS
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#endif

#include "Core/IonCorePCH.h"
