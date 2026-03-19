#ifndef POWER_LIST_H
#define POWER_LIST_H

#include <cassert>
#include <bit>

namespace kg {
	template <typename T>
	class power_list {
		struct node {
			node* next[2];
			T data;
		};

	public:
		constexpr power_list() = default;
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
			node* n = new node{ {nullptr, nullptr}, val };

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
				iterator it = lower_bound(val);
				it.prev->next[0] = n;
				n->next[0] = it.curr;
				n->next[1] = it.curr->next[1];
			}

			count += 1;
			rebalance();
		}

		constexpr void remove(T val) {
			if (head == nullptr || val < head->data || val > head->next[1]->data)
				return;

			node* n = head, *prev = nullptr;
			while (n->next[0] && val > n->next[0]->data) {
				n = n->next[val > n->next[1]->data];
			}
			while (n->data < val) {
				// The only node in the list that can have 'next[0] == nullptr' is
				// the last node in the list. It would have been reached in the above loop.
				assert(n->next[0] != nullptr && "This should not be possible, by design");
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
			rebalance();
		}

		constexpr void rebalance() {
			bool const needs_rebalance = std::has_single_bit(count) && (count & rebalance_threshold) != 0;
			if (head && needs_rebalance) {
				std::size_t const log_n{std::bit_width(count - 1)};
				std::size_t index{ 0 };
				auto steppers = new stepper[log_n];

				node* current = head;
				for (std::size_t i = 0, step = count; current != nullptr && i < log_n; i++, step >>= 1) {
					steppers[log_n - 1 - i].target = i + step;
					steppers[log_n - 1 - i].size = step;
					steppers[log_n - 1 - i].from = current;

					current = current->next[0];
				}
				
				while (nullptr != current->next[0]) {
					while (steppers->target == index) {
						steppers->from->next[1] = current->next[0];
						steppers->from = current;
						steppers->target += steppers->size;
						drop_front_in_heap(steppers, log_n);
					}

					current = current->next[0];
					index += 1;
				}

				for (std::size_t i = 0; i < log_n; i++)
					steppers[i].from->next[1] = current;

				delete[] steppers;
				rebalance_threshold = (log_n << 1) | (log_n >> 1);
			}
		}

		[[nodiscard]] constexpr node* find(T const& val) const {
			if (head == nullptr || val < head->data || val > head->next[1]->data)
				return nullptr;

			node* n = head;
			while (n->next[0] && val > n->next[0]->data) {
				n = n->next[val > n->next[1]->data];
			}
			while (n->data < val) {
				// The only node in the list that can have 'next[0] == nullptr' is
				// the last node in the list. It would have been reached in the above loop.
				assert(n->next[0] != nullptr && "This should not be possible, by design");

				n = n->next[0];
			}

			if (n->data == val)
				return n;
			else
				return nullptr;
		}

		[[nodiscard]] constexpr node* lower_bound(T const& val) const {
			if (empty())
				return nullptr;
			if (val < head->data)
				return head;

			node* curr = head;
			while (val > curr->data) {
				curr = curr->next[val > curr->next[1]->data];
			}
			return curr;
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
			node* n = head;
			head = nullptr;
			while (n) {
				node* next = n->next[0];
				assert(n != next && "Node points to itself");
				std::destroy_at(n);
				n = next;
			}
		}

		node* head = nullptr;
		std::size_t count : 56 = 0;
		std::size_t rebalance_threshold : 8 = 0; // 8 bits -> log count +- 1  ?
	};
}
#endif
