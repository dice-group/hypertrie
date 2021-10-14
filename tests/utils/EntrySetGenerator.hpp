#ifndef HYPERTRIE_ENTRYSETGENERATOR_HPP
#define HYPERTRIE_ENTRYSETGENERATOR_HPP

#include <algorithm>
#include <cmath>
#include <random>
#include <set>

#include <Dice/hypertrie/Key.hpp>
#include <Dice/hypertrie/internal/raw/RawKey.hpp>

#include <Dice/hypertrie/internal/raw/node/SingleEntry.hpp>

namespace hypertrie::tests::utils {
	template<size_t depth,
			 internal::raw::HypertrieCoreTrait tri_t,
			 typename tri_t::key_part_type max_key_part = typename tri_t::key_part_type(1),
			 typename tri_t::key_part_type min_key_part = typename tri_t::key_part_type(1)>
	class SingleEntryGenerator {
	public:
		using tri = tri_t;
		using value_type = typename tri::value_type;
		using key_part_type = typename tri::key_part_type;

		using SinlgeEntry_t = ::hypertrie::internal::raw::SingleEntry<depth, tri>;

		SinlgeEntry_t entry_;

		bool not_ended_ = false;

		const SinlgeEntry_t &operator*() noexcept {
			return entry_;
		}

		SingleEntryGenerator operator++(int) {
			SingleEntryGenerator old = *this;
			++(*this);
			return old;
		}

		SingleEntryGenerator &operator++() {
			for (size_t i = 0; i < depth; ++i) {
				if (++entry_.key()[i] <= max_key_part)
					return *this;
				else
					entry_.key()[i] = min_key_part;
			}
			not_ended_ = false;
			return *this;
		}

		SingleEntryGenerator &begin() noexcept {
			core::node::RawKey<depth, tri> key{};
			key.fill(min_key_part);
			entry_ = SinlgeEntry_t(key, value_type(1));
			not_ended_ = true;
			return *this;
		}

		bool end() noexcept {
			return false;
		}

		operator bool() {
			return not_ended_;
		}
	};

	template<size_t depth,
			 size_t number_of_entries,
			 internal::raw::HypertrieCoreTrait tri_t,
			 typename tri_t::key_part_type max_key_part = typename tri_t::key_part_type(1),
			 typename tri_t::key_part_type min_key_part = typename tri_t::key_part_type(1)>
	class EntrySetGenerator {
	public:
		using tri = tri_t;
		using value_type = typename tri::value_type;
		using key_part_type = typename tri::key_part_type;

	protected:
		static inline constexpr size_t pow(size_t base, size_t exponent) {
			size_t result = 1;
			for (size_t e = 0; e < exponent; ++e)
				result *= base;
			return result;
		}
		static_assert(min_key_part <= max_key_part);
		static_assert(ssize_t(pow(ssize_t(max_key_part + 1) - ssize_t(min_key_part), depth)) >= ssize_t(number_of_entries));

		static constexpr size_t max_entry_id = pow(size_t(1 + max_key_part - min_key_part), depth) - 1;

		using SinlgeEntry_t = ::hypertrie::internal::raw::SingleEntry<depth, tri>;

		std::vector<SinlgeEntry_t> entries_ = std::vector<SinlgeEntry_t>(number_of_entries);

		bool not_ended_ = false;


		/**
		 * Increments the entry
		 * @param entry
		 * @return if entry was further incremented; false if the key reached the maximum and was reset to all min_key_part
		 */
		static inline bool inc_entry_permutation(SinlgeEntry_t &entry) noexcept {

			for (size_t i = 0; i < depth; ++i) {
				if (++entry.key()[i] <= max_key_part)
					return true;
				else
					entry.key()[i] = min_key_part;
			}
			return false;
		}


		static inline size_t id_of(const SinlgeEntry_t &entry) noexcept {
			size_t id = 0;
			size_t factor = 1;
			for (size_t i = 0; i < depth; ++i) {
				if (i > 0)
					factor *= (max_key_part - min_key_part + 1);
				id += (entry.key()[i] - min_key_part) * pow(max_key_part - min_key_part + 1, i);
			}
			return id;
		}

		/**
		 * Increments the list of entries
		 * @param entry
		 * @return if entry was further incremented; false if the key reached the maximum and was reset to all min_key_part
		 */
		bool inc_entries_combination() noexcept {
			ssize_t i;
			for (i = 0; i < ssize_t(number_of_entries); ++i) {
				if (inc_entry_permutation(entries_[i]) and (id_of(entries_[i]) <= max_entry_id - i)) {
					break;
				}
			}

			if (i == number_of_entries)
				return false;
			for (ssize_t j = 0; j < i; ++j) {
				entries_[i - (j + 1)] = entries_[i - j];
				inc_entry_permutation(entries_[i - (j + 1)]);
			}
			return true;
		}

	public:
		EntrySetGenerator &operator++() noexcept {
			if (not inc_entries_combination())
				not_ended_ = false;
			return *this;
		}

		const std::vector<SinlgeEntry_t> &operator*() noexcept {
			return entries_;
		}

		EntrySetGenerator operator++(int) &noexcept {
			SingleEntryGenerator old = *this;
			++(*this);
			return old;
		}

		EntrySetGenerator &begin() noexcept {
			assert(ssize_t(std::pow(ssize_t(max_key_part + 1) - ssize_t(min_key_part), depth)) >= ssize_t(number_of_entries));
			static core::node::RawKey<depth, tri> key{};
			key.fill(min_key_part);

			for (size_t i = 0; i < number_of_entries; ++i) {
				if (i == 0)
					entries_[number_of_entries - 1] = SinlgeEntry_t{key, value_type(1)};
				else {
					entries_[number_of_entries - 1 - i] = entries_[number_of_entries - i - 1 + 1];
					inc_entry_permutation(entries_[number_of_entries - 1 - i]);
				}
			}
			not_ended_ = true;
			return *this;
		}

		bool end() noexcept {
			return false;
		}

		operator bool() noexcept {
			return not_ended_;
		}
	};
	template<size_t depth,
			 size_t number_of_entries,
			 internal::raw::HypertrieCoreTrait tri_t,
			 typename tri_t::key_part_type max_key_part = typename tri_t::key_part_type(1)>
	class EntrySetGenerator_with_exclude : public EntrySetGenerator<depth, number_of_entries, tri_t, max_key_part> {
		using super_t = EntrySetGenerator<depth, number_of_entries, tri_t, max_key_part>;
		using SinlgeEntry_t = typename super_t ::SinlgeEntry_t;
		const std::vector<SinlgeEntry_t> &excluded_entries_;

	public:
		EntrySetGenerator_with_exclude(const std::vector<SinlgeEntry_t> &excludedEntries) : excluded_entries_(excludedEntries) {}


		EntrySetGenerator_with_exclude &operator++() noexcept {
			do {
				if (not this->inc_entries_combination()) {
					this->not_ended_ = false;
					return *this;
				}
			} while (std::ranges::any_of(this->entries_, [&](const auto &entry) {
				return std::ranges::any_of(this->excluded_entries_, [&](const auto &excluded_entry) {//
					return entry == excluded_entry;
				});
			}));
			return *this;
		}

		EntrySetGenerator_with_exclude operator++(int) &noexcept {
			EntrySetGenerator_with_exclude old = *this;
			++(*this);
			return old;
		}

		EntrySetGenerator_with_exclude &begin() noexcept {
			super_t::begin();
			if (std::ranges::any_of(this->entries_, [&](const auto &entry) {
					return std::ranges::any_of(this->excluded_entries_, [&](const auto &excluded_entry) {//
						return entry == excluded_entry;
					});
				}))
				++(*this);
			return *this;
		}
	};

}// namespace hypertrie::tests::utils
#endif//HYPERTRIE_ENTRYSETGENERATOR_HPP
