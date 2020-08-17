#ifndef HYPERTRIE_TESTRAWDIAGONAL_HPP
#define HYPERTRIE_TESTRAWDIAGONAL_HPP


#include <Dice/hypertrie/internal/node_based/raw/storage/NodeContext.hpp>
#include <Dice/hypertrie/internal/node_based/interface/Hypertrie_traits.hpp>
#include <Dice/hypertrie/internal/node_based/raw/iterator/Iterator.hpp>
#include <Dice/hypertrie/internal/node_based/raw/iterator/Diagonal.hpp>
#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "TestTensor.hpp"


namespace hypertrie::tests::node_based::raw::node_context::iterator_test {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::node_based::raw;

	using namespace hypertrie::internal::node_based;


	TEST_CASE("bool depth 1", "[iterator]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 1;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		const auto entries = gen.entries(2);

		context.template set<depth>(nc, {1}, true);
		context.template set<depth>(nc, {2}, true);


		for(auto iter = iterator<depth, NodeCompression::uncompressed, tri>(nc, context); iter != false; ++iter){
			auto key = *iter;
			fmt::print("[{}] -> {}\n", key[0], true);
//			fmt::print("[{} {} {}] -> {}", key[0], key[1], key[2], true);
		}
	}

	TEST_CASE("bool depth 2", "[iterator]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 2;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		context.template set<depth>(nc, {1,3}, true);
		context.template set<depth>(nc, {2,4}, true);


		for(auto iter = iterator<depth, NodeCompression::uncompressed, tri>(nc, context); iter != false; ++iter){
			auto key = *iter;
			fmt::print("[{} {} ] -> {}\n", key[0], key[1],true);
		}
	}


}

#endif//HYPERTRIE_TESTRAWDIAGONAL_HPP
