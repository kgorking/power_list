#include <ranges>
#include "power_list.h"
#include "unittest.h"

using namespace kg;

int main() {
	UNITTEST(std::ranges::sized_range<power_list<int>>, "power_list must conform to sized_range concept");

	UNITTEST([] {
		constexpr std::size_t elems_to_alloc = 123;
		scatter_allocator<int> alloc;
		std::size_t total_alloc = 0;
		alloc.allocate_with_callback(elems_to_alloc, [&](std::span<int> s) {
			total_alloc += s.size();
			});
		return elems_to_alloc == total_alloc;
		}(), "allocates correctly");

	UNITTEST([] {
		scatter_allocator<int> alloc;
		std::vector<std::span<int>> r = alloc.allocate(10);
		auto const subspan = r[0].subspan(3, 4);
		alloc.deallocate(subspan);
		return true;
		}(), "frees correctly");

	UNITTEST(([] {
		scatter_allocator<int, 16> alloc;
		auto vec = alloc.allocate(10);
		alloc.deallocate(vec[0].subspan(2, 2));
		alloc.deallocate(vec[0].subspan(4, 2));

		// Fills in the two holes (2+2), the rest of the first pool (6),
		// and remaining in new second pool (10)
		int count = 0;
		std::size_t sizes[] = { 2, 2, 6, 10 };
		alloc.allocate_with_callback(20, [&](auto span) {
			assert(sizes[count] == span.size() && "unexpected span size");
			count += 1;
			});
		return (count == 4);
		}()), "scatters correctly");

	UNITTEST([] {
		constexpr std::size_t elems_to_alloc = 12;
		scatter_allocator<int> alloc;
		std::span<int> span;
		alloc.allocate_with_callback(elems_to_alloc, [&](std::span<int> s) {
			span = s;
			});
		for (int& i : span) {
			std::construct_at(&i);
			std::destroy_at(&i);
		}
		alloc.deallocate(span);
		return true;
		}(), "works with construction/destruction");

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
		list.assign_range(std::views::iota(4, 8));
		assert(list.size() == 4 && "Invalid element count in list");
		for (int v : std::views::iota(4, 8))
			assert(list.contains(v));
		return true;
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
		}(), "Remove one");

	UNITTEST([] {
		power_list<int> list(std::views::iota(0, 8));
		list.remove(0);
		for (int v : std::views::iota(1, 8))
			if (!list.contains(v))
				return false;
		return 7 == list.size();
		}(), "Remove head");

	UNITTEST([] {
		power_list<int> list(std::views::iota(0, 8));
		list.remove(7);
		for (int v : std::views::iota(0, 7))
			assert(list.contains(v));
		;
		return 7 == list.size();
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

	return 0;
}
