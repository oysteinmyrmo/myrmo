#pragma once
#include <cstdlib>

#define MYRMO_ASSERT(x) if (x); else abort();

#define FUZZY_EPSILON 0.0001

#define MYRMO_FUZZY_ASSERT(val, correctVal) \
{ \
    MYRMO_ASSERT(val <= correctVal + FUZZY_EPSILON); \
    MYRMO_ASSERT(val >= correctVal - FUZZY_EPSILON); \
}

