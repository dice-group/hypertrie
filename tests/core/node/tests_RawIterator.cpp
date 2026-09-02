#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <utils/ValidationRawHashDiagonal.hpp>
#include <utils/ValidationRawNodeContext.hpp>
#include <utils/Node_test_configs.hpp>
#include <utils/RawEntryGenerator.hpp>
#include <utils/DumpRawContext.hpp>


#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/iteration/RawIterator.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>
#include <utils/EntrySetGenerator.hpp>

#include <dice/template-library/for.hpp>

namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawIterator") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		template<size_t depth, HypertrieTrait htt_t,
				 size_t no_key_parts,
				 size_t min_no_entries,
				 size_t max_no_entries>
		void iterate() {
			CAPTURE(depth);
			CAPTURE(htt_t{});
			CAPTURE(no_key_parts);
			CAPTURE(min_no_entries);
			CAPTURE(max_no_entries);

			using SingleEntry_t = SingleEntry<depth, htt_t>;
			using key_part_type = typename htt_t::key_part_type;
			static constexpr auto max_key_part = key_part_type(no_key_parts);
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance

			dice::template_library::for_range<min_no_entries, max_no_entries + 1>([&](auto no_entries_0) {
				CAPTURE(no_entries_0);

				utils::EntrySetGenerator<depth, no_entries_0, htt_t, max_key_part> outer_generator{};
				for (const auto &entries_0 : outer_generator) {
					CAPTURE(entries_0);

					NodePtr<depth, htt_t, allocator_type> nc{};
					ValidationRawNodeContext<depth, htt_t, allocator_type> const context{alloc, nc, entries_0};
					INFO("Context to iterate:\n", context);

					std::unordered_set<SingleEntry_t, dice::hash::DiceHash<SingleEntry_t>> const entry_set{entries_0.begin(), entries_0.end()};

					using RawIterator_t = RawIterator<depth, true, htt_t, allocator_type>;

					size_t count = 0;
					for (RawIterator_t iter{nc}; iter; ++iter) {
						CAPTURE(*iter);

						{ // check for proper buffer isolation
							auto before = *iter;
							auto cpy = iter;
							++cpy;

							if (cpy) {
								auto after = *iter;

								CHECK(before == after);
								CHECK(before != *cpy);
								CHECK(entry_set.contains(*cpy));
							}
						}

						CHECK(entry_set.contains(*iter));
						count += 1;
					}
					CHECK(count == entry_set.size());
				}
			});
		}

		TEST_CASE_TEMPLATE("hypertrie depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {
			constexpr size_t no_key_parts = 3;
			constexpr size_t min_no_entries = 0;
			constexpr size_t max_no_entries = 3;
			iterate<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
		}

		TEST_CASE_TEMPLATE("hypertrie depth 2", T,
						   bool_cfg<2>,
						   tagged_bool_cfg<2>,
						   long_cfg<2>,
						   double_cfg<2>) {
			constexpr size_t no_key_parts = 3;
			constexpr size_t min_no_entries = 0;
			constexpr size_t max_no_entries = 4;
			iterate<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
		}

		TEST_CASE_TEMPLATE("hypertrie depth 3", T,
						   bool_cfg<3>,
						   tagged_bool_cfg<3>,
						   long_cfg<3>,
						   double_cfg<3>) {
			{
				constexpr size_t no_key_parts = 3;
				constexpr size_t min_no_entries = 0;
				constexpr size_t max_no_entries = 2;
				iterate<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
			}
			{
				constexpr size_t no_key_parts = 2;
				constexpr size_t min_no_entries = 3;
				constexpr size_t max_no_entries = 3;
				iterate<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 4", T,
						   bool_cfg<4>,
						   tagged_bool_cfg<4>,
						   long_cfg<4>,
						   double_cfg<4>) {
			{
				constexpr size_t no_key_parts = 3;
				constexpr size_t min_no_entries = 0;
				constexpr size_t max_no_entries = 1;
				iterate<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
			}
			{
				constexpr size_t no_key_parts = 2;
				constexpr size_t min_no_entries = 2;
				constexpr size_t max_no_entries = 3;
				iterate<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
			}
		}
	}
};// namespace dice::hypertrie::tests::core::node