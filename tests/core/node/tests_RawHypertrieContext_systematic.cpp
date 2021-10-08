#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include "ValidationRawNodeContext.hpp"
#include <Dice/hypertrie/internal/util/fmt_utils.hpp>
#include <Node_test_configs.hpp>
#include <RawEntryGenerator.hpp>


#include <Dice/hypertrie/internal/raw/fmt_Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <Dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

#include <EntrySetGenerator.hpp>

namespace hypertrie::tests::core::node {

	TEST_SUITE("systematic testing of RawNodeContext") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;


		template<size_t depth, HypertrieCoreTrait tri>
		void write_and_read() {
			SUBCASE("{}"_format(tri{}).c_str()) {
				SUBCASE("hypertrie depth = {}"_format(depth).c_str()) {

					SUBCASE("1 and 1 entry") {

						using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;

						using key_part_type = typename tri::key_part_type;
						using value_type = typename tri::value_type;

						utils::RawEntryGenerator<depth, tri> gen{};

						std::vector<key_part_type> s{iter::range(key_part_type(1), key_part_type(3 + 1)).begin(), iter::range(key_part_type(1), key_part_type(3 + 1)).end()};
						for (auto key_0_e : iter::combinations_with_replacement(s, depth)) {
							SingleEntry_t entry_0{{}, value_type(1)};
							std::copy_n(key_0_e.begin(), depth, entry_0.key().begin());
							SUBCASE("entry_0 {}"_format(entry_0).c_str()) {

								std::vector<SingleEntry_t> first_entries{entry_0};
								RawHypertrieContext<depth, tri> context{std::allocator<std::byte>()};
								NodeContainer<depth, tri> nc{};

								context.insert(nc, first_entries);
								std::cout << fmt::format("result identifier: {}", nc.identifier()) << std::endl;

								ValidationRawNodeContext<depth, tri> validation_context_0{std::allocator<std::byte>(), first_entries};
								REQUIRE(context == validation_context_0);

								for (auto key_1_e : iter::combinations_with_replacement(s, depth)) {
									SingleEntry_t entry_1{{}, value_type(1)};
									std::copy_n(key_1_e.begin(), depth, entry_1.key().begin());

									if (entry_0 == entry_1)
										continue;

									SUBCASE("entry_1 {}"_format(entry_1).c_str()) {

										std::vector<SingleEntry_t> second_entries{entry_1};
										std::vector<SingleEntry_t> all_entries{entry_0, entry_1};


										context.insert(nc, second_entries);
										std::cout << fmt::format("result identifier: {}", nc.identifier()) << std::endl;
										ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), all_entries};
										REQUIRE(context == validation_context);
									}
								}
							}
						}
					}

					SUBCASE("1 and 2 entry") {

						using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;

						using key_part_type = typename tri::key_part_type;
						using value_type = typename tri::value_type;

						utils::RawEntryGenerator<depth, tri> gen{};

						std::vector<key_part_type> s{iter::range(key_part_type(1), key_part_type(3 + 1)).begin(), iter::range(key_part_type(1), key_part_type(3 + 1)).end()};
						for (auto key_0_e : iter::combinations_with_replacement(s, depth)) {
							SingleEntry_t entry_0{{}, value_type(1)};
							std::copy_n(key_0_e.begin(), depth, entry_0.key().begin());
							SUBCASE("entry_0 {}"_format(entry_0).c_str()) {

								std::vector<SingleEntry_t> first_entries{entry_0};
								RawHypertrieContext<depth, tri> context{std::allocator<std::byte>()};
								NodeContainer<depth, tri> nc{};

								context.insert(nc, first_entries);
								std::cout << fmt::format("result identifier: {}", nc.identifier()) << std::endl;

								ValidationRawNodeContext<depth, tri> validation_context_0{std::allocator<std::byte>(), first_entries};
								REQUIRE(context == validation_context_0);

								for (auto key_1_e : iter::combinations_with_replacement(s, depth)) {
									SingleEntry_t entry_1{{}, value_type(1)};
									std::copy_n(key_1_e.begin(), depth, entry_1.key().begin());

									if (entry_0 == entry_1)
										continue;

									for (auto key_2_e : iter::combinations_with_replacement(s, depth)) {

										SingleEntry_t entry_2{{}, value_type(1)};
										std::copy_n(key_2_e.begin(), depth, entry_2.key().begin());
										if (not (entry_1 < entry_2 and entry_2 != entry_0))
											continue;



										SUBCASE("entry_1&2 {} {}"_format(entry_1, entry_2).c_str()) {

											std::vector<SingleEntry_t> second_entries{entry_1, entry_2};
											std::vector<SingleEntry_t> all_entries{entry_0, entry_1, entry_2};


											context.insert(nc, second_entries);
											std::cout << fmt::format("result identifier: {}", nc.identifier()) << std::endl;
											ValidationRawNodeContext<depth, tri> validation_context{std::allocator<std::byte>(), all_entries};
											REQUIRE(context == validation_context);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		TEST_CASE("entry_generator"){
			using config = bool_cfg<2>;
			constexpr auto depth = config::depth;
			using tri = config::tri;
			utils::EntrySetGenerator<depth, 2, tri, 2> entry_set_generator{};
			for (const auto &item : entry_set_generator)
				fmt::print("{}\n", fmt::join(item, " | "));
		}

//		TEST_CASE_TEMPLATE("write and read", T,
////						    bool_cfg<3>
//						   						   bool_cfg<1>, bool_cfg<2>, bool_cfg<3>,bool_cfg<4>, bool_cfg<5>
////									  						   tagged_bool_cfg<1>, tagged_bool_cfg<2>, tagged_bool_cfg<3>, tagged_bool_cfg<4>, tagged_bool_cfg<5>,
////									  						   long_cfg<1>, long_cfg<2>, long_cfg<3>, long_cfg<4>, long_cfg<5>,
////									  						   double_cfg<1>, double_cfg<2>, double_cfg<3>, double_cfg<4>, double_cfg<5>
//									  //
//		) {
//			write_and_read<T::depth, typename T::tri>();
//		}
	}
};// namespace hypertrie::tests::core::node
