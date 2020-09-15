#ifndef HYPERTRIE_TESTRAWDIAGONAL_HPP
#define HYPERTRIE_TESTRAWDIAGONAL_HPP

#include <Dice/hypertrie/internal/Hypertrie_traits.hpp>
#include <Dice/hypertrie/internal/raw/iterator/Diagonal.hpp>
#include <Dice/hypertrie/internal/raw/storage/NodeContext.hpp>
#include <fmt/format.h>

#include "../utils/AssetGenerator.hpp"
#include "../utils/NameOfType.hpp"
#include "TestTensor.hpp"
namespace hypertrie::tests::raw::node_context::diagonal_test {

	using namespace hypertrie::tests::utils;

	using namespace hypertrie::internal::raw;

	using namespace hypertrie::internal;


	TEST_CASE("bool depth 1 uncompressed d", "[raw diagonal]") {
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


		typename tri::DiagonalPositions<depth> diag_poss {true};

		auto diag = hypertrie::internal::raw::HashDiagonal<1,depth, NodeCompression::uncompressed, tri>(nc, diag_poss, context);

		for(auto res : diag){
			key_part_type k =  res.first;
			fmt::print("{}\n", k);
		}
	}

	TEST_CASE("bool depth 2 uncompressed d", "[raw diagonal]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 2;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		UncompressedNodeContainer<depth, tri> nc{};

		const auto entries = gen.entries(2);

		context.template set<depth>(nc, {1,1}, true);
		context.template set<depth>(nc, {1,2}, true);
		context.template set<depth>(nc, {2,1}, true);
		context.template set<depth>(nc, {2,2}, true);


		typename tri::DiagonalPositions<depth> diag_poss {};
		diag_poss[0] = true;
		diag_poss[1] = false;

		std::cout << "0:" << diag_poss[0] << std::endl;
		std::cout << "1:" << diag_poss[1] << std::endl;

		auto diag = hypertrie::internal::raw::HashDiagonal<1,depth, NodeCompression::uncompressed, tri>(nc, diag_poss, context);

		for(auto res : diag){
			key_part_type k =  res.first;
			fmt::print("{}\n", k);
		}
	}

	TEST_CASE("bool depth 2 compressed d", "[raw diagonal]") {
		using tri = default_bool_Hypertrie_internal_t;
		constexpr const size_t depth = 2;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

		utils::RawGenerator<depth, key_part_type, value_type> gen{};

		NodeContext<depth, tri> context{};
		NodeContainer<depth, tri> nc{};

		const auto entries = gen.entries(2);

		context.template set<depth>(nc, {1,1}, true);

		typename tri::DiagonalPositions<depth> diag_poss {};
		diag_poss[0] = true;
		diag_poss[1] = false;

		std::cout << "0:" << diag_poss[0] << std::endl;
		std::cout << "1:" << diag_poss[1] << std::endl;

		auto c_nc = nc.compressed();
		auto diag = hypertrie::internal::raw::HashDiagonal<1,depth, NodeCompression::compressed, tri>(c_nc, diag_poss);

		for(auto res : diag){
			key_part_type k =  res.first;
			fmt::print("{}\n", k);
		}
	}
};

#endif//HYPERTRIE_TESTRAWDIAGONAL_HPP
