#ifndef HYPERTRIE_TESTSLICING_HPP
#define HYPERTRIE_TESTSLICING_HPP


#include <iterator>
#include <ranges>

#include <boost/hana/for_each.hpp>
#include <boost/hana/range.hpp>

#include <Dice/hypertrie/internal/raw/storage/NodeContext.hpp>

#include <fmt/format.h>


#include "../slice/TestSlice.hpp"
#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"


namespace hypertrie::tests::raw::node_context::slicing {

	namespace hana = boost::hana;
	using namespace fmt::literals;


	using namespace hypertrie::tests::utils;
	using namespace hypertrie::internal::raw;

	template<HypertrieInternalTrait tri, size_t depth>
	void slicing_by_any_keypart() {
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using Key = typename tri::template RawKey<depth>;

		static constexpr const size_t max_key_part = 3;

		static utils::RawGenerator<depth, key_part_type, value_type, size_t(tri::is_lsb_unused)> gen{0, max_key_part};

		unsigned long number_of_entries = size_t(std::pow(max_key_part, depth) / 2);
		auto entries = gen.entries(number_of_entries);

		NodeContext<depth, tri> context{};
		// create emtpy primary node
		NodeContainer<depth, tri> nc{};

		std::string print_entries{};
		for (auto &[key, value] : entries)
			print_entries += "{} → value\n"_format(key);
		WARN(print_entries);


		// bulk insert keys
		for (const auto &[key, value] : entries)
			context.template set<depth>(nc, key, value);

		// we need slices of each depth
		hana::for_each(hana::range_c<size_t, 0, depth>, [&](auto fixed_depth) {
			SECTION("slice depth: {}"_format(size_t(fixed_depth))) {
				//			[[maybe_unused]] static constexpr size_t result_depth = depth - slice_depth;
				using RawSliceKey = typename tri::template RawSliceKey<fixed_depth>;

				RawSliceKey raw_slice_key{};

				// get all the combinations for slices of depth slice_depth
				for (auto slice_poss_tmp : iter::combinations(iter::range(depth), size_t(fixed_depth))) {
					std::vector<size_t> fixed_poss{slice_poss_tmp.begin(), slice_poss_tmp.end()};
					// try all possible slices keys
					for (auto key_parts : iter::product<size_t(fixed_depth)>(iter::range(0ul, max_key_part + 1))) {
						// from std::tuple to std::array
						auto &key_parts_reint = *reinterpret_cast<std::array<size_t, size_t(fixed_depth)> *>(&key_parts);


						CAPTURE(raw_slice_key, fixed_poss, key_parts_reint);

						for (auto [slice_key_part, slice_pos, key_part] : iter::zip(raw_slice_key, fixed_poss, key_parts_reint))
							slice_key_part = {slice_pos, key_part};

						CAPTURE(raw_slice_key, fixed_poss, key_parts_reint);

						TestSlice<tri, depth, fixed_depth> expected_slice{entries, raw_slice_key};
						auto expected_slice_container = context.template slice(nc, raw_slice_key);


						if constexpr (depth > size_t(fixed_depth)) {
							auto [actual_nodec, actual_is_managed] = expected_slice_container;

							// check whether the slice is empty
							REQUIRE(actual_nodec.empty() == expected_slice.empty());

							// check if the result is managed, i.e. not a slice of compressed key
							// TODO: refactor and reactivate once implemented
							//							REQUIRE(expected_slice.isManaged(context) == actual_is_managed);

							// check size
							REQUIRE(context.template size(actual_nodec) == expected_slice.size());

							// check the entries
							for (const auto &[expected_key, expected_value] : expected_slice.sliceEntries()) {
								REQUIRE(context.template get(actual_nodec, expected_key) == expected_value);
							}
						} else {
							auto [actual_value, actual_is_managed] = expected_slice_container.value();

							// check if the result is managed, i.e. not a slice of compressed key
							REQUIRE(expected_slice.isManaged(context) == actual_is_managed);

							REQUIRE(actual_value = expected_slice.sliceEntries().begin()->second);
						}
					}
				}
			}
		});
	}

	TEST_CASE("test") {
		slicing_by_any_keypart<default_bool_Hypertrie_internal_t, 3>();
	}

}// namespace hypertrie::tests::raw::node_context::slicing

#endif//HYPERTRIE_TESTSLICING_HPP
