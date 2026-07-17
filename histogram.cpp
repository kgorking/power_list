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
	std::map<int, int> diffs_histogram;
	std::map<int, int> skips_histogram;

	int max_steps = -1;
	int index = -1;
	for (int v : std::views::iota(0u, num_values)) {
		auto [steps, node] = list.count_steps_to_find(v);
		if (steps > max_steps) {
			max_steps = steps;
			index = v;
		}

		if (steps >= histogram.size()) {
			steps = histogram.size() - 1;
			//std::println("Error: too many steps {} at value {}", steps, v);
			//return 1;
		}

		// Count how many steps each search took
		histogram[steps] += 1;


		if (node->next[1]) {
			// Count the skip-size between a node and its skip node
			int const diff = node->next[1]->data - node->data;
			diffs_histogram[diff] += 1;

			// Count how many skip-nodes points to each value
			skips_histogram[node->next[1]->data] += 1;
		}
		else {
			diffs_histogram[0] += 1;
			//skips_histogram[0] += 1;
		}
	}

	std::println("Max steps was {} at value {}\n", max_steps, index);

	// Print the steps histogram
	std::println("steps | count");
	std::println("------+---------");
	int sum = 0;
	for (auto const& [index, val] : histogram | std::views::enumerate) {
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
	sum = 0;
	for (auto const& [index, val] : diffs_histogram) {
		sum += val;
		std::println("{:6} | {}", index, val);
	}
	std::println("-------+---------");
	std::println("total  | {}, with {} different offsets", sum, diffs_histogram.size());


	// Create a derivative(?) histogram to get number of skip counts.
	// This shows how many nodes points to a single node in the list.
	std::map<int, int> skips_deriv;
	for (auto const& [_, count] : skips_histogram) {
		skips_deriv[count] += 1;
	}

	std::println("\nHistogram of skip counts");
	std::println("skips  | count");
	std::println("-------+---------");
	for (auto const& [counts, val] : skips_deriv) {
		std::println("{:6} | {}", counts, val);
	}
	std::println("-------+---------");
	std::println("total  | {} different skip counts", skips_deriv.size());

}
