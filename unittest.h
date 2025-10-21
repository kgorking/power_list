#pragma once

// Custom state-of-the-art unit testing functionality

#define BEGIN_TEST int main() {
#define END_TEST return 0; }

#if 1
#define UNITTEST static_assert
#else
#include <cassert>
#define xassert(e, sz) assert((e) && (sz))
#define UNITTEST xassert
#endif
