#ifndef POWER_LIST_H
#define POWER_LIST_H

#include <array>
#include <bit>
#include <cassert>

// The maximum size of the list, as 2^N
constexpr unsigned char MaxPow2Size = 58;
constexpr unsigned char RebalanceThresholdSize = 64 - MaxPow2Size;

constexpr int log_2(std::unsigned_integral auto n) {
	return std::bit_width(n) - 1;
}

namespace kg {
	template <typename T>
	class power_list {
		struct node {
			T data;
			node* next[2];
		};

		struct stepper {
			node* from;
			std::size_t target;
			std::size_t size;
		};

	public:
		constexpr ~power_list() {
			destroy_nodes();
		}

		[[nodiscard]] constexpr std::size_t size() const {
			return count;
		}

		[[nodiscard]] constexpr T front() {
			assert(!empty() && "Can not call 'front()' on an empty list");
			return head->data;
		}

		[[nodiscard]] constexpr T back() {
			assert(!empty() && "Can not call 'back()' on an empty list");
			return head->next[1] ? head->next[1]->data : head->data;
		}

		[[nodiscard]] constexpr bool empty() const {
			return nullptr == head;
		}

		constexpr void clear() {
			destroy_nodes();
			head = nullptr;
			count = 0;
			rebalance_threshold = 0;
		}

		constexpr void insert(T val) {
			node* n = new node{ val, {} };

			if (head == nullptr) { // empty
				head = n;
				head->next[1] = n;
			}
			else if (val < head->data) { // before head
				n->next[0] = head;
				n->next[1] = head->next[1];
				head = n;
			}
			else if (node* last = head->next[1]; last && (val > last->data)) { // after tail
				last->next[0] = n;
				last->next[1] = n;
				head->next[1] = n;
			}
			else { // middle
				node* it = lower_bound(val);
				n->next[0] = it->next[0];
				n->next[1] = it->next[1];
				it->next[0] = n;
				it->next[1] = n->next[1];
			}

			count += 1;
			//rebalance();
		}

		constexpr void remove(T val) {
			if (head == nullptr || val < head->data || val > head->next[1]->data)
				return;

			node* n = head, *prev = nullptr;
			while (n->next[0] && val > n->next[0]->data) {
				n = n->next[val > n->next[1]->data];
			}
			while (n->data < val) {
				prev = n;
				n = n->next[0];
			}

			if (n->data != val)
				return;

			node* next = n->next[0];
			if (prev == nullptr) { // head
				if (next != nullptr) {
					node* tail = n->next[1];
					next->next[1] = tail;
				}
				head = next;
			}
			else {
				if (next == nullptr) // tail
					head->next[1] = prev;
				prev->next[0] = next;
			}

			delete n;
			count -= 1;
			//rebalance();
		}

		constexpr void rebalance(bool force = false) {
			if (count <= 1) return;

			auto const log_n = std::bit_width(count - 1);
			if (log_n <= 1)
				return;
			bool const needs_rebalance = std::has_single_bit(count) && (log_n & ~rebalance_threshold);

			if (head != nullptr && (force || needs_rebalance)) {
				auto const count_next_pow2 = 1 << log_n;
				node* current = head;
				node* prev = head;

				// Set up the min-heap of steppers, used to rebuild the 'tree'.
				std::array<stepper, MaxPow2Size> steppers;
				int index = 0;
				for (; index < log_n; index++) {
					std::size_t const step = count_next_pow2 >> index;
					steppers[log_n - 1 - index].from = current;
					steppers[log_n - 1 - index].target = index + step;
					steppers[log_n - 1 - index].size = step;
					
					current->next[1] = current->next[0]; // reset current tree
					current = current->next[0];
				}

				// Link up the skip-nodes
				while (current ) {
					stepper* top = steppers.data();
					while (top->target == index) {
						top->target += top->size;
						top->from->next[1] = current->next[0];
						top->from = current;
						drop_front_in_heap(top, log_n);
					}

					current->next[1] = current->next[0]; // reset current tree
					prev = current;
					current = current->next[0];
					index += 1;
				}

				for (std::size_t i = 0; i < log_n; i++) {
					steppers[i].from->next[1] = steppers[i].from->next[0];
				}

				head->next[1] = prev;
				rebalance_threshold = log_n;
			}
		}

		[[nodiscard]] constexpr node* find(T const& val) const {
			if (head == nullptr || val < head->data || val > head->next[1]->data)
				return nullptr;

			node* n = head;
			while (n->data < val) {
				bool const lane = val >= n->next[1]->data;
				n = n->next[lane];
			}

			if (n->data == val)
				return n;
			else
				return nullptr;
		}

		[[nodiscard]] constexpr std::pair<int, node*> count_steps_to_find(T const& val) const {
			if (head == nullptr || val < head->data || val > head->next[1]->data)
				return std::make_pair(-1, nullptr);

			int steps = 0;
			node* n = head;
			while (n->data < val) {
				bool const lane = val >= n->next[1]->data;
				n = n->next[lane];
				steps += 1;
			}

			return (n->data == val) ? std::make_pair(steps, n) : std::make_pair(-1, nullptr);
		}

		[[nodiscard]] constexpr node* lower_bound(T const& val) const {
			if (empty())
				return nullptr;
			if (val < head->data)
				return head;

			node* n = head;
			while (n->next[0]->data < val) {
				bool const lane = val >= n->next[1]->data;
				n = n->next[lane];
			}
			return n;
		}

		constexpr bool contains(T const& val) const {
			return find(val) != nullptr;
		}

	private:
		// Drops the front stepper down the heap until the heap property is restored (same as a pop and push)
		static constexpr void drop_front_in_heap(stepper* steppers, std::size_t log_n) {
			std::size_t i = 0;
			do {
				std::size_t const max = 2 * i + (steppers[2 * i].target > steppers[2 * i + 1].target);
				if (steppers[i].target > steppers[max].target) {
					std::swap<stepper>(steppers[i], steppers[max]);
					i = max;
				}
				else {
					break;
				}
			} while (2 * i + 1 < log_n);
		}

		constexpr void destroy_nodes() {
			while (head) {
				node* next = head->next[0];
				assert(head != next && "Node points to itself");
				delete head;
				head = next;
			}
			head = nullptr;
		}

		node* head = nullptr;
		std::size_t count : MaxPow2Size = 0;
		std::size_t rebalance_threshold : RebalanceThresholdSize = ~0;
	};
}
#endif
