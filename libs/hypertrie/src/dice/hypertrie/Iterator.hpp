#ifndef HYPERTRIE_ITERATOR_HPP
#define HYPERTRIE_ITERATOR_HPP

#include "dice/hypertrie/Hypertrie_predeclare.hpp"
#include "dice/hypertrie/internal/raw/iteration/RawIterator.hpp"
#include "dice/template-library/switch_cases.hpp"

namespace dice::hypertrie {

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class Iterator {
	public:
		using key_part_type = typename htt_t::key_part_type;

	protected:
		using max_sized_RawIterator_t = internal::raw::RawIterator<hypertrie_max_depth, false, htt_t, allocator_type, hypertrie_max_depth>;
		template<size_t depth>
		using RawIterator_t = internal::raw::RawIterator<depth, false, htt_t, allocator_type, hypertrie_max_depth>;

		// TODO: same as in HashDiagonal.hpp. Rewrite with typical inheritance might advance readability.
		//  However the runtime cost might be problematic.
		struct RawMethods {
			void (*construct)(const const_Hypertrie<htt_t, allocator_type> &, void *) noexcept = nullptr;
			void (*destroy)(void *) noexcept = nullptr;

			NonZeroEntry<htt_t> const &(*value)(void const *) noexcept = nullptr;

			void (*inc)(void *) noexcept = nullptr;

			bool (*ended)(void const *) noexcept = nullptr;
		};

		template<size_t depth>
		inline static RawMethods generate_raw_methods() noexcept {
			return RawMethods{
					.construct =
							[](const_Hypertrie<htt_t, allocator_type> const &hypertrie, void *raw_iterator_ptr) noexcept {
								if (hypertrie.empty())
									std::construct_at(reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr));
								else if (hypertrie.contextless())
									std::construct_at(reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr), hypertrie.template stl_node_container<depth>());
								else
									std::construct_at(reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr), hypertrie.template node_container<depth>(), hypertrie.context()->raw_context());
							},
					.destroy =
							[](void *raw_iterator_ptr) noexcept {
							  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
							  std::destroy_at(reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr));
							},
					.value =
							[](void const *raw_iterator_ptr) noexcept -> NonZeroEntry<htt_t> const & {
							  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
							  return reinterpret_cast<RawIterator_t<depth> const *>(raw_iterator_ptr)->value();
					},
					.inc =
							[](void *raw_iterator_ptr) noexcept {
							  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
							  reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr)->inc();
							},
					.ended =
							[](void const *raw_iterator_ptr) noexcept -> bool {
							  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
							  return reinterpret_cast<RawIterator_t<depth> const *>(raw_iterator_ptr)->ended();
					}};
		}

		inline static const std::vector<RawMethods> raw_method_cache = []() noexcept {
			using namespace internal;
			std::vector<RawMethods> raw_methods;
			for (size_t depth = 1; depth < hypertrie_max_depth + 1; ++depth) {
				raw_methods.push_back(template_library::switch_cases<1, hypertrie_max_depth + 1>(
						depth,
						[](auto depth_arg) -> RawMethods {
							return generate_raw_methods<depth_arg>();
						},
						[]() -> RawMethods { assert(false); __builtin_unreachable(); }));
			}
			return raw_methods;
		}();


		static RawMethods const &get_raw_methods(size_t depth) noexcept {
			return raw_method_cache[depth - 1];
		};

		RawMethods const *raw_methods = nullptr;
		std::array<std::byte, sizeof(max_sized_RawIterator_t)> raw_iterator;

	public:
		using value_type = NonZeroEntry<htt_t>;

		Iterator() = default;

		// TODO: review
		Iterator &operator=(Iterator const &other) noexcept {
			if (this == *other) {
				return *this;
			}
			if (raw_methods != nullptr) {
				raw_methods->destroy(&raw_iterator);
			}
			this->raw_methods = other.raw_methods;
			this->raw_iterator = other.raw_iterator;
			return *this;
		}

		// TODO: review
		Iterator(Iterator const &other) noexcept {
			assert(this != &other);
			if (raw_methods != nullptr) {
				raw_methods->destroy(&raw_iterator);
			}
			this->raw_methods = other.raw_methods;
			this->raw_iterator = other.raw_iterator;
		}

		Iterator(Iterator &&other) noexcept {
			assert(this != &other);
			if (raw_methods != nullptr) {
				raw_methods->destroy(&raw_iterator);
			}
			this->raw_methods = other.raw_methods;
			this->raw_iterator = other.raw_iterator;
			other.raw_iterator = {};
			other.raw_methods = nullptr;
		}

		Iterator &operator=(Iterator &&other) noexcept {
			assert(this != &other);
			if (raw_methods != nullptr) {
				raw_methods->destroy(&raw_iterator);
			}
			this->raw_methods = other.raw_methods;
			this->raw_iterator = other.raw_iterator;
			other.raw_iterator = {};
			other.raw_methods = nullptr;
			return *this;
		}

		~Iterator() noexcept {
			if (raw_methods != nullptr) {
				raw_methods->destroy(&raw_iterator);
			}
			raw_methods = nullptr;
		}
		explicit Iterator(const_Hypertrie<htt_t, allocator_type> const &hypertrie) noexcept : raw_methods(&get_raw_methods(hypertrie.depth())) {
			raw_methods->construct(hypertrie, &raw_iterator);
		}

		Iterator &operator++() noexcept {
			raw_methods->inc(&raw_iterator);
			return *this;
		}

		Iterator operator++(int) noexcept {
			auto copy = *this;
			++(*this);
			return copy;
		}

		value_type const &operator*() const noexcept { return raw_methods->value(&raw_iterator); }

		value_type const *operator->() const noexcept { return &raw_methods->value(&raw_iterator); }


		operator bool() const noexcept { return not raw_methods->ended(&raw_iterator); }
	};

}// namespace dice::hypertrie


#endif//HYPERTRIE_ITERATOR_HPP
