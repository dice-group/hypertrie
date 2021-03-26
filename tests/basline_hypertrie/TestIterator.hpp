#ifndef HYPERTRIE_TESTRAWITERATOR_HPP
#define HYPERTRIE_TESTRAWITERATOR_HPP


#include <Dice/hypertrie/internal/Hypertrie_traits.hpp>
#include <Dice/hypertrie/internal/raw/iterator/Diagonal.hpp>
#include <Dice/hypertrie/internal/raw/iterator/Iterator.hpp>
#include <Dice/hypertrie/internal/raw/storage/NodeContext.hpp>
#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"

#include <Dice/hypertrie/internal/Hypertrie.hpp>

namespace hypertrie::tests::raw::node_context::iterator_test {

	using namespace fmt::literals;

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::raw;

	template<HypertrieInternalTrait tri, size_t depth>
	void randomized_iterator_test() {
		using tr = typename tri::tr;
		using IteratorEntry = typename tr::IteratorEntry;
		using iter_funcs = typename tr::iterator_entry;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::Key;

		static utils::EntryGenerator<key_part_type, value_type, size_t(tri::is_lsb_unused)> gen{};

		Hypertrie<tr> nodec{depth};

		for (size_t count : iter::range(0, 30)) {
			gen.setKeyPartMinMax(0, size_t((count / depth + 1) * 1.5));
			gen.setValueMinMax(value_type(1), value_type(5));
			SECTION("{} entries"_format(count)) {
				for (const auto i : iter::range(10)) {
					SECTION("{}"_format(i)) {
						auto entries = [&]() {
							auto entries = gen.entries(count, depth);
							std::map<Key, value_type> entries_map;
							for (const auto &[key, value] : entries)
								entries_map[key] = value;
							return entries_map;
						}();

						for (const auto &[key, value] : entries)
							nodec.set(key, value);

						std::set<Key> found_keys{};


						for (const auto &actual_key : nodec) {
							auto actual_value = true;
							// check if the key is valid
							REQUIRE(entries.count(actual_key));
							// check if the value is valid
							REQUIRE(entries[actual_key] == actual_value);
							// check that the entry was not already found
							REQUIRE(not found_keys.count(actual_key));
							found_keys.insert(actual_key);

							WARN("[{}] -> {}\n"_format(fmt::join(actual_key, ", "), actual_value));
						}
						REQUIRE(found_keys.size() == entries.size());
					}
				}
			}
		}
	}

	TEMPLATE_TEST_CASE_SIG("iterating hypertrie entries [bool]", "[RawIterator]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_iterator_test<default_bool_Hypertrie_internal_t, depth>();
	}
}// namespace hypertrie::tests::raw::node_context::iterator_test

#endif//HYPERTRIE_TESTRAWITERATOR_HPP
