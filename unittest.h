#pragma once

// Custom state-of-the-art unit testing functionality

#if 1
#define UNITTEST static_assert
#define BEGIN_TEST int main() {}
#define END_TEST 
#else
#include <cassert>
#define xassert(e, sz) assert((e) && (sz))
#define UNITTEST xassert
#define BEGIN_TEST int main() {
#define END_TEST return 0; }
#endif
