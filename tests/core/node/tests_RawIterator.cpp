#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <itertools.hpp>

#include "ValidationRawHashDiagonal.hpp"
#include <Node_test_configs.hpp>
#include <RawEntryGenerator.hpp>


#include <Dice/hypertrie/internal/raw/fmt_Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/iteration/RawIterator.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <Dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <Dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>
#include <EntrySetGenerator.hpp>

namespace hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawIterator") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		template<size_t depth, HypertrieCoreTrait tri,
				 size_t no_key_parts,
				 size_t min_no_entries,
				 size_t max_no_entries>
		void iterate() {
			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;

			using key_part_type = typename tri::key_part_type;

			static constexpr auto max_key_part = key_part_type(no_key_parts);


			boost::hana::for_each(boost::hana::range_c<size_t, min_no_entries, max_no_entries + 1>, [&](auto no_entries_0) {
				SUBCASE("{} entries"_format(no_entries_0).c_str()) {
					utils::EntrySetGenerator<depth, no_entries_0, tri, max_key_part> outer_generator{};
					for (const auto &entries_0 : outer_generator) {
						SUBCASE("entries: {}"_format(fmt::join(entries_0, " | ")).c_str()) {


							RawHypertrieContext<depth, tri> context{std::allocator<std::byte>()};
							NodeContainer<depth, tri> nc{};

							std::unordered_set<SingleEntry_t, Dice::hash::DiceHash<SingleEntry_t>> entry_set{entries_0.begin(), entries_0.end()};

							context.insert(nc, entries_0);

							RawIterator<depth, tri, depth> raw_iterator{nc, context};

							size_t count = 0;
							while (not raw_iterator.ended()) {
								CHECK(entry_set.contains(raw_iterator.value()));
								raw_iterator.inc();
								count++;
							}
							CHECK(count == entry_set.size());
						}
					}
				}
			});
		}

		TEST_CASE_TEMPLATE("hypertrie depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {
			SUBCASE("{}"_format(typename T::tri{}).c_str()) {
				constexpr size_t no_key_parts = 3;
				constexpr size_t min_no_entries = 0;
				constexpr size_t max_no_entries = 3;
				iterate<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 2", T,
						   bool_cfg<2>,
						   tagged_bool_cfg<2>,
						   long_cfg<2>,
						   double_cfg<2>) {
			SUBCASE("{}"_format(typename T::tri{}).c_str()) {
				constexpr size_t no_key_parts = 3;
				constexpr size_t min_no_entries = 0;
				constexpr size_t max_no_entries = 4;
				iterate<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 3", T,
						   bool_cfg<3>,
						   tagged_bool_cfg<3>,
						   long_cfg<3>,
						   double_cfg<3>) {
			SUBCASE("{}"_format(typename T::tri{}).c_str()) {
				{
					constexpr size_t no_key_parts = 3;
					constexpr size_t min_no_entries = 0;
					constexpr size_t max_no_entries = 2;
					iterate<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
				}
				{
					constexpr size_t no_key_parts = 2;
					constexpr size_t min_no_entries = 3;
					constexpr size_t max_no_entries = 3;
					iterate<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
				}
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 4", T,
						   bool_cfg<4>,
						   tagged_bool_cfg<4>,
						   long_cfg<4>,
						   double_cfg<4>) {
			SUBCASE("{}"_format(typename T::tri{}).c_str()) {
				{
					constexpr size_t no_key_parts = 3;
					constexpr size_t min_no_entries = 0;
					constexpr size_t max_no_entries = 1;
					iterate<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
				}
				{
					constexpr size_t no_key_parts = 2;
					constexpr size_t min_no_entries = 2;
					constexpr size_t max_no_entries = 3;
					iterate<T::depth, typename T::tri, no_key_parts, min_no_entries, max_no_entries>();
				}
			}
		}
	}
};// namespace hypertrie::tests::core::node