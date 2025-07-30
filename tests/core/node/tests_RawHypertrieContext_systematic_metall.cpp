#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <dice/hypertrie/internal/container/AllContainer.hpp>

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
#include <dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <dice/hypertrie/internal/raw/node_context/fmt_RawHypertrieContext.hpp>

#include <utils/EntrySetGenerator.hpp>

//#include <metall/metall.hpp>

#include "../metall_mute_warnings.hpp"
// TODO: where do we include this?? -> library withing tests, maybe?
// TODO: do not use relative paths
#include <dice/sparse-map/boost_offset_pointer.hpp>

#include <dice/template-library/for.hpp>

//TODO: I only need this on my laptop, not on my other machines.
// I need to find out why that is.
namespace std {
	template<typename T>
	struct indirectly_readable_traits<boost::interprocess::offset_ptr<T>> {
		using value_type = typename boost::interprocess::offset_ptr<T>::value_type;
	};
}// namespace std

namespace dice::hypertrie::tests::core::node {
	bool debug = false;

	using allocator = metall::manager::allocator_type<std::byte>;

	TEST_SUITE("systematic testing of RawNodeContext with the metall allocator") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		template<size_t depth, HypertrieTrait htt_t,
				 size_t no_key_parts,
				 size_t min_no_entries,
				 size_t max_no_entries>
		void metall_write_and_read() {
			using SingleEntry_t = SingleEntry<depth, htt_t>;
			using key_part_type = typename htt_t::key_part_type;
			static constexpr auto max_key_part = key_part_type(no_key_parts);
			using allocator = metall::manager::allocator_type<std::byte>;

			const std::string path = fmt::format("/tmp/{}", std::random_device()());// important for parallel execution of different tests

			const std::string contextName = "someNameForTheObjectCreatedByMetall1";
			const std::string containerName = "someNameForTheObjectCreatedByMetall2";

			//create segment
			{
				metall::manager manager(metall::create_only, path.c_str());
			}
			//create allocator and manager to use
			metall::manager manager(metall::open_only, path.c_str());
			auto alloc = manager.get_allocator();

			auto generateConAndNC = [&manager, &alloc, &contextName, &containerName]() {
				auto &context = *manager.construct<RawHypertrieContext<depth, htt_t, allocator>>(contextName.c_str())(alloc);
				auto &nc = *manager.construct<NodeContainer<depth, htt_t, allocator>>(containerName.c_str())();
				return std::forward_as_tuple(context, nc);
			};
			auto deleteConAndNC = [&manager, &contextName, &containerName]() {
				manager.destroy<RawHypertrieContext<depth, htt_t, allocator>>(contextName.c_str());
				manager.destroy<NodeContainer<depth, htt_t, allocator>>(containerName.c_str());
			};

			SUBCASE("{}"_format(htt_t{}).c_str()) {
				SUBCASE("hypertrie depth = {}"_format(depth).c_str()) {
					dice::template_library::for_range<min_no_entries, max_no_entries + 1>([&](auto no_entries_0) {
						dice::template_library::for_range<min_no_entries, max_no_entries + 1>([&](auto no_entries_1) {
							SUBCASE("first {} entries, then {} entries"_format(no_entries_0, no_entries_1).c_str()) {
								utils::EntrySetGenerator<depth, no_entries_0, htt_t, max_key_part> outer_generator{};
								for (const auto &entries_0 : outer_generator) {
									SUBCASE("first_entries: {}"_format(fmt::join(entries_0, " | ")).c_str()) {
										auto [context, nc] = generateConAndNC();
										context.insert(nc, std::vector{entries_0});
										ValidationRawNodeContext<depth, htt_t, std::allocator<std::byte>> validation_context_0{std::allocator<std::byte>(), entries_0};
										CHECK(context == validation_context_0);
										if (debug) {
											std::cout << "\nContext before:" << std::endl;
											dump_context(context);
											dump_context_hash_translation_table(context);
										}

										utils::EntrySetGenerator_with_exclude<depth, no_entries_1, htt_t, max_key_part> inner_generator{entries_0};
										for (const auto &entries_1 : inner_generator) {
											SUBCASE("second_entries: {}"_format(fmt::join(entries_1, " | ")).c_str()) {
												std::vector<SingleEntry_t> all_entries = entries_0;
												all_entries.insert(all_entries.end(), entries_1.begin(), entries_1.end());

												context.insert(nc, std::vector{entries_1});
												if (debug) {
													std::cout << "\nContext after:" << std::endl;
													dump_context(context);
													dump_context_hash_translation_table(context);
												}

												ValidationRawNodeContext<depth, htt_t, std::allocator<std::byte>> validation_context{std::allocator<std::byte>(), all_entries};
												if (debug) {
													std::cout << "\nContext expected:" << std::endl;
													dump_context(validation_context);
													dump_context_hash_translation_table(validation_context);
												}

												CHECK(context == validation_context);
											}
										}

										deleteConAndNC();
									}
								}
							}
						});
					});
				}
			}
		}

		TEST_CASE("entry_generator") {
			using config = bool_cfg<2>;
			constexpr auto depth = config::depth;
			using htt_t = config::htt_t;
			utils::EntrySetGenerator<depth, 1, htt_t, 2> entry_set_generator{};
			for (const auto &item : entry_set_generator) {
				fmt::print("outer: {}\n", fmt::join(item, " | "));
				utils::EntrySetGenerator_with_exclude<depth, 1, htt_t, 2> inner_generator{item};
				for (const auto &inner_item : inner_generator) {
					fmt::print("  inner: {}\n", fmt::join(inner_item, " | "));
				}
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 1", T,
						   bool_cfg<1>,
						   tagged_bool_cfg<1>,
						   long_cfg<1>,
						   double_cfg<1>) {
			constexpr size_t no_key_parts = 3;
			constexpr size_t min_no_entries = 1;
			constexpr size_t max_no_entries = 3;
			metall_write_and_read<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
		}

		TEST_CASE_TEMPLATE("hypertrie depth 2", T,
						   bool_cfg<2>,
						   tagged_bool_cfg<2>,
						   long_cfg<2>,
						   double_cfg<2>) {
			constexpr size_t no_key_parts = 2;
			constexpr size_t min_no_entries = 1;
			constexpr size_t max_no_entries = 2;
			metall_write_and_read<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
		}
		//
		//
		TEST_CASE_TEMPLATE("hypertrie depth 3", T,
						   bool_cfg<3>,
						   tagged_bool_cfg<3>,
						   long_cfg<3>,
						   double_cfg<3>) {
			{
				constexpr size_t no_key_parts = 3;
				constexpr size_t min_no_entries = 1;
				constexpr size_t max_no_entries = 1;
				metall_write_and_read<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
			}
			{
				constexpr size_t no_key_parts = 2;
				constexpr size_t min_no_entries = 1;
				constexpr size_t max_no_entries = 3;
				metall_write_and_read<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
			}
		}

		TEST_CASE_TEMPLATE("hypertrie depth 4", T,
						   bool_cfg<4>,
						   tagged_bool_cfg<4>,
						   long_cfg<4>,
						   double_cfg<4>) {
			{
				constexpr size_t no_key_parts = 3;
				constexpr size_t min_no_entries = 1;
				constexpr size_t max_no_entries = 1;
				metall_write_and_read<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
			}
			{
				constexpr size_t no_key_parts = 2;
				constexpr size_t min_no_entries = 2;
				constexpr size_t max_no_entries = 3;
				metall_write_and_read<T::depth, typename T::htt_t, no_key_parts, min_no_entries, max_no_entries>();
			}
		}
	}
}// namespace dice::hypertrie::tests::core::node