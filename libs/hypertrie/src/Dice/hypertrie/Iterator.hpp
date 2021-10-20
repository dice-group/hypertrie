#ifndef HYPERTRIE_ITERATOR_HPP
#define HYPERTRIE_ITERATOR_HPP

#include "Dice/hypertrie/Hypertrie_predeclare.hpp"
#include "Dice/hypertrie/internal/raw/iteration/RawIterator.hpp"

namespace hypertrie {

	template<internal::HypertrieTrait tr_t>
	class Iterator {
	public:
		using tr = tr_t;
		using tri = internal::raw::template Hypertrie_core_t<tr>;
		using key_part_type = typename tr::key_part_type;

	protected:
		using max_sized_RawIterator_t = internal::raw::RawIterator<hypertrie_max_depth, false, tri, hypertrie_max_depth>;
		template<size_t depth>
		using RawIterator_t = internal::raw::RawIterator<depth, false, tri, hypertrie_max_depth>;

		struct RawMethods {
			void (*construct)(const const_Hypertrie<tr> &, void *) noexcept = nullptr;

			NonZeroEntry<tr> const &(*value)(void const *) noexcept = nullptr;

			void (*inc)(void *) noexcept = nullptr;

			bool (*ended)(void const *) noexcept = nullptr;

			RawMethods() = default;

			RawMethods(void (*construct)(const_Hypertrie<tr> const &, void *) noexcept,
					   NonZeroEntry<tr> const &(*value)(const void *) noexcept,
					   void (*inc)(void *) noexcept,
					   bool (*ended)(void const *) noexcept)
				: construct(construct), value(value), inc(inc), ended(ended) {}
		};

		template<size_t depth>
		inline static RawMethods generateRawMethods() noexcept {
			return RawMethods(
					[](const_Hypertrie<tr> const &hypertrie, void *raw_iterator_ptr) noexcept {
						std::construct_at(reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr), hypertrie.template node_container<depth>(), hypertrie.context()->raw_context());
					},
					[](void const *raw_iterator_ptr) noexcept -> NonZeroEntry<tr> const & {
						return reinterpret_cast<RawIterator_t<depth> const *>(raw_iterator_ptr)->value();
					},
					[](void *raw_iterator_ptr) noexcept {
						reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr)->inc();
					},
					[](void const *raw_iterator_ptr) noexcept -> bool {
						return reinterpret_cast<RawIterator_t<depth> const *>(raw_iterator_ptr)->ended();
					});
		}

		inline static const std::vector<RawMethods> raw_method_cache = []() noexcept {
			using namespace internal;
			std::vector<RawMethods> raw_methods;
			for (size_t depth : iter::range(1UL, hypertrie_max_depth + 1))
				raw_methods.push_back(util::switch_cases<1, hypertrie_max_depth + 1>(
						depth,
						[](auto depth_arg) -> RawMethods {
							return generateRawMethods<depth_arg>();
						},
						[]() -> RawMethods { assert(false); __builtin_unreachable(); }));
			return raw_methods;
		}();

		static RawMethods const &getRawMethods(size_t depth) noexcept {
			return raw_method_cache[depth - 1];
		};


	protected:
		RawMethods const *raw_methods = nullptr;
		std::array<std::byte, sizeof(max_sized_RawIterator_t)> raw_iterator;

	public:
		using value_type = NonZeroEntry<tr>;

		Iterator() = default;

		explicit Iterator(const_Hypertrie<tr> const &hypertrie) noexcept : raw_methods(&getRawMethods(hypertrie.depth())) {
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

		value_type const &operator*() const { return raw_methods->value(&raw_iterator); }

		operator bool() const noexcept { return not raw_methods->ended(&raw_iterator); }
	};

}// namespace hypertrie


#endif//HYPERTRIE_ITERATOR_HPP
