#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <dice/hypertrie/internal/util/fmt_utils.hpp>
#include <utils/Node_test_configs.hpp>
#include <utils/RawEntryGenerator.hpp>
#include <utils/ValidationRawNodeContext.hpp>
#include <utils/DumpRawContext.hpp>

#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_FullNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_Identifier.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntry.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_SingleEntryNode.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawBulkUpdater.hpp>
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/SynchronousRawBulkUpdater.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

#include <chrono>

namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("Loading a large dataset into RawNodeContext") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		TEST_CASE("load many triples") {
			using cfg = tagged_bool_cfg<3>;
			static constexpr size_t depth = cfg::depth;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using htt_t = typename cfg::htt_t;

			using SingleEntry_t = SingleEntry<depth, htt_t>;

			using key_part_type = typename htt_t::key_part_type;

			utils::RawEntryGenerator<depth, htt_t> gen{};

			static constexpr uint32_t batch = 1'000'000U;
			static constexpr size_t runs = 10UL;
			static constexpr size_t total_count = runs * batch;


			gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(total_count, 1.1 / depth))));
			gen.setValueMinMax(true, true);


			auto entries = gen.entries(total_count);

			std::cout << "total triples" << entries.size() << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			NodeContainer<depth, htt_t, allocator_type> nc{};

			SUBCASE("insert entries manually") {
				auto start = std::chrono::high_resolution_clock::now();
				std::vector<std::vector<SingleEntry_t>> entry_batches;
				for (size_t run : iter::range(runs))
					entry_batches.emplace_back(entries.begin() + (run * batch), entries.begin() + ((run + 1) * batch));

				for (const auto run : iter::range(runs)) {
					auto batch_start = std::chrono::high_resolution_clock::now();
					for (auto &entry : entry_batches[run])
						if (context.template get<depth>(nc, entry.key()))
							break;
					context.insert(nc, std::move(entry_batches[run]));
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

			SUBCASE("insert with asynchronous bulk loader") {
				auto start = std::chrono::high_resolution_clock::now();
				{
					size_t run = 0;
					auto batch_start = std::chrono::high_resolution_clock::now();
					RawNodeContainer<htt_t, allocator_type> raw_nc{nc};
					RawHypertrieBulkInserter<depth, htt_t, allocator_type, depth> bulk_insert//
							{raw_nc, context, batch,
							 [&]([[maybe_unused]] size_t processed_entries,
								 [[maybe_unused]] size_t inserted_entries,
								 [[maybe_unused]] size_t hypertrie_size_after) {
								 auto batch_duration = std::chrono::high_resolution_clock::now() - batch_start;
								 fmt::print("batch {}\n  time: {}\n",
											run++,
											std::chrono::duration_cast<std::chrono::milliseconds>(batch_duration).count());
								 std::cout << std::flush;
								 batch_start = std::chrono::high_resolution_clock::now();
							 }};
					for (const auto &entry : entries) {
						bulk_insert.add(entry);
					}
					nc = NodeContainer<depth, htt_t, allocator_type>{raw_nc};
				}
				fmt::print("size: {}\n", context.size(nc));
				auto duration = std::chrono::high_resolution_clock::now() - start;
				fmt::print("total time: {}\n",
						   std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
			}

			SUBCASE("insert with synchronous bulk loader") {
				auto start = std::chrono::high_resolution_clock::now();
				{
					size_t run = 0;
					auto batch_start = std::chrono::high_resolution_clock::now();
					RawNodeContainer<htt_t, allocator_type> raw_nc{nc};
					SynchronousRawHypertrieBulkUpdater<BulkUpdaterMode::Insert, depth, htt_t, allocator_type, depth> bulk_insert//
							{raw_nc, context, batch,
							 [&]([[maybe_unused]] size_t processed_entries,
								 [[maybe_unused]] size_t inserted_entries,
								 [[maybe_unused]] size_t hypertrie_size_after) {
								 auto batch_duration = std::chrono::high_resolution_clock::now() - batch_start;
								 fmt::print("batch {}\n  time: {}\n",
											run++,
											std::chrono::duration_cast<std::chrono::milliseconds>(batch_duration).count());
								 std::cout << std::flush;
								 batch_start = std::chrono::high_resolution_clock::now();
							 }};
					for (const auto &entry : entries) {
						bulk_insert.add(entry);
					}
					nc = NodeContainer<depth, htt_t, allocator_type>{raw_nc};
				}
				fmt::print("size: {}\n", context.size(nc));
				auto duration = std::chrono::high_resolution_clock::now() - start;
				fmt::print("total time: {}\n",
						   std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
			}
		}

		TEST_CASE("load then remove many triples") {
			using cfg = tagged_bool_cfg<3>;
			static constexpr size_t depth = cfg::depth;
			using allocator_type = std::allocator<std::byte>;
			allocator_type alloc{};// allocator instance
			using htt_t = typename cfg::htt_t;

			using key_part_type = typename htt_t::key_part_type;

			utils::RawEntryGenerator<depth, htt_t> gen{};

			static constexpr uint32_t batch = 1'000'000;
			static constexpr size_t runs = 10;
			static constexpr size_t total_count = runs * batch;


			gen.setKeyPartMinMax(key_part_type(1), key_part_type(1 + std::ceil(std::pow(total_count, 1.1 / depth))));
			gen.setValueMinMax(true, true);

			auto entries = gen.entries(total_count);

			std::cout << "total triples: " << entries.size() << std::endl;

			RawHypertrieContext<depth, htt_t, allocator_type> context{std::allocator<std::byte>()};
			RawNodeContainer<htt_t, allocator_type> raw_nc{};

			auto start = std::chrono::high_resolution_clock::now();
			{
				size_t run = 0;
				auto batch_start = std::chrono::high_resolution_clock::now();

				std::cout << "inserting..." << std::endl;
				RawHypertrieBulkInserter<depth, htt_t, allocator_type, depth> bulk_insert{
						raw_nc,
						context,
						batch,
						[&]([[maybe_unused]] size_t processed_entries,
							[[maybe_unused]] size_t inserted_entries,
							[[maybe_unused]] size_t hypertrie_size_after) {
							auto batch_duration = std::chrono::high_resolution_clock::now() - batch_start;

							std::cout << "batch " << run++ << '\n'
									  << "time: " << std::chrono::duration_cast<std::chrono::milliseconds>(batch_duration).count() << std::endl;

							batch_start = std::chrono::high_resolution_clock::now();
						}};

				for (const auto &entry : entries) {
					bulk_insert.add(entry);
				}

				auto duration = std::chrono::high_resolution_clock::now() - start;
				fmt::print("total time for insertion: {}\n",
						   std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
			}

			std::cout << "size after insertion: " << context.size(NodeContainer<depth, htt_t, allocator_type>{raw_nc}) << std::endl;

			start = std::chrono::high_resolution_clock::now();
			{
				size_t run = 0;
				auto batch_start = std::chrono::high_resolution_clock::now();

				std::cout << "removing..." << std::endl;

				RawHypertrieBulkRemover<depth, htt_t, allocator_type, depth> bulk_remove {
						raw_nc,
						context,
						batch,
						[&]([[maybe_unused]] size_t processed_entries,
							[[maybe_unused]] size_t inserted_entries,
							[[maybe_unused]] size_t hypertrie_size_after) {
							auto batch_duration = std::chrono::high_resolution_clock::now() - batch_start;

							std::cout << "batch " << run++ << '\n'
									<< "time: " << std::chrono::duration_cast<std::chrono::milliseconds>(batch_duration).count() << std::endl;

							batch_start = std::chrono::high_resolution_clock::now();
						}};

				for (auto const &entry : entries) {
					bulk_remove.add(entry);
				}
			}

			auto const size = context.size(NodeContainer<depth, htt_t, allocator_type>{raw_nc});

			CHECK(size == 0);

			fmt::print("size after removal: {}\n", size);
			auto duration = std::chrono::high_resolution_clock::now() - start;
			fmt::print("total time for removal: {}\n",
					   std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
		}
	}
}// namespace dice::hypertrie::tests::core::node
