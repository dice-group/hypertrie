#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <utils/RawEntryGenerator.hpp>

#include <dice/hypertrie.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>


namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("Testing of HypertrieContext") {
		using namespace ::dice::hypertrie;
		using allocator_type = std::allocator<std::byte>;
		allocator_type alloc{};// allocator instance

		TEST_CASE("simulate subscript") {
			using htt_t = tagged_bool_Hypertrie_trait;
			//				"abc,ab->c",
			Hypertrie<htt_t, allocator_type> op_0(3);
			op_0.set({{1, 1, 1}}, true);
			op_0.set({{2, 1, 1}}, true);
			op_0.set({{1, 2, 2}}, true);

			Hypertrie<htt_t, allocator_type> op_1(2);
			op_1.set({{2, 1}}, true);
			op_1.set({{2, 2}}, true);

			std::vector<const_Hypertrie<htt_t, allocator_type>> ops{op_0, op_1};
			HashJoin join_0(ops, {{0}, {0}});
			auto join_0_iter = join_0.begin();
			while (join_0_iter) {
				const auto &[label0, sub_ops0] = *join_0_iter;

				std::vector<const_Hypertrie<htt_t, allocator_type>> sub_pos0_tmp = sub_ops0;
				auto sub_ops0_mvd = std::move(sub_pos0_tmp);

				fmt::print("label a: {}\n", label0);
				for (auto &sub_op : sub_ops0_mvd)
					fmt::print("- {}\n", std::string(sub_op));

				HashJoin join_1(sub_ops0_mvd, {{0}, {0}});
				auto join_1_iter = join_1.begin();
				join_0_iter++;
				while (join_1_iter) {
					const auto &[label1, sub_ops1] = *join_1_iter;
					std::vector<const_Hypertrie<htt_t, allocator_type>> sub_pos1_tmp = sub_ops1;
					auto sub_ops1_mvd = std::move(sub_pos1_tmp);

					++join_1_iter;
					fmt::print("## label b: {}\n", label1);
					for (auto &sub_op : sub_ops1_mvd)
						fmt::print("## - {}\n", std::string(sub_op));

					{
						CHECK(sub_ops1_mvd.size() == 1);
						auto iter = sub_ops1_mvd[0].cbegin();
						while (iter) {
							CHECK(iter->key().size() == 1);
							fmt::print("## ## label c = {}\n", (*iter).key()[0]);
							++iter;
						}
					}
				}
			}
		}

		TEST_CASE("write and read a single entry") {
			using htt_t = tagged_bool_Hypertrie_trait;
			Hypertrie<htt_t, allocator_type> hypertrie{3};
			hypertrie.set({{1, 2, 3}}, true);

			auto result = hypertrie[SliceKey<htt_t>{{1, 2, 3}}];
			// NOLINTNEXTLINE
			CHECK(std::get<bool>(result) == true);
			CHECK(hypertrie[Key<htt_t>{{1, 2, 3}}] == true);
			CHECK(hypertrie[Key<htt_t>{{0, 0, 0}}] == false);

			SUBCASE("Diagonal") {
				HashDiagonal<htt_t, allocator_type> hash_diagonal(hypertrie, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{1, 2}});

				CHECK(not hash_diagonal.find(2));
				CHECK(not hash_diagonal.find(3));
			}

			SUBCASE("Iterator") {
				for (const auto &entry : hypertrie) {
					auto [key, value] = entry.tuple();
					fmt::print("{} -> {}\n", fmt::join(key, ", "), value);
				}
			}

			SUBCASE("Add another entry") {
				hypertrie.set({{1, 2, 2}}, true);
				auto slice_0_var = hypertrie[SliceKey<htt_t>{{1, std::nullopt, std::nullopt}}];
				auto slice_0 = std::get<0>(slice_0_var);
				CHECK(slice_0[Key<htt_t>{{2, 2}}] == true);
				CHECK(slice_0[Key<htt_t>{{2, 3}}] == true);
				CHECK(slice_0[Key<htt_t>{{3, 2}}] == false);
				SUBCASE("Diagonal") {
					HashDiagonal<htt_t, allocator_type> hash_diagonal(slice_0, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0, 1}});

					CHECK(hash_diagonal.find(2));
					CHECK(not hash_diagonal.find(3));
				}

				SUBCASE("Iterator") {
					for (const auto &entry : slice_0) {
						auto [key, value] = entry.tuple();
						fmt::print("{} -> {}\n", fmt::join(key, ", "), value);
					}
				}
			}

			SUBCASE("Hypertrie2") {
				Hypertrie<htt_t, allocator_type> hypertrie2{3};
				hypertrie.set({{1, 2, 3}}, true);
				auto slice_12 = std::get<0>(hypertrie[SliceKey<htt_t>{{std::nullopt, 2, 3}}]);
				for (const auto &entry : slice_12) {
					auto [key, value] = entry.tuple();
					fmt::print("{} -> {}\n", fmt::join(key, ", "), value);
				}

				auto use_bulk_inserter = [&]<BulkUpdaterSyncness syncness>() {
					{
						BulkInserter<htt_t, allocator_type, syncness> bi{
								hypertrie2, 10, []([[maybe_unused]] size_t processed_entries,//
												   [[maybe_unused]] size_t inserted_entries, //
												   [[maybe_unused]] size_t hypertrie_size_after) noexcept {
									fmt::print("processed_entries {}\n"
											   "inserted_entries {}\n"
											   "hypertrie_size_after {}\n",
											   processed_entries,
											   inserted_entries,
											   hypertrie_size_after);
								}//
						};
						bi.add(NonZeroEntry<htt_t>{{1, 2, 3}});
						bi.add(NonZeroEntry<htt_t>{{1, 2, 3}});
						bi.add(NonZeroEntry<htt_t>{{1, 4, 3}});
						bi.add(NonZeroEntry<htt_t>{{3, 2, 3}});
					}

					CHECK(hypertrie2.size() == 3);
					CHECK(hypertrie2[Key<htt_t>{{1, 2, 3}}] == true);
					CHECK(hypertrie2[Key<htt_t>{{1, 4, 3}}] == true);
					CHECK(hypertrie2[Key<htt_t>{{3, 2, 3}}] == true);

					CHECK(hypertrie2[Key<htt_t>{{3, 4, 3}}] == false);
				};

				SUBCASE("async Bulkinserter") {
					use_bulk_inserter.operator()<BulkUpdaterSyncness::Async>();
				}

				SUBCASE("sync Bulkinserter") {
					use_bulk_inserter.operator()<BulkUpdaterSyncness::Sync>();
				}
			}
		};

		TEST_CASE("read write remove one entry") {
			using htt_t = tagged_bool_Hypertrie_trait;

			Hypertrie<htt_t, allocator_type> hyp{3};
			CHECK(hyp.size() == 0);

			hyp.set({1, 2, 3}, true);
			CHECK(hyp.size() == 1);

			hyp.set({1, 2, 3}, false);
			CHECK(hyp.size() == 0);
		}
	};
};// namespace dice::hypertrie::tests::core::node