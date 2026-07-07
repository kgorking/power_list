#include "power_list.h"
#include "unittest.h"
#include <map>
#include <print>
#include <ranges>

using namespace kg;

int main() {
	constexpr unsigned num_values = 1'000'000;

	// Create the ordered list of values
	power_list<int> list;
	for (int v : std::views::iota(0u, num_values))
		list.insert(v);
	list.rebalance(true);

	// Create a histogram of the number of steps required to find a value in the list
	std::array<int, 64> histogram{};
	std::map<int, int> step_histogram;
	int max_steps = -1;
	int index = -1;
	for (int v : std::views::iota(0u, num_values)) {
		auto const [steps, node] = list.count_steps_to_find(v);
		if (steps > max_steps) {
			max_steps = steps;
			index = v;
		}

		if (steps >= histogram.size()) {
			std::println("Error: too many steps {} at value {}", steps, v);
			return 1;
		}

		histogram[steps] += 1;
		if (node->next[1]) {
			int const diff = node->next[1]->data - node->data;
			step_histogram[diff] += 1;
		}
		else {
			step_histogram[0] += 1;
		}
	}

	std::println("Max steps was {} at value {}\n", max_steps, index);

	// Print the steps histogram
	std::println("steps | count");
	std::println("------+---------");
	int sum = 0;
	for (auto [index, val] : histogram | std::views::enumerate) {
		if (val == 0)
			break;

		sum += index * val;
		std::println("{:5} | {}", index, val);
	}
	std::println("------+---------");
	std::println("total | {} steps", sum);


	std::println("\nHistogram of step difference between the current- and its fast-forward node");
	std::println("diffs  | count");
	std::println("-------+---------");
	for (auto const [index, val] : step_histogram) {
		std::println("{:6} | {}", index, val);
	}
	std::println("-------+---------");
	std::println("total  | {} different offsets", step_histogram.size());
}
