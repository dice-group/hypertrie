#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <utils/EntrySetGenerator.hpp>
#include <utils/Node_test_configs.hpp>
#include <utils/RawEntryGenerator.hpp>
#include <utils/ValidationRawNodeContext_slice.hpp>
#include <utils/ValidationRawNodeContext.hpp>
#include <utils/DumpRawContext.hpp>

#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/fmt_RawKey.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_CartesianNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>


#include <dice/template-library/for.hpp>

namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawNodeContext") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		// TODO: check handling all all positions fixed
		// TODO: add handling of no positions fixed
		// TODO: test for tri_with_stl_alloc for SingleEntryNodes

		template<size_t depth, HypertrieTrait htt_t,
				 size_t no_key_parts,
				 size_t no_entries>
		void slice(size_t max = std::numeric_limits<size_t>::max()) {
			CAPTURE(depth);
			CAPTURE(htt_t{});
			CAPTURE(no_key_parts);
			CAPTURE(no_entries);

			using key_part_type = typename htt_t::key_part_type;
			using value_type = typename htt_t::value_type;

			utils::RawEntryGenerator<depth, htt_t> gen{};

			static constexpr key_part_type min_key_part = 1;

			static constexpr key_part_type max_key_part = 1 + no_key_parts;

			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance


			gen.setKeyPartMinMax(key_part_type(1), key_part_type(2));
			gen.setValueMinMax(true, true);

			utils::EntrySetGenerator<depth, no_entries, htt_t, max_key_part, min_key_part> outer_generator{};
			for (const auto &entries : outer_generator) {
				CAPTURE(entries);

				NodePtr<depth, htt_t, allocator_type> nodec;
				ValidationRawNodeContext<depth, htt_t, allocator_type> const context{std::allocator<std::byte>{}, nodec, entries};
				INFO("Context to slice:\n", context);

				dice::template_library::for_range<0UL, depth>(
						[&](/** the fixed depth of slices */ auto fixed_depth) {
							for (/** the positions where the raw_slice_key has <div>key_part</div> */ auto positions : iter::combinations(iter::range(depth), fixed_depth)) {
								static constexpr size_t result_depth = depth - fixed_depth;
								for (auto key_parts : iter::combinations_with_replacement(iter::range(min_key_part, max_key_part + 1), fixed_depth)) {
									RawSliceKey<fixed_depth, htt_t> raw_slice_key;
									for (const auto [i, pos_and_key_part] : iter::enumerate(iter::zip(positions, key_parts))) {
										const auto &[pos, key_part] = pos_and_key_part;
										raw_slice_key[i].pos = pos;
										raw_slice_key[i].key_part = key_part;
									}
									CAPTURE(raw_slice_key);

									auto expected_entries_v = slice_entries(entries, raw_slice_key);
									std::unordered_map<RawKey<result_depth, htt_t>, value_type, dice::hash::DiceHashMartinus<RawKey<result_depth, htt_t>>> expected_entries;
									for (const auto expected_entry : expected_entries_v) {
										expected_entries[expected_entry.key()] = expected_entry.value();
									}

									for (const auto &entry : utils::SingleEntryGenerator<result_depth, htt_t, max_key_part, 0>()) {
										INFO(entry.key(), " -> ", expected_entries.contains(entry.key()) ? expected_entries[entry.key()] : value_type{});
										auto slice = context.slice(nodec, raw_slice_key);

										if constexpr (fixed_depth == depth) {
											if (expected_entries.contains(entry.key()))
												CHECK(slice == expected_entries[entry.key()]);
											else
												CHECK(slice == value_type{});
										} else {
											NodePtr<result_depth, htt_t, allocator_type> slice_instance = slice.as_node_ptr();

											if (expected_entries.contains(entry.key())) {
												CHECK(not slice.empty());
												CHECK(context.template get(slice_instance, entry.key()) == expected_entries[entry.key()]);
											} else {
												if (slice.empty()) {
													CHECK_MESSAGE(true, "is empty");
												} else {
													CHECK(context.template get(slice_instance, entry.key()) == value_type{});
												}
											}
										}
									}
								}
							}
						});

				if (--max == 0)
					break;
			}
		}

		TEST_CASE_TEMPLATE("slice depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {
			constexpr size_t no_key_parts = 3;
			dice::template_library::for_range<0UL, 3UL>(
					[&](auto no_entries) {
						slice<T::depth, typename T::htt_t, no_key_parts, no_entries>();
					});
		}

		TEST_CASE_TEMPLATE("slice depth 2", T,
						   bool_cfg<2>,
						   tagged_bool_cfg<2>,
						   long_cfg<2>,
						   double_cfg<2>) {
			constexpr size_t no_key_parts = 3;
			dice::template_library::for_range<0UL, 4UL>(
					[&](auto no_entries) {
						slice<T::depth, typename T::htt_t, no_key_parts, no_entries>();
					});
		}

		TEST_CASE_TEMPLATE("slice depth 3", T,
						   bool_cfg<3>,
						   tagged_bool_cfg<3>,
						   long_cfg<3>,
						   double_cfg<3>) {
			constexpr size_t no_key_parts = 2;
			dice::template_library::for_range<0UL, 3UL>(
					[&](auto no_entries) {
						slice<T::depth, typename T::htt_t, no_key_parts, no_entries>();
					});
		}

		TEST_CASE_TEMPLATE("slice depth 4", T,
						   bool_cfg<4>,
						   tagged_bool_cfg<4>,
						   long_cfg<4>,
						   double_cfg<4>) {
			constexpr size_t no_key_parts = 2;
			dice::template_library::for_range<0UL, 3UL>(
					[&](auto no_entries) {
						slice<T::depth, typename T::htt_t, no_key_parts, no_entries>(500UL);
					});
		}

		template<size_t depth, HypertrieTrait htt_t>
		void slice_many() {
			CAPTURE(depth);
			CAPTURE(htt_t{});

			using key_part_type = typename htt_t::key_part_type;
			using value_type = typename htt_t::value_type;

			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance

			utils::RawEntryGenerator<depth, htt_t> gen{};

			for (size_t count : iter::chain(iter::range(1, 10), iter::range(10, 30, 5), iter::range(300, 301))) {
				// TODO: reconsider -- bad for high count and low depth
				key_part_type const min_key_part = 1;
				key_part_type const max_key_part = 1 + std::ceil(std::pow(count, 1.0 / depth));

				gen.setKeyPartMinMax(min_key_part, max_key_part);
				gen.setValueMinMax(value_type(1), value_type(2));

				CAPTURE(count);

				auto const runs = (count != 300) ? 500 : 5;
				for (auto const run : iter::range(runs)) {
					SUBCASE("{}"_format(run).c_str()) {
						gen.wind(run);

						auto const entries = gen.entries(count);
						CAPTURE(entries);

						NodePtr<depth, htt_t, allocator_type> nc{};
						ValidationRawNodeContext<depth, htt_t, allocator_type> const context{alloc, nc, entries};
						INFO("Context to slice:\n", context);

						dice::template_library::for_range<0UL, depth + 1>([&](/** the fixed depth of slices */ auto fixed_depth) {
							for (/** the positions where the raw_slice_key has <div>key_part</div> */ auto positions : iter::combinations(iter::range(depth), fixed_depth)) {
								static constexpr size_t result_depth = depth - fixed_depth;
								for (auto key_parts : iter::combinations_with_replacement(iter::range(min_key_part, max_key_part + 1), fixed_depth)) {
									RawSliceKey<fixed_depth, htt_t> raw_slice_key; {
										for (const auto [i, pos_and_key_part] : iter::enumerate(iter::zip(positions, key_parts))) {
											const auto &[pos, key_part] = pos_and_key_part;
											raw_slice_key[i].pos = pos;
											raw_slice_key[i].key_part = key_part;
										}
									}

									CAPTURE(raw_slice_key);

									std::unordered_set<SingleEntry<result_depth, htt_t>, dice::hash::DiceHashMartinus<SingleEntry<result_depth, htt_t>>> expected_entries; {
										auto expected_entries_v = slice_entries(entries, raw_slice_key);

										for (const auto expected_entry : expected_entries_v) {
											expected_entries.insert(expected_entry);
										}
									}

									auto slice = context.slice(nc, raw_slice_key);

									if constexpr (result_depth == 0) {
										if (expected_entries.empty()) {
											CHECK(slice == typename htt_t::value_type{});
										} else {
											assert(expected_entries.size() == 1);
											CHECK(slice == expected_entries.begin()->value());
										}
									} else {
										size_t count = 0;
										RawIterator<result_depth, true, htt_t, allocator_type> iter{slice.as_node_ptr()};
										for (; iter; ++iter) {
											auto const &e = *iter;
											CHECK(expected_entries.contains(e));
											count += 1;
										}

										CHECK(count == expected_entries.size());
									}
								}
							}
						});
					}
				}
			}
		}

		TEST_CASE_TEMPLATE("slice many depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {

			slice_many<T::depth, typename T::htt_t>();
		}

		TEST_CASE_TEMPLATE("slice many depth 2", T,
						   bool_cfg<2>,
						   tagged_bool_cfg<2>,
						   long_cfg<2>,
						   double_cfg<2>) {
			slice_many<T::depth, typename T::htt_t>();
		}

		TEST_CASE_TEMPLATE("slice many depth 3", T,
						   bool_cfg<3>,
						   tagged_bool_cfg<3>,
						   long_cfg<3>,
						   double_cfg<3>) {
			slice_many<T::depth, typename T::htt_t>();
		}

		TEST_CASE_TEMPLATE("slice many depth 4", T,
						   bool_cfg<4>,
						   tagged_bool_cfg<4>,
						   long_cfg<4>,
						   double_cfg<4>) {
			slice_many<T::depth, typename T::htt_t>();
		}

		TEST_CASE_TEMPLATE("slice many depth 5", T,
						   bool_cfg<5>,
						   tagged_bool_cfg<5>,
						   long_cfg<5>,
						   double_cfg<5>) {
			slice_many<T::depth, typename T::htt_t>();
		}

		TEST_CASE("problematic slice 1") {
			using cfg = bool_cfg<4>;
			static constexpr size_t depth = cfg::depth;
			using htt_t = typename cfg::htt_t;
			using allocator_type = std::allocator<std::byte>;

			std::vector<SingleEntry<depth, htt_t>> entries{SingleEntry<depth, htt_t>{{2, 1, 2, 1}, true},
														   SingleEntry<depth, htt_t>{{1, 1, 1, 1}, true}};

			NodePtr<depth, htt_t, allocator_type> node;
			ValidationRawNodeContext<5, htt_t, allocator_type> context{allocator_type{}, node, entries};
			dump_context(context);

			RawSliceKey<2, htt_t> slice_key;
			slice_key[0].pos = 0;
			slice_key[0].key_part = 1;
			slice_key[1].pos = 1;
			slice_key[1].key_part = 1;

			auto slice = context.template slice(node, slice_key);

			fmt::print("{}\n", slice.identifier());

			if (slice.holds_xn()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<CartesianNode>());
			} else if (slice.holds_fn()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<FullNode>());
			} else if (slice.holds_sen()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<SingleEntryNode>());
			}
		}

		TEST_CASE("problematic slice 2") {
			using cfg = bool_cfg<4>;
			static constexpr size_t depth = cfg::depth;
			using htt_t = typename cfg::htt_t;
			using allocator_type = std::allocator<std::byte>;

			std::vector<SingleEntry<depth, htt_t>> entries{SingleEntry<depth, htt_t>{{1, 1, 1, 2}, true},
														   SingleEntry<depth, htt_t>{{2, 1, 1, 1}, true},
														   SingleEntry<depth, htt_t>{{1, 1, 1, 1}, true}};

			NodePtr<depth, htt_t, allocator_type> nodec;
			ValidationRawNodeContext<5, htt_t, allocator_type> context{allocator_type{}, nodec, entries};
			dump_context(context);

			RawKeyPositions<depth> slice_key{std::initializer_list<uint8_t>{0, 3}};
			auto slice = context.template diagonal_slice<depth, 2>(nodec, slice_key, 1);

			fmt::print("{}\n", slice.identifier());

			if (slice.holds_xn()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<CartesianNode>());
			} else if (slice.holds_fn()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<FullNode>());
			} else if (slice.holds_sen()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<SingleEntryNode>());
			}
		}

		TEST_CASE("problematic slice 3") {
			using cfg = bool_cfg<4>;
			static constexpr size_t depth = cfg::depth;
			using htt_t = typename cfg::htt_t;
			using allocator_type = std::allocator<std::byte>;

			std::vector<SingleEntry<depth, htt_t>> entries{SingleEntry<depth, htt_t>{{1, 1, 2, 2}, true},
														   SingleEntry<depth, htt_t>{{2, 1, 1, 1}, true},
														   SingleEntry<depth, htt_t>{{1, 1, 1, 1}, true}};

			NodePtr<depth, htt_t, allocator_type> nodec;
			ValidationRawNodeContext<5, htt_t, allocator_type> context{allocator_type{}, nodec, entries};
			dump_context(context);

			RawKeyPositions<depth> slice_key{std::initializer_list<uint8_t>{0, 2}};
			auto slice = context.template diagonal_slice<depth, 2>(nodec, slice_key, 1);

			fmt::print("{}\n", slice.identifier());

			if (slice.holds_xn()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<CartesianNode>());
			} else if (slice.holds_fn()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<FullNode>());
			} else if (slice.holds_sen()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<SingleEntryNode>());
			}
		}

		TEST_CASE("problematic slice 4") {
			using cfg = long_cfg<2>;
			static constexpr size_t depth = cfg::depth;
			using htt_t = typename cfg::htt_t;
			using allocator_type = std::allocator<std::byte>;

			std::vector<SingleEntry<depth, htt_t>> entries{SingleEntry<depth, htt_t>{{3, 2}, 1},
														   SingleEntry<depth, htt_t>{{2, 2}, 2}};

			NodePtr<depth, htt_t, allocator_type> nodec;
			ValidationRawNodeContext<5, htt_t, allocator_type> context{allocator_type{}, nodec, entries};
			dump_context(context);

			RawSliceKey<1, htt_t> slice_key;
			slice_key[0].pos = 0;
			slice_key[0].key_part = 2;

			auto slice = context.slice(nodec, slice_key);

			fmt::print("{}\n", slice.identifier());

			if (slice.holds_xn()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<CartesianNode>());
			} else if (slice.holds_fn()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<FullNode>());
			} else if (slice.holds_sen()) {
				fmt::print("{}\n", *slice.as_node_ptr().template specific_ptr<SingleEntryNode>());
			}
		}
	}
};// namespace dice::hypertrie::tests::core::node