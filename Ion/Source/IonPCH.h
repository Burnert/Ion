#pragma once
#pragma warning(disable:4251)

#include <iostream>
#include <memory>
#include <functional>
#include <thread>
#include <utility>
#include <chrono>
#include <algorithm>
#include <optional>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#ifdef ION_PLATFORM_WINDOWS
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#endif

#include "Core/IonCorePCH.h"
