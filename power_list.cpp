#include <ranges>
#include <iostream>
#include "power_list.h"
#include "unittest.h"

using namespace kg;

BEGIN_TEST // Test power_list

UNITTEST(std::ranges::sized_range<power_list<int>>, "power_list must conform to sized_range concept");

UNITTEST([] {
	power_list<int> list;
	list.remove(123);
	return list.empty() && list.size() == 0 && !list.contains(0);
	}(), "Empty list");

UNITTEST([] {
	auto const iota = std::views::iota(-2, 2);
	power_list<int> list(iota);
	for (int v : iota)
		assert(list.contains(v) && "Value not found");
	return true;
	}(), "Construction from a range");

UNITTEST([] {
	auto const iota = std::views::iota(-2, 2);
	power_list<int> list(iota);
	power_list<int> list2(list);
	return list == list2;
	}(), "Copy construction");

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
	power_list<int> list(std::views::iota(-2, 2));
	list.assign_range(std::views::iota(0, 4));
	list.assign_range(std::views::iota(4, 10));
	assert(list.size() == 6 && "Invalid element count in list");
	assert(std::ranges::contains_subrange(std::views::iota(4, 10), list));
	list.assign_range(std::vector<int>{});
	return list.empty();
	}(), "Assign from a range");

UNITTEST([] {
	power_list<int> list;
	list.remove(23);
	return list.empty();
	}(), "Remove from empty");

UNITTEST([] {
	power_list<int> list(std::views::iota(0, 1));
	list.remove(0);
	return list.empty();
	}(), "Remove single value");

UNITTEST([] {
	power_list<int> list(std::views::iota(0, 8));
	list.remove(0);
	return std::ranges::contains_subrange(std::views::iota(1, 8), list);
	}(), "Remove head");

UNITTEST([] {
	power_list<int> list(std::views::iota(0, 8));
	list.remove(7);
	return std::ranges::contains_subrange(std::views::iota(0, 7), list);
	}(), "Remove tail");

UNITTEST([] {
	power_list<int> list(std::views::iota(0, 8));
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

UNITTEST([] {
	auto const iota = std::views::iota(-10, 20);
	power_list<int> list;
	for (int v : iota)
		list.insert(v);
	int sum = 0;
	for (int v : list)
		sum += v;
	return sum > 0 && list.contains(1);
	}(), "Implicit rebalance");

UNITTEST([] {
	auto const iota = std::views::iota(0, 20);
	power_list<int> list1(iota);
	power_list<int> list2(iota);
	if (list1 != list2)
		return false;

	power_list<int> list3;
	for (int val : iota)
		list3.insert(val);
	if (list1 != list3)
		return false;

	return true;
	}(), "Comparison operator");

END_TEST