#ifndef HYPERTRIE_KEY_HPP
#define HYPERTRIE_KEY_HPP

#include <Dice/hypertrie/internal/Hypertrie_trait.hpp>
#include <optional>
#include <vector>

namespace hypertrie {
	template<::hypertrie::internal::HypertrieTrait tr>
	class Key : public ::std::vector<typename tr::key_part_type> {
		using super_t = ::std::vector<typename tr::key_part_type>;

	public:
		Key() = default;
		explicit Key(typename super_t::size_type n, const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(n, a) {}

		Key(typename super_t::size_type n, const typename super_t::value_type &value,
			const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(n, value, a){};

		Key(super_t &&rv, const typename super_t::allocator_type &m) noexcept
			: super_t(std::move(rv), m, typename super_t::_Alloc_traits::is_always_equal{}) {}

		Key(std::initializer_list<typename super_t::value_type> l,
			const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(l, a) {}

		template<typename InputIterator,
				 typename = std::_RequireInputIter<InputIterator>>
		Key(InputIterator first, InputIterator last,
			const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(first, last, a) {}
	};

	template<::hypertrie::internal::HypertrieTrait tr>
	class SliceKey : public ::std::vector<::std::optional<typename tr::key_part_type>> {
		using super_t = ::std::vector<::std::optional<typename tr::key_part_type>>;

	public:
		SliceKey() = default;
		explicit SliceKey(typename super_t::size_type n, const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(n, a) {}

		SliceKey(typename super_t::size_type n, const typename super_t::value_type &value,
				 const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(n, value, a){};

		SliceKey(super_t &&rv, const typename super_t::allocator_type &m) noexcept
			: super_t(std::move(rv), m, typename super_t::_Alloc_traits::is_always_equal{}) {}

		SliceKey(std::initializer_list<typename super_t::value_type> l,
				 const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(l, a) {}

		template<typename InputIterator,
				 typename = std::_RequireInputIter<InputIterator>>
		SliceKey(InputIterator first, InputIterator last,
				 const typename super_t::allocator_type &a = typename super_t::allocator_type())
			: super_t(first, last, a) {}

	public:
		[[nodiscard]] size_t get_fixed_depth() const noexcept {
			return this->size() - std::ranges::count_if(*this, [](auto const &item) { return not item.has_value(); });
		}
	};
}// namespace hypertrie

#endif//HYPERTRIE_KEY_HPP