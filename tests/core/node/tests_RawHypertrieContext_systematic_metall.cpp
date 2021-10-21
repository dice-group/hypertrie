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

#include <metall/metall.hpp>
//where do we include this??
#include <tsl/boost_offset_pointer.h>

namespace hypertrie::tests::core::node {

	using metall_bool_Hypertrie_trait = Hypertrie_trait<unsigned long,
													 bool,
													 metall::manager::allocator_type<std::byte>,
													 hypertrie::internal::container::tsl_sparse_map,
													 hypertrie::internal::container::tsl_sparse_set>;
	template<size_t depth>
	using metall_bool_cfg = Node_test_config<depth, metall_bool_Hypertrie_trait>;


	TEST_SUITE("systematic testing of RawNodeContext with the metall allocator") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		template<size_t depth, HypertrieCoreTrait tri,
				 size_t no_key_parts,
				 size_t min_no_entries,
				 size_t max_no_entries>
		void metall_write_and_read() {
			using stl_alloc_trait = tri_with_stl_alloc<tri>;
			using SingleEntry_t = SingleEntry<depth, stl_alloc_trait>;
			using key_part_type = typename tri::key_part_type;
			static constexpr auto max_key_part = key_part_type(no_key_parts);
			const std::string path = "/tmp";
			const std::string contextName = "someNameForTheObjectCreatedByMetall1";
			const std::string containerName = "someNameForTheObjectCreatedByMetall2";

			//create segment
			{
				metall::manager manager(metall::create_only, path.c_str());
			}
			//create allocator and manager to use
			metall::manager manager(metall::open_only, path.c_str());
			auto alloc = manager.get_allocator();

			auto generateConAndNC = [&manager, &alloc, &contextName, &containerName]() {
				auto &context = *manager.construct<RawHypertrieContext<depth, tri>>(contextName.c_str())(alloc);
				auto &nc = *manager.construct<NodeContainer<depth, tri>>(containerName.c_str())();
				return std::make_tuple(context, nc);
			};
			auto deleteConAndNC = [&manager, &contextName, &containerName]() {
				manager.destroy<RawHypertrieContext<depth, tri>>(contextName.c_str());
				manager.destroy<NodeContainer<depth, tri>>(containerName.c_str());
			};

			SUBCASE("{}"_format(tri{}).c_str()) {
				SUBCASE("hypertrie depth = {}"_format(depth).c_str()) {
					boost::hana::for_each(boost::hana::range_c<size_t, min_no_entries, max_no_entries + 1>, [&](auto no_entries_0) {
						boost::hana::for_each(boost::hana::range_c<size_t, min_no_entries, max_no_entries + 1>, [&](auto no_entries_1) {
							SUBCASE("first {} entries, then {} entries"_format(no_entries_0, no_entries_1).c_str()) {
								utils::EntrySetGenerator<depth, no_entries_0, stl_alloc_trait, max_key_part> outer_generator{};
								for (const auto &entries_0 : outer_generator) {
									SUBCASE("first_entries: {}"_format(fmt::join(entries_0, " | ")).c_str()) {
										auto [context, nc] = generateConAndNC();
										context.insert(nc, entries_0);
										ValidationRawNodeContext<depth, stl_alloc_trait> validation_context_0{std::allocator<std::byte>(), entries_0};
										CHECK(context == validation_context_0);
										std::cout << fmt::format("result identifier: {}", nc.raw_identifier()) << std::endl;

										utils::EntrySetGenerator_with_exclude<depth, no_entries_1, stl_alloc_trait, max_key_part> inner_generator{entries_0};
										for (const auto &entries_1 : inner_generator) {
											SUBCASE("second_entries: {}"_format(fmt::join(entries_1, " | ")).c_str()) {
												std::vector<SingleEntry_t> all_entries = entries_0;
												all_entries.insert(all_entries.end(), entries_1.begin(), entries_1.end());
												context.insert(nc, entries_1);
												std::cout << fmt::format("result identifier: {}", nc.raw_identifier()) << std::endl;
												ValidationRawNodeContext<depth, stl_alloc_trait> validation_context{std::allocator<std::byte>(), all_entries};
												CHECK(context == validation_context);
											}
										}

										deleteConAndNC();
									}
								}
							}
						});
					});
				}
			}
		}

		TEST_CASE("entry_generator") {
			using config = bool_cfg<2>;
			constexpr auto depth = config::depth;
			using tri = config::tri;
			utils::EntrySetGenerator<depth, 1, tri, 2> entry_set_generator{};
			for (const auto &item : entry_set_generator) {
				fmt::print("outer: {}\n", fmt::join(item, " | "));
				utils::EntrySetGenerator_with_exclude<depth, 1, tri, 2> inner_generator{item};
				for (const auto &inner_item : inner_generator) {
					fmt::print("  inner: {}\n", fmt::join(inner_item, " | "));
				}
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 1", T,
						   metall_bool_cfg<1>, metall_bool_cfg<2>, metall_bool_cfg<3>){
			constexpr size_t no_key_parts = 3;
			constexpr size_t min_no_entries = 1;
			constexpr size_t max_no_entries = 3;
			metall_write_and_read<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
		}
	}
}