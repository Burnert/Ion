#pragma once

#include "Core/CoreMacros.h"
#include "Core/CoreTypes.h"

NODISCARD constexpr int Bitflag(byte bit) noexcept { return 1 << bit; }
