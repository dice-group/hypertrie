#ifndef HYPERTRIE_TESTRAWITERATOR_HPP
#define HYPERTRIE_TESTRAWITERATOR_HPP


#include <Dice/hypertrie/internal/Hypertrie_traits.hpp>
#include <Dice/hypertrie/internal/raw/iterator/Diagonal.hpp>
#include <Dice/hypertrie/internal/raw/iterator/Iterator.hpp>
#include <Dice/hypertrie/internal/raw/storage/NodeContext.hpp>
#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "TestTensor.hpp"


namespace hypertrie::tests::raw::node_context::iterator_test {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::raw;

	using namespace hypertrie::internal;

	template<HypertrieInternalTrait tri, size_t depth>
	void randomized_iterator_test() {
		using tr = typename tri::tr;
		using IteratorEntry = typename tr::IteratorEntry;
		using iter_funcs = typename tr::iterator_entry;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static utils::RawGenerator<depth, key_part_type, value_type, size_t(tri::is_lsb_unused)> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nodec{};

		for (size_t count : iter::range(0, 30)) {
			gen.setKeyPartMinMax(0, size_t((count / depth + 1) * 1.5));
			gen.setValueMinMax(value_type(1), value_type(5));
			SECTION("{} entries"_format(count)) {
				for (const auto i : iter::range(10)) {
					SECTION("{}"_format(i)) {
						auto entries = [&]() {
							auto entries = gen.entries(count);
							std::map<Key, value_type> entries_map;
							for (const auto &[key, value] : entries)
								entries_map[key] = value;
							return entries_map;
						}();

						for (const auto &[key, value] : entries)
							context.template set<depth>(nodec, key, value);

						std::set<Key> found_keys{};
						for (auto iter = iterator<depth, tri>(nodec, context); iter != false; ++iter) {
							IteratorEntry entry = *iter;
							auto actual_key = iter_funcs::key(entry);
							auto actual_rawkey = [&]() {
								Key raw_key;
								for (auto [raw_key_part, non_raw_key_part] : iter::zip(raw_key, actual_key))
									raw_key_part = non_raw_key_part;
								return raw_key;
							}();
							auto actual_value = iter_funcs::value(entry);
							// check if the key is valid
							REQUIRE(entries.count(actual_rawkey));
							// check if the value is valid
							REQUIRE(entries[actual_rawkey] == actual_value);
							// check that the entry was not already found
							REQUIRE(not found_keys.count(actual_rawkey));
							found_keys.insert(actual_rawkey);

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

	TEMPLATE_TEST_CASE_SIG("iterating hypertrie entries [bool lsb-unused]", "[RawIterator]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_iterator_test<lsbunused_bool_Hypertrie_internal_t, depth>();
	}

	TEMPLATE_TEST_CASE_SIG("iterating hypertrie entries [long]", "[RawIterator]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_iterator_test<default_long_Hypertrie_internal_t, depth>();
	}

	TEMPLATE_TEST_CASE_SIG("iterating hypertrie entries [double]", "[RawIterator]", ((size_t depth), depth), 1, 2, 3, 4, 5) {
		randomized_iterator_test<default_double_Hypertrie_internal_t, depth>();
	}
}// namespace hypertrie::tests::raw::node_context::iterator_test

#endif//HYPERTRIE_TESTRAWITERATOR_HPP
