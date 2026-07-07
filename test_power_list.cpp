#include <ranges>
#include <print>
#include "power_list.h"
#include "unittest.h"

using namespace kg;

BEGIN_TEST // Test power_list

UNITTEST([] {
	power_list<int> list;
	list.remove(123);
	return list.empty() && !list.contains(0);
	}(), "Empty list");

UNITTEST([] {
	power_list<int> list;
	list.insert(23);
	return list.contains(23);
	}(), "Insert empty");

UNITTEST([] {
	power_list<int> list;
	list.insert(23);
	list.insert(22);
	return list.contains(23);
	}(), "Insert before head");

UNITTEST([] {
	power_list<int> list;
	list.insert(23);
	list.insert(24);
	return list.contains(23);
	}(), "Insert after tail");

UNITTEST([] {
	power_list<int> list;
	list.insert(22);
	list.insert(24);

	list.insert(23);
	return list.contains(23);
	}(), "Insert in middle");

UNITTEST([] {
	power_list<int> list;
	list.insert(23);
	list.remove(23);
	list.insert(24);
	return !list.contains(23) && list.contains(24);
	}(), "Insert/Remove/Insert");

UNITTEST([] {
	power_list<int> list;
	list.remove(23);
	return list.empty();
	}(), "Remove from empty");

UNITTEST([] {
	power_list<int> list;
	list.insert(0);
	list.remove(0);
	return list.empty();
	}(), "Remove single value");

UNITTEST([] {
	power_list<int> list;
	for (int i : std::views::iota(0, 8))
		list.insert(i);
	list.remove(0);
	return list.front() == 1 && list.back() == 7;
	}(), "Remove head");

UNITTEST([] {
	power_list<int> list;
	for (int i : std::views::iota(0, 8))
		list.insert(i);
	list.remove(7);
	return list.front() == 0 && list.back() == 6;
	}(), "Remove tail");

UNITTEST([] {
	power_list<int> list;
	for (int i : std::views::iota(0, 8))
		list.insert(i);
	for (int v : std::views::iota(1, 7))
		list.remove(v);
	int items = 0;
	for (int v : std::views::iota(0, 8))
		items += list.contains(v);
	assert(items == 2);
	return 2 == list.size();
	}(), "Remove middle");

UNITTEST([] {
	auto const iota = std::views::iota(-20, 20);
	power_list<int> list;
	for (int v : iota)
		list.insert(v);
	list.rebalance();
	return list.contains(1);
	}(), "Explicit rebalance");

END_TEST
