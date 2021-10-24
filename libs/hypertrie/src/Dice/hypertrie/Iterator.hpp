#ifndef HYPERTRIE_ITERATOR_HPP
#define HYPERTRIE_ITERATOR_HPP

#include "Dice/hypertrie/Hypertrie_predeclare.hpp"
#include "Dice/hypertrie/internal/raw/iteration/RawIterator.hpp"

namespace Dice::hypertrie {

	template<HypertrieTrait tr_t>
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
			void (*destroy)(void *) noexcept = nullptr;

			NonZeroEntry<tr> const &(*value)(void const *) noexcept = nullptr;

			void (*inc)(void *) noexcept = nullptr;

			bool (*ended)(void const *) noexcept = nullptr;
		};

		template<size_t depth>
		inline static RawMethods generate_raw_methods() noexcept {
			return RawMethods{
					.construct =
							[](const_Hypertrie<tr> const &hypertrie, void *raw_iterator_ptr) noexcept {
								if (not(hypertrie.size() == 1 and hypertrie.contextless()))
									std::construct_at(reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr), hypertrie.template node_container<depth>(), hypertrie.context()->raw_context());
								else
									std::construct_at(reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr), hypertrie.template stl_node_container<depth>());
							},
					.destroy =
							[](void *raw_iterator_ptr) noexcept {
								std::destroy_at(reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr));
							},
					.value =
							[](void const *raw_iterator_ptr) noexcept -> NonZeroEntry<tr> const & {
						return reinterpret_cast<RawIterator_t<depth> const *>(raw_iterator_ptr)->value();
					},
					.inc =
							[](void *raw_iterator_ptr) noexcept {
								reinterpret_cast<RawIterator_t<depth> *>(raw_iterator_ptr)->inc();
							},
					.ended =
							[](void const *raw_iterator_ptr) noexcept -> bool {
						return reinterpret_cast<RawIterator_t<depth> const *>(raw_iterator_ptr)->ended();
					}};
		}

		inline static const std::vector<RawMethods> raw_method_cache = []() noexcept {
			using namespace internal;
			std::vector<RawMethods> raw_methods;
			for (size_t depth : iter::range(1UL, hypertrie_max_depth + 1))
				raw_methods.push_back(util::switch_cases<1, hypertrie_max_depth + 1>(
						depth,
						[](auto depth_arg) -> RawMethods {
							return generate_raw_methods<depth_arg>();
						},
						[]() -> RawMethods { assert(false); __builtin_unreachable(); }));
			return raw_methods;
		}();

		static RawMethods const &get_raw_methods(size_t depth) noexcept {
			return raw_method_cache[depth - 1];
		};


	protected:
		RawMethods const *raw_methods = nullptr;
		std::array<std::byte, sizeof(max_sized_RawIterator_t)> raw_iterator;

	public:
		using value_type = NonZeroEntry<tr>;

		Iterator() = default;

		Iterator(Iterator &&other) noexcept {
			if (raw_methods != nullptr)
				raw_methods->destroy(&raw_iterator);
			this->raw_methods = other.raw_methods;
			this->raw_iterator = other.raw_iterator;
			other.raw_iterator = {};
			other.raw_methods = nullptr;
		}

		// TODO: copy constructor and assignment
		Iterator &operator=(Iterator &&other) noexcept {
			if (raw_methods != nullptr)
				raw_methods->destroy(&raw_iterator);
			this->raw_methods = other.raw_methods;
			this->raw_iterator = other.raw_iterator;
			other.raw_iterator = {};
			other.raw_methods = nullptr;
			return *this;
		}

		~Iterator() noexcept {
			if (raw_methods != nullptr)
				raw_methods->destroy(&raw_iterator);
			raw_methods = nullptr;
		}
		explicit Iterator(const_Hypertrie<tr> const &hypertrie) noexcept : raw_methods(&get_raw_methods(hypertrie.depth())) {
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

		operator bool() const noexcept { return not raw_methods->ended(&raw_iterator); }
	};

}// namespace Dice::hypertrie


#endif//HYPERTRIE_ITERATOR_HPP
