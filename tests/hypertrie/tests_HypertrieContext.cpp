#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <utils/RawEntryGenerator.hpp>
#include <utils/DumpRawContext.hpp>

#include <dice/hypertrie.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>

// for testing large value types (> sizeof(void *))
struct Value {
	uint64_t high;
	uint64_t low;

	Value() : high{0}, low{0} {}
	explicit Value(uint64_t val) : high{}, low{val} {}
	Value(uint64_t high, uint64_t low) : high{high}, low{low} {}

	constexpr auto operator<=>(Value const &other) const noexcept = default;
};

template<typename Policy>
struct dice::hash::dice_hash_overload<Policy, Value> {
	static size_t dice_hash(Value const &val) noexcept {
		return Policy::hash_combine({Policy::hash_fundamental(val.low), Policy::hash_fundamental(val.high)});
	}
};

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
			while (join_0_iter != join_0.end()) {
				const auto &[label0, sub_ops0] = *join_0_iter;

				std::vector<const_Hypertrie<htt_t, allocator_type>> sub_pos0_tmp = sub_ops0;
				auto sub_ops0_mvd = std::move(sub_pos0_tmp);

				fmt::print("label a: {}\n", label0);
				for (auto &sub_op : sub_ops0_mvd)
					fmt::print("- {}\n", std::string(sub_op));

				HashJoin join_1(sub_ops0_mvd, {{0}, {0}});
				auto join_1_iter = join_1.begin();
				join_0_iter++;
				while (join_1_iter != join_1.end()) {
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
						while (iter != std::default_sentinel) {
							CHECK(iter->key().size() == 1);
							fmt::print("## ## label c = {}\n", (*iter).key()[0]);
							++iter;
						}
					}
				}
			}
		}

		TEST_CASE("join foreach") {
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
			for (auto const &elem : HashJoin(ops, {{0}, {0}})) {
				(void)elem;
				// only checking if this compiles
			}
		}

		TEST_CASE("write and read a single entry") {
			using htt_t = tagged_bool_Hypertrie_trait;
			Hypertrie<htt_t, allocator_type> hypertrie{3};
			hypertrie.set({{1, 2, 3}}, true);

			auto result = hypertrie[SliceKey<htt_t>{{1, 2, 3}}];
			// NOLINTNEXTLINE
			CHECK(result.to_scalar() == true);
			CHECK(hypertrie[Key<htt_t>{{1, 2, 3}}] == true);
			CHECK(hypertrie[Key<htt_t>{{0, 0, 0}}] == false);

			SUBCASE("Diagonal") {
				HashDiagonal<htt_t, allocator_type> hash_diagonal(hypertrie, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{1, 2}});

				CHECK(not hash_diagonal.find(2));
				CHECK(not hash_diagonal.find(3));
			}

			SUBCASE("Iterator") {
				for (const auto &entry : hypertrie) {
					auto [key, value] = entry.as_tuple();
					fmt::print("{} -> {}\n", fmt::join(key, ", "), value);
				}
			}

			SUBCASE("Add another entry") {
				hypertrie.set({{1, 2, 2}}, true);
				auto slice_0_var = hypertrie[SliceKey<htt_t>{{1, std::nullopt, std::nullopt}}];
				auto slice_0 = slice_0_var;
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
						auto [key, value] = entry.as_tuple();
						fmt::print("{} -> {}\n", fmt::join(key, ", "), value);
					}
				}
			}

			SUBCASE("Hypertrie2") {
				Hypertrie<htt_t, allocator_type> hypertrie2{3};
				hypertrie.set({{1, 2, 3}}, true);
				auto slice_12 = hypertrie[SliceKey<htt_t>{{std::nullopt, 2, 3}}];
				for (const auto &entry : slice_12) {
					auto [key, value] = entry.as_tuple();
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

		TEST_CASE("scalar hypertries") {
			using htt_t = tagged_bool_Hypertrie_trait;

			auto const true_scalar = const_Hypertrie<htt_t, allocator_type>::from_scalar(true);
			CHECK(!true_scalar.empty());
			CHECK(true_scalar.to_scalar() == true);
			CHECK(true_scalar.to_scalar() == true);
			CHECK(true_scalar[SliceKey<htt_t>{}] == true_scalar);
			CHECK(true_scalar.get_cards({}).empty());
			CHECK(std::ranges::distance(true_scalar.begin(), true_scalar.end()) == 1);

			auto const false_scalar = const_Hypertrie<htt_t, allocator_type>::from_scalar(false);
			CHECK(false_scalar.empty());
			CHECK(false_scalar.to_scalar() == false);
			CHECK(false_scalar.to_scalar() == false);
			CHECK(false_scalar[SliceKey<htt_t>{}] == false_scalar);
			CHECK(false_scalar.get_cards({}).empty());
			CHECK(std::ranges::distance(false_scalar.begin(), false_scalar.end()) == 0);
		}

		TEST_CASE("SEN Diagonal edge case") {
			using htt_t = tagged_bool_Hypertrie_trait;
			Hypertrie<htt_t, allocator_type> hyp{3};
			hyp.set({1, 2, 3}, true);

			HashDiagonal<htt_t, allocator_type> diagonal{hyp, internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0}}};
			CHECK(diagonal.find(1));

			auto diag = diagonal.current_diagonal();

			Hypertrie<htt_t, allocator_type> expected{2};
			expected.set({2, 3}, true);
			CHECK(diag == expected);
		}

		TEST_CASE("proxy") {
			SUBCASE("depth 3") {
				using htt_t = default_bool_Hypertrie_trait;

				Hypertrie<htt_t, allocator_type> hyp{3};
				hyp.set({{1, 2, 3}}, true);
				hyp.set({{2, 3, 4}}, true);

				auto proxy = hyp[Key<htt_t>{{1, 2, 3}}];
				CHECK(proxy == true);

				proxy = false;
				CHECK(proxy == false);
				CHECK(hyp[Key<htt_t>{{1, 2, 3}}] == false);
				CHECK(hyp[Key<htt_t>{{2, 3, 4}}] == true);
			}

			SUBCASE("depth 0") {
				using htt_t = default_bool_Hypertrie_trait;

				Hypertrie<htt_t, allocator_type> hyp{0};
				auto proxy = hyp[Key<htt_t>{}];
				CHECK(proxy == false);
				CHECK(hyp.hash() == internal::raw::RawIdentifier<0, htt_t>{}.hash());

				proxy = true;
				CHECK(proxy == true);
				CHECK(hyp[Key<htt_t>{}] == true);
				CHECK(hyp.hash() != internal::raw::RawIdentifier<0, htt_t>{}.hash());

				proxy = false;
				CHECK(proxy == false);
				CHECK(hyp[Key<htt_t>{}] == false);
				CHECK(hyp.hash() == internal::raw::RawIdentifier<0, htt_t>{}.hash());
			}
		}

		TEST_CASE("prevent corruption through duplication") {
			using htt_t = default_bool_Hypertrie_trait;

			std::vector<internal::raw::SingleEntry<3, htt_t>> entries{
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 1}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 2}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 3}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 4}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 5}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 6}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 7}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 8}}};

			Hypertrie<htt_t, allocator_type> hyp{3};

			{ // prepare hypertrie
				AsyncBulkInserter<htt_t, allocator_type> inserter{hyp};
				for (auto const &e : entries) {
					inserter.add(e);
				}
			}

			{
				// bulk size = 2 => dedup table max size = 8
				AsyncBulkInserter<htt_t, allocator_type> inserter{hyp, 2};

				// insert one new entry
				inserter.add(internal::raw::SingleEntry<3, htt_t>{{1, 1, 99}});

				// insert 8 already known entries => forces deduplication table clear, without actually performing an insert
				for (auto const &e : entries) {
					inserter.add(e);
				}

				// insert the one entry from before again, which will now not be deduplicated as the dedup table was not cleared
				// and no inserted happened in the meantime
				inserter.add(internal::raw::SingleEntry<3, htt_t>{{1, 1, 99}});
			}

			// if this is buggy, it will trigger an assertion in the insertion algorithm
		}

		TEST_CASE("get_cards sanity check") {
			using htt_t = default_bool_Hypertrie_trait;

			SUBCASE("SEN") {
				Hypertrie<htt_t, allocator_type> hyp{3};
				hyp.set({1, 2, 3}, true);

				CHECK(hyp.get_cards({0, 2, 1}) == std::vector<size_t>{1, 1, 1});
			}

			SUBCASE("FN") {
				Hypertrie<htt_t, allocator_type> hyp{3};
				hyp.set({1, 2, 3}, true);
				hyp.set({3, 2, 1}, true);
				hyp.set({1, 4, 4}, true);

				CHECK(hyp.get_cards({1, 0, 2}) == std::vector<size_t>{2, 2, 3});
			}

			SUBCASE("XN") {
				Hypertrie<htt_t, allocator_type> hyp{3};
				hyp.set({1, 2, 3}, true);
				hyp.set({3, 2, 3}, true);
				hyp.set({1, 4, 3}, true);

				CHECK(hyp.get_cards({1, 0, 2}) == std::vector<size_t>{2, 2, 1});
			}
		}

		TEST_CASE("large value types") {
			using allocator_type = std::allocator<std::byte>;

			SUBCASE("non-tagged") {
				using htt_t = Hypertrie_trait<unsigned long, Value>;

				SUBCASE("depth 3") {
					Hypertrie<htt_t, allocator_type> hyp{3};
					hyp.set({1, 2, 3}, Value{123, 456});
					hyp.set({1, 4, 5}, Value{789, 654});

					CHECK_EQ(hyp[{1, 2, 3}], Value{123, 456});
					CHECK_EQ(hyp[{1, 4, 5}], Value{789, 654});
					CHECK_EQ(hyp[{9, 9, 9}], Value{});
				};

				SUBCASE("depth 0") {
					Hypertrie<htt_t, allocator_type> hyp{0};
					hyp.set({}, Value{0, 1});

					CHECK_EQ(hyp[{}], Value{0, 1});
				}
			}
		}

		TEST_CASE("prevent corruption through duplication") {
			using htt_t = default_bool_Hypertrie_trait;

			std::vector<internal::raw::SingleEntry<3, htt_t>> entries{
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 1}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 2}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 3}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 4}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 5}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 6}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 7}},
					internal::raw::SingleEntry<3, htt_t>{{1, 1, 8}}};

			Hypertrie<htt_t, allocator_type> hyp{3};

			{ // prepare hypertrie
				AsyncBulkInserter<htt_t, allocator_type> inserter{hyp};
				for (auto const &e : entries) {
					inserter.add(e);
				}
			}

			{
				// bulk size = 2 => dedup table max size = 8
				AsyncBulkInserter<htt_t, allocator_type> inserter{hyp, 2};

				// insert one new entry
				inserter.add(internal::raw::SingleEntry<3, htt_t>{{1, 1, 99}});

				// insert 8 already known entries => forces deduplication table clear, without actually performing an insert
				for (auto const &e : entries) {
					inserter.add(e);
				}

				// insert the one entry from before again, which will now not be deduplicated as the dedup table was not cleared
				// and no inserted happened in the meantime
				inserter.add(internal::raw::SingleEntry<3, htt_t>{{1, 1, 99}});
			}

			// if this is buggy, it will trigger an assertion in the insertion algorithm
		}
	};

	TEST_CASE_TEMPLATE("Valued bulk insertion sanity check", BI,
					   AsyncBulkInserter<default_long_Hypertrie_trait, std::allocator<std::byte>>,
					   SyncBulkInserter<default_long_Hypertrie_trait, std::allocator<std::byte>>) {
		using htt_t = default_long_Hypertrie_trait;
		Hypertrie<htt_t, std::allocator<std::byte>> hyp{3};

		{
			BI inserter{hyp};

			std::vector<internal::raw::SingleEntry<3, htt_t>> const entries{
					internal::raw::SingleEntry<3, htt_t>{{1, 2, 3}, 1},
					internal::raw::SingleEntry<3, htt_t>{{4, 5, 6}, 1},
					internal::raw::SingleEntry<3, htt_t>{{1, 2, 3}, 2},// should be ignored
					internal::raw::SingleEntry<3, htt_t>{{4, 5, 6}, 1},// should also be ignored
			};

			for (auto const &e : entries) {
				inserter.add(e);
			}

			CHECK_THROWS(inserter.add(internal::raw::SingleEntry<3, htt_t>{{9, 10, 11}, 0}));
		}

		CHECK(hyp.size() == 2);
		CHECK(hyp[Key<htt_t>{1, 2, 3}] == 1);
		CHECK(hyp[Key<htt_t>{4, 5, 6}] == 1);
	}

	TEST_CASE_TEMPLATE("Valued bulk removal sanity check", BR,
					   AsyncBulkRemover<default_long_Hypertrie_trait, std::allocator<std::byte>>,
					   SyncBulkRemover<default_long_Hypertrie_trait, std::allocator<std::byte>>) {
		using htt_t = default_long_Hypertrie_trait;
		Hypertrie<htt_t, std::allocator<std::byte>> hyp{3};
		hyp.set(Key<htt_t>{1, 2, 3}, 1);
		hyp.set(Key<htt_t>{4, 5, 6}, 1);

		{
			BR remover{hyp};

			std::vector<internal::raw::RawKey<3, htt_t>> const entries{
					internal::raw::RawKey<3, htt_t>{1, 2, 3},
					internal::raw::RawKey<3, htt_t>{4, 5, 6},
					internal::raw::RawKey<3, htt_t>{7, 8, 9}// should be ignored
			};

			for (auto const &e : entries) {
				remover.add(e);
			}
		}

		CHECK(hyp.size() == 0);
	}

	TEST_CASE("hypertries in unordered set") {
		using htt_t = default_long_Hypertrie_trait;
		using allocator_type = std::allocator<std::byte>;

		std::vector<internal::raw::SingleEntry<3, htt_t>> const entries{internal::raw::SingleEntry<3, htt_t>{{1, 2, 1}, true},
																		internal::raw::SingleEntry<3, htt_t>{{1, 2, 2}, true},
																		internal::raw::SingleEntry<3, htt_t>{{1, 3, 1}, true}};

		Hypertrie<htt_t, allocator_type> hyp{3};
		for (auto const &e : entries) {
			hyp.set(e.key(), e.value());
		}

		dump_context(hyp.context()->raw_context());

		auto xn_slice_1_1 = hyp[SliceKey<htt_t>{std::nullopt, 2, std::nullopt}];
		auto xn_slice_1_2 = hyp[SliceKey<htt_t>{std::nullopt, 2, std::nullopt}];
		CHECK_EQ(xn_slice_1_1.hash(), xn_slice_1_2.hash());

		auto xn_slice_2 = hyp[SliceKey<htt_t>{1, std::nullopt, std::nullopt}];

		auto sen_slice_1_1 = hyp[SliceKey<htt_t>{std::nullopt, std::nullopt, 1}];
		auto sen_slice_1_2 = hyp[SliceKey<htt_t>{std::nullopt, std::nullopt, 1}];

		SUBCASE("direct") {
			std::unordered_set<const_Hypertrie<htt_t, allocator_type>> operands{xn_slice_1_1, xn_slice_1_2,
																				xn_slice_2,
																				sen_slice_1_1, sen_slice_1_2};
			CHECK_EQ(operands.size(), 3);
		}

		SUBCASE("reference wrapper") {
			struct FwdHash {
				size_t operator()(std::reference_wrapper<const_Hypertrie<htt_t, allocator_type> const> const hyp) const noexcept {
					return hyp.get().hash();
				}
			};

			struct FwdEq {
				bool operator()(std::reference_wrapper<const_Hypertrie<htt_t, allocator_type> const> const lhs,
								std::reference_wrapper<const_Hypertrie<htt_t, allocator_type> const> const rhs) const noexcept {
					return lhs.get() == rhs.get();
				}
			};

			std::unordered_set<std::reference_wrapper<const_Hypertrie<htt_t, allocator_type>>, FwdHash, FwdEq> operands{xn_slice_1_1, xn_slice_1_2,
																														xn_slice_2,
																														sen_slice_1_1, sen_slice_1_2};
			CHECK_EQ(operands.size(), 3);
		}
	}

	TEST_CASE("Hypertrie::extend_from_iter") {
		using hyp_htt_t = default_double_Hypertrie_trait;
		using iter_htt_t = default_long_Hypertrie_trait;
		using allocator_type = std::allocator<std::byte>;

		// turns some non-null values into null values
		auto cfunc = [](auto v) { return static_cast<double>(v - 1); };

		SUBCASE("depth 0") {
			SUBCASE("SingleEntry") {
				std::vector<internal::raw::SingleEntry<0, iter_htt_t>> entries{
						internal::raw::SingleEntry<0, iter_htt_t>{{}, 5},  // final value
						internal::raw::SingleEntry<0, iter_htt_t>{{}, 1},  // is ignored: zero-value
						internal::raw::SingleEntry<0, iter_htt_t>{{}, 0}}; // is ignored: repeat of key

				Hypertrie<hyp_htt_t, allocator_type> hyp{0};
				hyp.extend_from_iter(entries.begin(), entries.end(), cfunc);

				CHECK_EQ(hyp[Key<hyp_htt_t>{}].get(), 4);

				Hypertrie<hyp_htt_t, allocator_type> hyp2{0};
				hyp2.extend(hyp);
				CHECK_EQ(hyp2, hyp);
			}

			SUBCASE("Entry") {
				std::vector<Entry<iter_htt_t>> entries{
						Entry<iter_htt_t>{{}, 43},  // final value
						Entry<iter_htt_t>{{}, -12}, // is ignored: repeat of key
						Entry<iter_htt_t>{{}, 1}};  // is ignored: zero value

				Hypertrie<hyp_htt_t, allocator_type> hyp{0};
				hyp.extend_from_iter(entries.begin(), entries.end(), cfunc);

				CHECK_EQ(hyp[Key<hyp_htt_t>{}].get(), 42.0);

				Hypertrie<hyp_htt_t, allocator_type> hyp2{0};
				hyp2.extend(hyp);
				CHECK_EQ(hyp2, hyp);
			}

			SUBCASE("NonZeroEntry") {
				CHECK_THROWS(NonZeroEntry<iter_htt_t>{{}, 0});

				std::vector<NonZeroEntry<iter_htt_t>> entries{
						NonZeroEntry<iter_htt_t>{{}, 43}, // final value
						NonZeroEntry<iter_htt_t>{{}, 1}}; // is ignored: zero value

				Hypertrie<hyp_htt_t, allocator_type> hyp{0};
				hyp.extend_from_iter(entries.begin(), entries.end(), cfunc);

				CHECK_EQ(hyp[Key<hyp_htt_t>{}].get(), 42.0);

				Hypertrie<hyp_htt_t, allocator_type> hyp2{0};
				hyp2.extend(hyp);
				CHECK_EQ(hyp2, hyp);
			}
		}


		SUBCASE("depth 3") {
			SUBCASE("SingleEntry") {
				std::vector<internal::raw::SingleEntry<3, iter_htt_t>> entries{
						internal::raw::SingleEntry<3, iter_htt_t>{{1, 2, 3}, 5},  // final value of 1,2,3
						internal::raw::SingleEntry<3, iter_htt_t>{{4, 5, 6}, 12}, // final value of 4,5,6
						internal::raw::SingleEntry<3, iter_htt_t>{{4, 5, 6}, 1},  // is ignored: zero-value
						internal::raw::SingleEntry<3, iter_htt_t>{{1, 2, 3}, 0},  // is ignored: repeat of 1,2,3
						internal::raw::SingleEntry<3, iter_htt_t>{{9, 9, 1}, 2}}; // final value of 9, 9, 1

				Hypertrie<hyp_htt_t, allocator_type> hyp{3};
				hyp.extend_from_iter(entries.begin(), entries.end(), cfunc);

				CHECK_EQ(hyp[Key<hyp_htt_t>{1, 2, 3}].get(), 4.0);
				CHECK_EQ(hyp[Key<hyp_htt_t>{4, 5, 6}].get(), 11.0);
				CHECK_EQ(hyp[Key<hyp_htt_t>{9, 9, 1}].get(), 1.0);

				Hypertrie<hyp_htt_t, allocator_type> hyp2{3};
				hyp2.extend(hyp);
				CHECK_EQ(hyp2, hyp);
			}

			SUBCASE("Entry") {
				std::vector<Entry<iter_htt_t>> entries{
						Entry<iter_htt_t>{{1, 2, 3}, 5},  // final value of 1,2,3
						Entry<iter_htt_t>{{4, 5, 6}, 12}, // final value of 4,5,6
						Entry<iter_htt_t>{{4, 5, 6}, 1},  // is ignored: zero value
						Entry<iter_htt_t>{{1, 2, 3}, 0},  // is ignored: repeat of 1,2,3
						Entry<iter_htt_t>{{9, 9, 1}, 2}}; // final value of 9,9,1

				Hypertrie<hyp_htt_t, allocator_type> hyp{3};
				hyp.extend_from_iter(entries.begin(), entries.end(), cfunc);

				CHECK_EQ(hyp[Key<hyp_htt_t>{1, 2, 3}].get(), 4.0);
				CHECK_EQ(hyp[Key<hyp_htt_t>{4, 5, 6}].get(), 11.0);
				CHECK_EQ(hyp[Key<hyp_htt_t>{9, 9, 1}].get(), 1.0);

				Hypertrie<hyp_htt_t, allocator_type> hyp2{3};
				hyp2.extend(hyp);
				CHECK_EQ(hyp2, hyp);
			}

			SUBCASE("NonZeroEntry") {
				CHECK_THROWS(NonZeroEntry<iter_htt_t>{{4, 5, 6}, 0});

				std::vector<NonZeroEntry<iter_htt_t>> entries{
						NonZeroEntry<iter_htt_t>{{1, 2, 3}, 5},  // final value of 1,2,3
						NonZeroEntry<iter_htt_t>{{4, 5, 6}, 12}, // final value of 4,5,6
						NonZeroEntry<iter_htt_t>{{1, 2, 3}, 1},  // is ignored: zero value
						NonZeroEntry<iter_htt_t>{{9, 9, 1}, 2}}; // final value of 9,9,1

				Hypertrie<hyp_htt_t, allocator_type> hyp{3};

				// testing for case where convert func turns non-null-value into null-value
				hyp.extend_from_iter(entries.begin(), entries.end(), cfunc);

				CHECK_EQ(hyp[Key<hyp_htt_t>{1, 2, 3}].get(), 4.0);
				CHECK_EQ(hyp[Key<hyp_htt_t>{4, 5, 6}].get(), 11.0);
				CHECK_EQ(hyp[Key<hyp_htt_t>{9, 9, 1}].get(), 1.0);

				Hypertrie<hyp_htt_t, allocator_type> hyp2{3};
				hyp2.extend(hyp);
				CHECK_EQ(hyp2, hyp);
			}
		}
	}

};// namespace dice::hypertrie::tests::core::node