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
#include <chrono>

namespace hypertrie::tests::core::node {

	TEST_SUITE("Loading a large dataset into RawNodeContext") {
		using namespace ::hypertrie::internal::raw;
		using namespace ::hypertrie::internal::util;

		TEST_CASE("load many triples") {
			using cfg = tagged_bool_cfg<3>;
			static constexpr size_t depth = cfg::depth;
			using tri = typename cfg::tri;

			using SingleEntry_t = SingleEntry<depth, tri_with_stl_alloc<tri>>;

			using key_part_type = typename tri::key_part_type;

			utils::RawEntryGenerator<depth, tri> gen{};

			static constexpr size_t batch = 1'000'000UL;
			static constexpr size_t runs = 10UL;
			static constexpr size_t total_count = runs * batch;


			gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(total_count, 1.0 / depth))));
			gen.setValueMinMax(true, true);


			auto entries = gen.entries(total_count);
			std::vector<std::vector<SingleEntry_t>> entry_batches;
			for (size_t run : iter::range(runs))
				entry_batches.emplace_back(entries.begin() + (run * batch), entries.begin() + ((run + 1) * batch));

			SUBCASE("insert entries ") {
				RawHypertrieContext<depth, tri> context{std::allocator<std::byte>()};
				NodeContainer<depth, tri> nc{};


				auto start = std::chrono::high_resolution_clock::now();
				for (const auto run : iter::range(runs)) {
					auto batch_start = std::chrono::high_resolution_clock::now();
					context.insert(nc, entry_batches[run]);
					auto batch_duration = std::chrono::high_resolution_clock::now() - batch_start;
					fmt::print("batch {}\n  time: {}\n",
							   run,
							   std::chrono::duration_cast<std::chrono::milliseconds>(batch_duration).count());
					std::cout.flush();
				}
				auto duration = std::chrono::high_resolution_clock::now() - start;
				fmt::print("total time: {}\n",
						   std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
			}
		}
	}
}// namespace hypertrie::tests::core::node
