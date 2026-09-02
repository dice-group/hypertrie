#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <utils/Node_test_configs.hpp>
#include <utils/RawEntryGenerator.hpp>
#include <utils/ValidationRawHashDiagonal.hpp>
#include <utils/ValidationRawNodeContext.hpp>
#include <utils/DumpRawContext.hpp>


#include <dice/hypertrie/Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/iteration/RawHashDiagonal.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_CartesianNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>
#include <utils/EntrySetGenerator.hpp>

#include <dice/template-library/for.hpp>

namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("Testing of RawHashDiagonal") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		template<size_t depth, HypertrieTrait htt_t,
				 size_t no_key_parts,
				 size_t no_entries>
		void test_diagonal(size_t max_entry_sets = 500) {
			CAPTURE(depth);
			CAPTURE(htt_t{});
			CAPTURE(no_key_parts);
			CAPTURE(no_entries);
			CAPTURE(max_entry_sets);

			using key_part_type = typename htt_t::key_part_type;
			//			using value_type = typename htt_t::value_type;

			utils::RawEntryGenerator<depth, htt_t> gen{};

			static constexpr key_part_type min_key_part = 1;
			static constexpr key_part_type max_key_part = no_key_parts;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{}; // allocator instance

			gen.setKeyPartMinMax(key_part_type(1), key_part_type(2));
			gen.setValueMinMax(true, true);

			utils::EntrySetGenerator<depth, no_entries, htt_t, max_key_part, min_key_part> outer_generator{};
			for (const auto &entries : outer_generator) {
				CAPTURE(entries);

				NodePtr<depth, htt_t, allocator_type> nodec;
				ValidationRawNodeContext<depth, htt_t, allocator_type> const context{alloc, nodec, entries};
				INFO("Context to slice:\n", context);

				dice::template_library::for_range<1UL, depth + 1UL>(
						[&](/** the fixed depth of the diagonals */ auto fixed_depth) {
							for (/** the positions for the diagonal */ auto const &positions : iter::combinations(iter::range(depth), fixed_depth)) {
								static constexpr size_t result_depth = depth - fixed_depth;

								RawKeyPositions<depth> const diag_poss(positions);
								CAPTURE(diag_poss);

								ValidationRawHashDiagonal<fixed_depth, depth, htt_t> const validation_raw_hash_diagonal(entries, diag_poss);
								CAPTURE(validation_raw_hash_diagonal);

								RawHashDiagonal<fixed_depth, depth, htt_t, allocator_type> raw_hash_diagonal{nodec, diag_poss};

								CHECK_MESSAGE(validation_raw_hash_diagonal.size() <= raw_hash_diagonal.size(), "size estimation must be an upper bound to the actual number of non-zero slices in the diagonal.");

								SUBCASE("check iterator") {
									if (raw_hash_diagonal.empty()) {
										CHECK(validation_raw_hash_diagonal.size() == 0);
										CHECK(raw_hash_diagonal.size() == 0);
										CHECK(raw_hash_diagonal.ended());
									} else {
										CHECK(raw_hash_diagonal.size() > 0);

										size_t count = 0;

										for (; raw_hash_diagonal; ++raw_hash_diagonal) {
											auto const &kps = *raw_hash_diagonal;
											auto const key_part = kps.first;
											auto const &slice = kps.second;

											CAPTURE(key_part);
											CHECK(validation_raw_hash_diagonal.has_diagonal(key_part));
											if constexpr (result_depth != 0) {
												NodePtr<result_depth, htt_t, allocator_type> slice_instance = slice.as_node_ptr();

												switch (slice_instance.tag()) {
													case IdentifierTag::FN: {
														CAPTURE(*slice_instance.template specific_ptr<FullNode>());
														break;
													}
													case IdentifierTag::SEN: {
														if constexpr (result_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
															CAPTURE(slice_instance.decode_key_part());
														} else {
															CAPTURE(*slice_instance.template specific_ptr<SingleEntryNode>());
														}
														break;
													}
													case IdentifierTag::XN: {
														CAPTURE(*slice_instance.template specific_ptr<CartesianNode>());
														break;
													}
													case IdentifierTag::Indeterminate: {
														HYPERTRIE_UNREACHABLE;
													}
												}

												CHECK(!slice.empty());
												CHECK(slice_instance.tag() == validation_raw_hash_diagonal.raw_identifier(key_part).tag());

												auto const &dentries = validation_raw_hash_diagonal.entries(key_part);
												CAPTURE(dentries);

												for (const auto &entry : dentries) {
													CHECK(context.get(slice_instance, entry.key()) == entry.value());
												}

												CHECK(context.size(slice_instance) == dentries.size());
											} else {
												CAPTURE(slice);
												CHECK(slice != typename htt_t::value_type{});
												CHECK(slice == validation_raw_hash_diagonal.entries(key_part)[0].value());
											}
											++count;
										}

										CHECK(validation_raw_hash_diagonal.size() == count);
									}
								}

								SUBCASE("check element retrieval") {
									if (raw_hash_diagonal.empty()) {
										CHECK(validation_raw_hash_diagonal.size() == 0);
										CHECK(raw_hash_diagonal.size() == 0);
										CHECK(raw_hash_diagonal.ended());
									} else {
										CHECK(raw_hash_diagonal.size() > 0);

										for (const auto &key_part : validation_raw_hash_diagonal.key_parts()) {
											CAPTURE(key_part);
											bool found = raw_hash_diagonal.find(key_part);
											CHECK_MESSAGE(found, "make sure the key_part was found");
											auto const &slice = raw_hash_diagonal.current_diagonal();

											if constexpr (result_depth != 0) {
												CHECK(!slice.empty());

												NodePtr<result_depth, htt_t, allocator_type> slice_instance = slice.as_node_ptr();

												CHECK(slice_instance.tag() == validation_raw_hash_diagonal.raw_identifier(key_part).tag());
												for (const auto &entry : validation_raw_hash_diagonal.entries(key_part))
													CHECK(context.get(slice_instance, entry.key()) == entry.value());

												CAPTURE(validation_raw_hash_diagonal.entries(key_part));
												CHECK(context.size(slice_instance) == validation_raw_hash_diagonal.entries(key_part).size());
											} else {
												CAPTURE(slice);
												CHECK(slice != typename htt_t::value_type{});
												CHECK(slice == validation_raw_hash_diagonal.entries(key_part)[0].value());
											}
										}
									}
								}
							}
						});

				if (max_entry_sets-- == 0)
					break;
			}
		}

		template<size_t depth, HypertrieTrait htt_t>
		void test_diagonal_many() {
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

				for (auto const run : iter::range(500)) {
					SUBCASE("{}"_format(run).c_str()) {
						gen.wind(run);

						auto const entries = gen.entries(count);
						CAPTURE(entries);

						NodePtr<depth, htt_t, allocator_type> nodec;
						ValidationRawNodeContext<depth, htt_t, allocator_type> const context{alloc, nodec, entries};
						INFO("Context to slice:\n", context);

						dice::template_library::for_range<1UL, depth + 1>([&](/** the fixed depth of slices */ auto fixed_depth) {
							for (/** the positions where the raw_slice_key has <div>key_part</div> */ auto positions : iter::combinations(iter::range(depth), fixed_depth)) {
								static constexpr size_t result_depth = depth - fixed_depth;

								RawKeyPositions<depth> diag_poss{positions};
								CAPTURE(diag_poss);

								ValidationRawHashDiagonal<fixed_depth, depth, htt_t> validation_raw_hash_diagonal(entries, diag_poss);
								CAPTURE(validation_raw_hash_diagonal);

								RawHashDiagonal<fixed_depth, depth, htt_t, allocator_type> raw_hash_diagonal{nodec, diag_poss};

								CHECK_MESSAGE(validation_raw_hash_diagonal.size() <= raw_hash_diagonal.size(), "size estimation must be an upper bound to the actual number of non-zero slices in the diagonal.");

								{ // check iterator
									if (raw_hash_diagonal.empty()) {
										CHECK(validation_raw_hash_diagonal.size() == 0);
										CHECK(raw_hash_diagonal.size() == 0);
										CHECK(raw_hash_diagonal.ended());
									} else {
										CHECK(raw_hash_diagonal.size() > 0);

										size_t count = 0;

										for (; raw_hash_diagonal; ++raw_hash_diagonal) {
											auto const &kps = *raw_hash_diagonal;
											auto const key_part = kps.first;
											auto const &slice = kps.second;

											CAPTURE(key_part);
											CHECK(validation_raw_hash_diagonal.has_diagonal(key_part));
											if constexpr (result_depth != 0) {
												NodePtr<result_depth, htt_t, allocator_type> slice_instance = slice.as_node_ptr();

												switch (slice_instance.tag()) {
													case IdentifierTag::FN: {
														CAPTURE(*slice_instance.template specific_ptr<FullNode>());
														break;
													}
													case IdentifierTag::SEN: {
														if constexpr (result_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
															CAPTURE(slice_instance.decode_key_part());
														} else {
															CAPTURE(*slice_instance.template specific_ptr<SingleEntryNode>());
														}
														break;
													}
													case IdentifierTag::XN: {
														CAPTURE(*slice_instance.template specific_ptr<CartesianNode>());
														break;
													}
													case IdentifierTag::Indeterminate: {
														HYPERTRIE_UNREACHABLE;
													}
												}

												CHECK(!slice.empty());
												CHECK(slice_instance.tag() == validation_raw_hash_diagonal.raw_identifier(key_part).tag());

												auto const &dentries = validation_raw_hash_diagonal.entries(key_part);
												CAPTURE(dentries);

												for (const auto &entry : dentries) {
													CHECK(context.get(slice_instance, entry.key()) == entry.value());
												}

												CHECK(context.size(slice_instance) == dentries.size());
											} else {
												CAPTURE(slice);
												CHECK(slice != typename htt_t::value_type{});
												CHECK(slice == validation_raw_hash_diagonal.entries(key_part)[0].value());
											}
											++count;
										}

										CHECK(validation_raw_hash_diagonal.size() == count);
									}
								}

								 { // check find
									if (raw_hash_diagonal.empty()) {
										CHECK(validation_raw_hash_diagonal.size() == 0);
										CHECK(raw_hash_diagonal.size() == 0);
										CHECK(raw_hash_diagonal.ended());
									} else {
										CHECK(raw_hash_diagonal.size() > 0);

										for (const auto &key_part : validation_raw_hash_diagonal.key_parts()) {
											CAPTURE(key_part);

											bool found = raw_hash_diagonal.find(key_part);
											CHECK_MESSAGE(found, "make sure the key_part was found");
											auto const &slice = raw_hash_diagonal.current_diagonal();

											if constexpr (result_depth != 0) {
												CHECK(!slice.empty());

												NodePtr<result_depth, htt_t, allocator_type> slice_instance = slice.as_node_ptr();

												CHECK(slice_instance.tag() == validation_raw_hash_diagonal.raw_identifier(key_part).tag());

												auto const &dentries = validation_raw_hash_diagonal.entries(key_part);
												CAPTURE(dentries);
												for (const auto &entry : dentries) {
													CHECK(context.get(slice_instance, entry.key()) == entry.value());
												}

												CHECK(context.size(slice_instance) == dentries.size());
											} else {
												CAPTURE(slice);
												CHECK(slice != typename htt_t::value_type{});
												CHECK(slice == validation_raw_hash_diagonal.entries(key_part)[0].value());
											}
										}
									}
								}
							}
						});
					}
				}
			}
		}


		TEST_CASE_TEMPLATE("hypertrie depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {
			SUBCASE(fmt::format("{}", typename T::htt_t{}).c_str()) {

				constexpr size_t no_key_parts = 3;

				{
					constexpr size_t no_entries = 1;
					test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
				}

				{
					constexpr size_t no_entries = 2;
					test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
				}

				{
					constexpr size_t no_entries = 3;
					test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
				}
			}

			SUBCASE("diagonal many") {
				test_diagonal_many<T::depth, typename T::htt_t>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 2", T,
						   bool_cfg<2>,
						   tagged_bool_cfg<2>,
						   long_cfg<2>,
						   double_cfg<2>) {
			SUBCASE(fmt::format("{}", typename T::htt_t{}).c_str()) {

				constexpr size_t no_key_parts = 2;
				{
					constexpr size_t no_entries = 1;
					test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
				}

				{
					constexpr size_t no_entries = 2;
					test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
				}
			}

			SUBCASE("diagonal many") {
				test_diagonal_many<T::depth, typename T::htt_t>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 3", T,
						   bool_cfg<3>,
						   tagged_bool_cfg<3>,
						   long_cfg<3>,
						   double_cfg<3>) {
			SUBCASE(fmt::format("{}", typename T::htt_t{}).c_str()) {
				{
					constexpr size_t no_key_parts = 3;
					{
						constexpr size_t no_entries = 1;
						test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
					}
				}
				{
					constexpr size_t no_key_parts = 2;
					{
						constexpr size_t no_entries = 2;
						test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
					}
					{
						constexpr size_t no_entries = 3;
						test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
					}
				}
			}

			SUBCASE("diagonal many") {
				test_diagonal_many<T::depth, typename T::htt_t>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 4", T,
						   bool_cfg<4>,
						   tagged_bool_cfg<4>,
						   long_cfg<4>,
						   double_cfg<4>) {
			SUBCASE(fmt::format("{}", typename T::htt_t{}).c_str()) {

				{
					constexpr size_t no_key_parts = 3;
					constexpr size_t no_entries = 1;
					test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
				}

				{
					constexpr size_t no_key_parts = 2;
					constexpr size_t no_entries = 2;
					test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
				}

				{
					constexpr size_t no_key_parts = 2;
					constexpr size_t no_entries = 3;
					test_diagonal<T::depth, typename T::htt_t, no_key_parts, no_entries>();
				}
			}

			SUBCASE("diagonal many") {
				test_diagonal_many<T::depth, typename T::htt_t>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 5", T,
						   bool_cfg<5>,
						   tagged_bool_cfg<5>,
						   long_cfg<5>,
						   double_cfg<5>) {
			SUBCASE("diagonal many") {
				test_diagonal_many<T::depth, typename T::htt_t>();
			}
		}

		TEST_CASE("copy") {
			using cfg = bool_cfg<3>;
			using htt_t = typename cfg::htt_t;
			using allocator_type = std::allocator<std::byte>;
			static constexpr size_t depth = cfg::depth;
			using SingleEntry_t = SingleEntry<depth, htt_t>;

			std::vector<SingleEntry_t> entries{
					SingleEntry_t{{1, 2, 1}, true},
					SingleEntry_t{{2, 1, 1}, true},
					SingleEntry_t{{3, 1, 1}, true}};

			static constexpr size_t fixed_depth = 1;
			RawKeyPositions<depth> positions{std::initializer_list<uint8_t>{0}};

			NodePtr<depth, htt_t, allocator_type> nodec;
			ValidationRawNodeContext<depth, htt_t, allocator_type> context{allocator_type{}, nodec, entries};
			dump_context(context);

			auto eq = [](auto const &a, auto const &b) noexcept {
				fmt::print("a.first: {}\n", a.first);
				fmt::print("b.first: {}\n", b.first);
				fmt::print("a.second.identifier(): {}\n", a.second.identifier());
				fmt::print("b.second.identifier(): {}\n", b.second.identifier());

				return a.first == b.first && a.second.identifier() == b.second.identifier();
			};

			RawKeyPositions<depth> diag_poss(positions);
			RawHashDiagonal<fixed_depth, depth, htt_t, allocator_type> diag{nodec, diag_poss};
			auto cpy = diag;

			CHECK(eq(*cpy, *diag));
			++cpy;

			auto cpy2 = cpy;

			CHECK(!eq(*cpy, *diag));
			CHECK(eq(*cpy2, *cpy));
			++diag;
			++cpy2;

			CHECK(eq(*cpy, *diag));
			CHECK(!eq(*cpy, *cpy2));
			++diag;
			CHECK(!eq(*cpy, *diag));
		}

		TEST_CASE("problematic diagonal") {
			using allocator_type = std::allocator<std::byte>;
			using htt_t = default_bool_Hypertrie_trait;

			std::vector<SingleEntry<5, htt_t>> entries{
					SingleEntry<5, htt_t>{ { 1, 2, 3, 2, 3 }, true },
					SingleEntry<5, htt_t>{ { 2, 1, 2, 3, 3 }, true },
					SingleEntry<5, htt_t>{ { 3, 2, 2, 2, 3 }, true }};

			RawKeyPositions<5> diag_poss{std::initializer_list<size_t>{1, 2}};
			NodePtr<5, htt_t, allocator_type> nodec;
			ValidationRawNodeContext<5, htt_t, allocator_type> context{allocator_type{}, nodec, entries};
			dump_context(context);

			RawHashDiagonal<2, 5, htt_t, allocator_type> diagonal{nodec, diag_poss};
		}
	}
};// namespace dice::hypertrie::tests::core::node