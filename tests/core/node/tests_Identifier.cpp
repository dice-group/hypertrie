#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include <AssetGenerator.hpp>
#include <Dice/hypertrie/internal/raw/RawKey.hpp>
#include <Node_test_configs.hpp>

#include <Dice/hypertrie/internal/Hypertrie_traits_tostring.hpp>
#include <Dice/hypertrie/internal/raw/node/Identifier.hpp>

namespace hypertrie::tests::core::node {

	using namespace hypertrie::internal::raw;
	TEST_CASE_TEMPLATE("RawIdentifier", T,
					   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
					   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
					   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
					   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>//
	) {

		using tri = typename T::tri;
		constexpr size_t depth = T::depth;
		using RawKey_t = RawKey<depth, tri>;
		using RawIdentifier_t = RawIdentifier<depth, tri>;
		using Entry = SingleEntry<depth, tri>;

		hypertrie::tests::utils::RawGenerator<depth, tri> gen{};

		SUBCASE(::hypertrie::internal::to_string<typename T::tr>().c_str()) {
			SUBCASE(fmt::format("depth = {}", depth).c_str()) {
				SUBCASE("default construct") {
					REQUIRE(RawIdentifier_t().empty());
					REQUIRE(RawIdentifier_t() == RawIdentifier_t(std::initializer_list<Entry>{}));
				}

				SUBCASE("hash for single entry node") {
					RawKey_t key{};
					RawIdentifier_t sen_id(Entry{key, {}});
					REQUIRE(sen_id.is_sen());
					REQUIRE(not sen_id.empty());
					REQUIRE(sen_id);// inverse of above
				}

				SUBCASE("add and remove starting with compressed") {
					// TODO: generate entries directly
					std::vector<Entry> entries = [&]() {
						std::vector<Entry> entries;
						for (auto &&entry : gen.entries(4))
							entries.push_back(Entry{entry.first, entry.second});
						return entries;
					}();


					RawIdentifier_t id0(entries[0]);
					REQUIRE(id0 == RawIdentifier_t(std::initializer_list<Entry>{entries[0]}));
					REQUIRE(id0.is_sen());
					SUBCASE("change value (1 entry)") {
						if constexpr (not tri::is_bool_valued) {
							auto new_value = std::numeric_limits<typename tri::value_type>::max();
							if (new_value == entries[0].value())
								new_value -= 1;
							REQUIRE(RawIdentifier_t{id0}.changeValue(entries[0], std::numeric_limits<typename tri::value_type>::max()) == RawIdentifier_t{Entry{entries[0].key(), new_value}});
						}
					}

					RawIdentifier_t id1 = id0;
					id1.addEntry(entries[1]);
					REQUIRE(id1 == RawIdentifier_t{entries[0], entries[1]});
					REQUIRE(id1.is_fn());
					if constexpr (not(depth == 1 and tri::taggable_key_part))
						REQUIRE(RawIdentifier_t(id1).removeEntry(entries[1], true) == id0);
					SUBCASE("change value (2 entries)") {
						if constexpr (not tri::is_bool_valued) {
							auto new_value = std::numeric_limits<typename tri::value_type>::max();
							if (new_value == entries[1].value())
								new_value -= 1;
							REQUIRE(RawIdentifier_t{id1}.changeValue(entries[1], std::numeric_limits<typename tri::value_type>::max()) == RawIdentifier_t{entries[0], Entry{entries[1].key(), new_value}});
						}
					}

					RawIdentifier_t id2 = id1;
					id2.addEntry(entries[2]);
					REQUIRE(id2 == RawIdentifier_t{entries[0], entries[1], entries[2]});
					REQUIRE(id2.is_fn());
					if constexpr (not(depth == 1 and tri::taggable_key_part))
						REQUIRE(RawIdentifier_t(id2).removeEntry(entries[2], false) == id1);
					REQUIRE(RawIdentifier_t(id2).removeEntry(entries[2]) == id1);


					RawIdentifier_t id3 = id2;
					id3.addEntry(entries[3]);
					REQUIRE(id3 == RawIdentifier_t{entries[0], entries[1], entries[2], entries[3]});
					REQUIRE(id3.is_fn());
					if constexpr (not(depth == 1 and tri::taggable_key_part))
						REQUIRE(RawIdentifier_t(id3).removeEntry(entries[3], false) == id2);
					REQUIRE(RawIdentifier_t(id3).removeEntry(entries[3]) == id2);

					if constexpr (not(depth == 1 and tri::taggable_key_part))
						REQUIRE(RawIdentifier_t(id3)
										.removeEntry(entries[2])
										.removeEntry(entries[1])
										.removeEntry(entries[3], true) == id0);
				}

				SUBCASE("use default sorting") {
					std::vector<Entry> entries = [&]() {
						std::vector<Entry> entries;
						for (auto &&entry : gen.entries(4))
							entries.push_back(Entry{entry.first, entry.second});
						return entries;
					}();
					RawIdentifier_t id_empty{};
					RawIdentifier_t id0(entries[0]);
					RawIdentifier_t id1{entries[0], entries[1]};
					RawIdentifier_t id2{entries[0], entries[1], entries[2]};
					RawIdentifier_t id3{entries[0], entries[1], entries[2], entries[3]};
					std::vector<RawIdentifier_t> ids{id_empty, id0, id1, id2, id3};

					std::sort(ids.begin(), ids.end());
				}
			}
		}
	}
}// namespace hypertrie::tests::core::node