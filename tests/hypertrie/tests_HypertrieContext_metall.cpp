#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>


#include <dice/hypertrie.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>
#include <dice/hypertrie/internal/fmt_Hypertrie_trait.hpp>
//#include <metall/metall.hpp>
#include "../metall_mute_warnings.hpp"

#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

#include <random>


namespace dice::hypertrie::tests::core::node {

	TEST_SUITE("Testing of HypertrieContext") {
		using namespace ::dice::hypertrie;

		using htt_t = default_bool_Hypertrie_trait;
		using allocator_type = metall::manager::allocator_type<std::byte>;

		TEST_CASE("Basic creation-insertion-removal-deletion") {
			const std::string path = fmt::format("/tmp/{}", std::random_device()());
			constexpr auto context = "context";
			constexpr auto hypertrie = "hypertrie001";


			//create segment
			{
				metall::manager manager(metall::create_only, path.c_str());
			}

			try {// write into manager
				metall::manager manager(metall::open_only, path.c_str());
				// create context
				auto ctx_ptr = manager.construct<HypertrieContext<htt_t, allocator_type>>(context)(manager.get_allocator());
				// create hypertrie in context
				auto ht_ptr = manager.construct<Hypertrie<htt_t, allocator_type>>(hypertrie)(3, ctx_ptr);

				{
					auto &hypertrie = *ht_ptr;
					// set entry
					hypertrie.set({{1, 2, 3}}, true);
					// validate entry
					CHECK(std::get<bool>(hypertrie[SliceKey<htt_t>{{1, 2, 3}}]) == true);
					CHECK(hypertrie[Key<htt_t>{{1, 2, 3}}] == true);
					CHECK(hypertrie[Key<htt_t>{{0, 0, 0}}] == false);
				}
			} catch (...) {}


			try {// reopen and read the segment
				metall::manager manager(metall::open_only, path.c_str());
				// get existing context
				auto ctx_ptr = std::get<0>(manager.find<HypertrieContext<htt_t, allocator_type>>(context));
				// get existing hypertrie from context
				auto ht_ptr = std::get<0>(manager.find<Hypertrie<htt_t, allocator_type>>(hypertrie));

				{
					fmt::print("before: {}\n", ctx_ptr->raw_context());
					// destroy hypertrie
					manager.destroy_ptr(ht_ptr);
					fmt::print("after: {}\n", ctx_ptr->raw_context());
				}
			} catch (...) {}

			metall::manager::remove(path.c_str());
		}

		TEST_CASE("write and read a single entry") {
			const std::string path = fmt::format("/tmp/{}", std::random_device()());
			constexpr auto context = "context";
			constexpr auto hypertrie001 = "hypertrie001";
			constexpr auto hypertrie002 = "hypertrie002";

			//create segment
			{
				metall::manager manager(metall::create_only, path.c_str());
			}

			try {// write into manager
				metall::manager manager(metall::open_only, path.c_str());
				// create context
				auto *ctx_ptr = manager.construct<HypertrieContext<htt_t, allocator_type>>(context)(manager.get_allocator());
				// create hypertrie in context
				auto &hypertrie = *manager.construct<Hypertrie<htt_t, allocator_type>>(hypertrie001)(3, ctx_ptr);

				{
					hypertrie.set({{1, 2, 3}}, true);

					auto result = hypertrie[SliceKey<htt_t>{{1, 2, 3}}];
					CHECK(std::get<bool>(result) == true);
					CHECK(hypertrie[Key<htt_t>{{1, 2, 3}}] == true);
					CHECK(hypertrie[Key<htt_t>{{0, 0, 0}}] == false);

					SUBCASE("Diagonal") {
						HashDiagonal<htt_t, allocator_type> hash_diagonal(hypertrie, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{1, 2}});

						CHECK(not hash_diagonal.find(2));// this should be false
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
						auto &hypertrie2 = *manager.construct<Hypertrie<htt_t, allocator_type>>(hypertrie002)(3, ctx_ptr);
						hypertrie2.set({{1, 2, 3}}, true);
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
				}
			} catch (...) {}

			metall::manager::remove(path.c_str());
		};


		TEST_CASE("move HashDiagonals") {
			const std::string path = fmt::format("/tmp/{}", std::random_device()());// important for parallel execution of different tests
			constexpr auto context = "context";
			constexpr auto hypertrie001 = "hypertrie001";

			//create segment
			{
				metall::manager manager(metall::create_only, path.c_str());
			}

			try {// write into manager
				metall::manager manager(metall::open_only, path.c_str());

				// create context
				auto *ctx_ptr = manager.construct<HypertrieContext<htt_t, allocator_type>>(context)(manager.get_allocator());
				// create hypertrie in context
				auto &hypertrie = *manager.construct<Hypertrie<htt_t, allocator_type>>(hypertrie001)(2, ctx_ptr);

				{
					hypertrie.set({{1, 2}}, true);
					hypertrie.set({{2, 2}}, true);

					HashDiagonal<htt_t, allocator_type> hash_diagonal(hypertrie, ::dice::hypertrie::internal::raw::RawKeyPositions<hypertrie_max_depth>{std::initializer_list<size_t>{0}});

					HashDiagonal<htt_t, allocator_type> hash_diagonal_2 = std::move(hash_diagonal);

					CHECK(hash_diagonal_2.find(1) == true);
					std::cout << static_cast<std::string>(hash_diagonal_2.current_hypertrie()) << std::endl;
				}
			} catch (...) {}

			metall::manager::remove(path.c_str());
		};
	};
};// namespace dice::hypertrie::tests::core::node