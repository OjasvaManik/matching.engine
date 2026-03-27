#pragma once
#include "Types.h"
#include <limits>

struct Constants {
    static constexpr Price INVALID_PRICE = std::numeric_limits<Price>::quiet_NaN();
};
