#pragma once

// Custom state-of-the-art unittesting functionality

#if 1
#define UNITTEST static_assert
#else
#include <cassert>
#define xassert(e, sz) assert((e) && (sz))
#define UNITTEST xassert
#endif
