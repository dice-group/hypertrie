#ifndef HYPERTRIE_HYPERTRIE_HPP
#define HYPERTRIE_HYPERTRIE_HPP

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

#include "Dice/hypertrie/internal/node_based/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <vector>
#include <optional>
#include <variant>

namespace hypertrie::internal::node_based {

	template<typename tr = default_bool_Hypertrie_t>
	class const_Hypertrie {
	public:
		using traits = tr;

		using Key = typename tr::Key;
		using SliceKey = typename tr::SliceKey;
		using value_type = typename  tr::value_type;

		[[nodiscard]]
		std::vector<size_t> getCards(const std::vector<pos_type> &positions) const;

		[[nodiscard]]
		size_t size() const;

		[[nodiscard]]
		bool operator[](const Key &key) const;

		[[nodiscard]]
		std::variant<std::optional<const_Hypertrie>, value_type> operator[](const SliceKey &slice_key) const;

		class iterator;

		using const_iterator = iterator;

		[[nodiscard]]
		iterator begin() const { return iterator{this}; }

		[[nodiscard]]
		const_iterator cbegin() const { return iterator{this}; }

		[[nodiscard]]
		bool end() const { return false; }

		[[nodiscard]]
		bool cend() const { return false; }

	};

	template<typename tr = default_bool_Hypertrie_t>
	class Hypertrie : public const_Hypertrie<tr> {
	public:
		using traits = tr;

		using Key = typename tr::Key;
		using value_type = typename  tr::value_type;

		void set(const Key &key, value_type value);
	};

}

#endif //HYPERTRIE_HYPERTRIE_HPP
