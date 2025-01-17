#ifndef SCATTER_ALLOCATOR_H
#define SCATTER_ALLOCATOR_H

#include <cassert>
#include <span>
#include <memory>
#include <vector>

namespace kg {
	template <typename Fn, typename T>
	concept callback_takes_a_span = std::invocable<Fn, std::span<T>>;

	// 'Scatter Allocator'
	// 
	// * A single allocation can result in many addresses being returned, as the
	//   allocator fills in holes in the internal pools of memory.
	// * No object construction/destruction happens.
	// * It is not thread safe.
	// * Deallocated memory is reused before new memory is taken from pools.
	//   This way old pools will be filled with new data before newer pools are tapped.
	//   Filling it 'from the back' like this should keep fragmentation down.
	template <typename T, std::size_t DefaultStartingSize = 16>
	struct scatter_allocator {
		static_assert(DefaultStartingSize > 0);

		constexpr scatter_allocator() noexcept = default;
		constexpr scatter_allocator(scatter_allocator const&) noexcept = delete;
		constexpr scatter_allocator(scatter_allocator&& other) noexcept {
			pools.swap(other.pools);
			free_list.swap(other.free_list);
		}
		constexpr scatter_allocator& operator=(scatter_allocator const&) = delete;
		constexpr scatter_allocator& operator=(scatter_allocator&& other) noexcept {
			pools.swap(other.pools);
			free_list.swap(other.free_list);
			return *this;
		}

		constexpr std::vector<std::span<T>> allocate(std::size_t const count) {
			std::vector<std::span<T>> r;
			allocate_with_callback(count, [&r](std::span<T> s) {
				r.push_back(s);
				});
			return r;
		}

		constexpr T* allocate_one() {
			T* t{};
			allocate_with_callback(1, [&t](std::span<T> s) {
				assert(t == nullptr);
				assert(s.size() == 1);
				t = s.data();
				});
			return t;
		}

		constexpr void allocate_with_callback(std::size_t const count, callback_takes_a_span<T> auto&& alloc_callback) {
			std::size_t remaining_count = count;

			// Take space from free list
			std::unique_ptr<free_block>* ptr_free = &free_list;
			while (*ptr_free) {
				free_block* ptr = ptr_free->get();
				std::size_t const min_space = std::min(remaining_count, ptr->span.size());
				if (min_space == 0) {
					ptr_free = &ptr->next;
					continue;
				}

				std::span<T> span = ptr->span.subspan(0, min_space);
				alloc_callback(span);

				remaining_count -= min_space;
				if (remaining_count == 0)
					return;

				if (min_space == ptr->span.size()) {
					auto next = std::move(ptr->next);
					*ptr_free = std::move(next);
				}
				else {
					ptr->span = ptr->span.subspan(min_space + 1);
					ptr_free = &ptr->next;
				}
			}

			// Take space from pools
			pool* ptr_pool = pools.get();
			while (remaining_count > 0) {
				if (ptr_pool == nullptr) {
					auto const next_pow2_size = std::size_t{ 1 } << std::bit_width(remaining_count);
					ptr_pool = add_pool(pools ? pools->data.size() << 1 : std::max(next_pow2_size, DefaultStartingSize));
				}

				pool& p = *ptr_pool;
				ptr_pool = ptr_pool->next.get();

				std::size_t const min_space = std::min({ remaining_count, (p.data.size() - p.next_available) });
				if (min_space == 0)
					continue;

				std::span<T> span = p.data.subspan(p.next_available, min_space);
				alloc_callback(span);

				p.next_available += min_space;
				remaining_count -= min_space;
			}
		}

		constexpr void deallocate(std::span<T> const span) {
			assert(validate_addr(span) && "Invalid address passed to deallocate()");

			// Poison the allocation
			if (!std::is_constant_evaluated())
				std::memset(span.data(), 0xee, span.size_bytes());

			// Add it to the free list
			free_list = std::make_unique<free_block>(std::move(free_list), span);
		}

	private:
		constexpr auto* add_pool(std::size_t const size) {
			std::span data{ std::allocator<T>{}.allocate(size), size };
			pools = std::make_unique<pool>(0, data, std::move(pools));
			return pools.get();
		}

		static constexpr bool valid_addr(T* p, T* begin, T* end) {
			// It is undefined behavior to compare pointers directly,
			// so use distances instead. This also works at compile time.
			auto const size = std::distance(begin, end);
			return std::distance(begin, p) >= 0 && std::distance(p, end) <= size;
		}

		constexpr bool validate_addr(std::span<T> const span) {
			for (auto* p = pools.get(); p; p = p->next.get()) {
				if (!valid_addr(span.data(), &p->data.front(), &p->data.back()))
					return false;
			}
			return true;
		}

		struct pool {
			constexpr ~pool() {
				std::allocator<T>{}.deallocate(data.data(), data.size());
			}

			std::size_t next_available;
			std::span<T> data;
			std::unique_ptr<pool> next;
		};

		struct free_block {
			std::unique_ptr<free_block> next;
			std::span<T> span;
		};

		std::unique_ptr<pool> pools;
		std::unique_ptr<free_block> free_list;
	};
} // namespace ecs::detail

#endif // !SCATTER_ALLOCATOR_H
