#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <utils/AssetGenerator.hpp>
#include <dice/hypertrie/internal/raw/RawKey.hpp>
#include <utils/Node_test_configs.hpp>

#include <dice/hypertrie/internal/Hypertrie_traits_tostring.hpp>
#include <dice/hypertrie/internal/raw/node/RawIdentifier.hpp>

namespace dice::hypertrie::tests::core::node {

	using namespace dice::hypertrie::internal::raw;
	TEST_CASE_TEMPLATE("RawIdentifier", T,
					   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>, bool_cfg<4>, bool_cfg<5>,
					   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
					   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
					   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>//
	) {

		using htt_t = typename T::htt_t;
		constexpr size_t depth = T::depth;
		using RawKey_t = RawKey<depth, htt_t>;
		using RawIdentifier_t = RawIdentifier<depth, htt_t>;
		using Entry = SingleEntry<depth, htt_t>;

		hypertrie::tests::utils::RawGenerator<depth, htt_t> gen{};

		SUBCASE(::dice::hypertrie::internal::to_string<typename T::htt_t>().c_str()) {
			SUBCASE(fmt::format("depth = {}", depth).c_str()) {
				SUBCASE("default construct") {
					REQUIRE(RawIdentifier_t().empty());
					REQUIRE(RawIdentifier_t() == RawIdentifier_t(std::initializer_list<Entry>{}));
				}

				SUBCASE("hash for single entry node") {
					RawKey_t key{};
					RawIdentifier_t sen_id(Entry{key, typename htt_t::value_type{1}});
					REQUIRE(sen_id.is_sen());
					REQUIRE(not sen_id.empty());
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
						if constexpr (not htt_t::is_bool_valued) {
							auto new_value = std::numeric_limits<typename htt_t::value_type>::max();
							if (new_value == entries[0].value())
								new_value -= 1;
							REQUIRE(RawIdentifier_t{id0}.change_value(entries[0], std::numeric_limits<typename htt_t::value_type>::max()) == RawIdentifier_t{Entry{entries[0].key(), new_value}});
						}
					}

					RawIdentifier_t id1 = id0;
					id1.add_entry(entries[1], IdentifierTag::FN);
					REQUIRE(id1 == RawIdentifier_t{std::initializer_list<SingleEntry<depth, htt_t>>{entries[0], entries[1]}, IdentifierTag::FN});
					REQUIRE(id1.is_fn());
					if constexpr (not(depth == 1 and htt_t::taggable_key_part))
						REQUIRE(RawIdentifier_t(id1).remove_entry(entries[1], IdentifierTag::SEN) == id0);
					SUBCASE("change value (2 entries)") {
						if constexpr (not htt_t::is_bool_valued) {
							auto new_value = std::numeric_limits<typename htt_t::value_type>::max();
							if (new_value == entries[1].value())
								new_value -= 1;
							REQUIRE(RawIdentifier_t{id1}.change_value(entries[1], std::numeric_limits<typename htt_t::value_type>::max())
									== RawIdentifier_t{std::initializer_list<SingleEntry<depth, htt_t>>{entries[0], Entry{entries[1].key(), new_value}}, IdentifierTag::FN});
						}
					}

					RawIdentifier_t id2 = id1;
					id2.add_entry(entries[2], IdentifierTag::FN);
					REQUIRE(id2 == RawIdentifier_t{std::initializer_list<SingleEntry<depth, htt_t>>{entries[0], entries[1], entries[2]}, IdentifierTag::FN});
					REQUIRE(id2.is_fn());
					if constexpr (not(depth == 1 and htt_t::taggable_key_part))
						REQUIRE(RawIdentifier_t(id2).remove_entry(entries[2], IdentifierTag::FN) == id1);
					REQUIRE(RawIdentifier_t(id2).remove_entry(entries[2], IdentifierTag::FN) == id1);


					RawIdentifier_t id3 = id2;
					id3.add_entry(entries[3], IdentifierTag::FN);
					REQUIRE(id3 == RawIdentifier_t{std::initializer_list<SingleEntry<depth, htt_t>>{entries[0], entries[1], entries[2], entries[3]}, IdentifierTag::FN});
					REQUIRE(id3.is_fn());
					if constexpr (not(depth == 1 and htt_t::taggable_key_part))
						REQUIRE(RawIdentifier_t(id3).remove_entry(entries[3], IdentifierTag::FN) == id2);
					REQUIRE(RawIdentifier_t(id3).remove_entry(entries[3], IdentifierTag::FN) == id2);

					if constexpr (not(depth == 1 and htt_t::taggable_key_part))
						REQUIRE(RawIdentifier_t(id3)
										.remove_entry(entries[2])
										.remove_entry(entries[1])
										.remove_entry(entries[3], IdentifierTag::SEN) == id0);
				}
			}
		}
	}
}// namespace dice::hypertrie::tests::core::node