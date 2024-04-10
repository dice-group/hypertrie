#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "dice/query/OperandDependencyGraph.hpp"
#include <algorithm>
#include <doctest/doctest.h>

namespace dice::einsum::tests {

	using pos_type = hypertrie::internal::pos_type;

	TEST_CASE("Label Removal") {
		query::OperandDependencyGraph odg{};
		odg.add_operand({'a'});
		odg.add_operand({'a', 'b'});
		odg.add_operand({'a', 'c'});
		odg.add_operand({'a', 'd'});
		odg.add_dependency(0, 1, 'a');
		odg.add_dependency(1, 0, 'a');
		odg.add_dependency(1, 2, 'a');
		odg.add_dependency(2, 1, 'a');
		odg.add_dependency(2, 3, 'a');
		odg.add_dependency(3, 2, 'a');
		auto new_odg = odg.remove_var_id('a');
		CHECK_EQ(new_odg.size(), 3);
		CHECK_EQ(new_odg.operand_var_ids(0), std::vector<char>{'b'});
		CHECK_EQ(new_odg.operand_var_ids(1), std::vector<char>{'c'});
		CHECK_EQ(new_odg.operand_var_ids(2), std::vector<char>{'d'});
		auto copy_graph = odg;
	}

	TEST_CASE("Cartesian Components") {
		SUBCASE("Non Optional") {
			query::OperandDependencyGraph odg{};
			odg.add_operand({'b'});
			odg.add_operand({'c'});
			odg.add_operand({'d'});
			odg.add_dependency(0, 1);
			odg.add_dependency(1, 0);
			odg.add_dependency(1, 2);
			odg.add_dependency(2, 1);
			odg.add_dependency(0, 2);
			odg.add_dependency(2, 0);
			auto cc = odg.cartesian_components();
			CHECK_EQ(cc.size(), 3);
			CHECK_EQ(odg.optional_cartesian(), false);
		}
		SUBCASE("Optional") {
			query::OperandDependencyGraph odg{};
			odg.add_operand({'b'});
			odg.add_operand({'c'});
			odg.add_operand({'d'});
			odg.add_connection(0, 1);
			odg.add_connection(1, 0);
			odg.add_connection(1, 2);
			odg.add_connection(2, 1);
			odg.add_connection(0, 2);
			odg.add_connection(2, 0);
			auto cc = odg.cartesian_components();
			CHECK_EQ(cc.size(), 3);
			CHECK_EQ(odg.optional_cartesian(), true);
		}
	}

	TEST_CASE("Operands Original Positions") {
		SUBCASE("Cartesian Components") {
			query::OperandDependencyGraph odg{};
			odg.add_operand({'a'});
			odg.add_operand({'b', 'c'});
			odg.add_operand({'d', 'd'});
			odg.add_dependency(0, 1);
			odg.add_dependency(1, 0);
			odg.add_dependency(0, 2);
			odg.add_dependency(2, 0);
			odg.add_dependency(1, 2, 'c');
			odg.add_dependency(2, 1, 'c');
			auto cc1 = odg.cartesian_components();
			CHECK_EQ(cc1.size(), 2);
			CHECK_EQ(cc1[0].operands_original_positions(), std::vector<uint8_t>{0});
			CHECK_EQ(cc1[1].operands_original_positions(), std::vector<uint8_t>{1, 2});
			auto cc2 = cc1[1].remove_var_id('c').cartesian_components();
			CHECK_EQ(cc2.size(), 2);
			CHECK_EQ(cc2[0].operands_original_positions(), std::vector<uint8_t>{0});
			CHECK_EQ(cc2[1].operands_original_positions(), std::vector<uint8_t>{1});
		}
		SUBCASE("Remove Vertices") {
			query::OperandDependencyGraph odg{};
			odg.add_operand({'a'});
			odg.add_operand({'b', 'c'});
			odg.add_operand({'c', 'd'});
			odg.add_dependency(0, 1);
			odg.add_dependency(1, 0);
			odg.add_dependency(0, 2);
			odg.add_dependency(2, 0);
			odg.add_dependency(1, 2, 'c');
			odg.add_dependency(2, 1, 'c');
			auto new_odg = odg.remove_vertices({0});
			CHECK_EQ(new_odg.operands_original_positions(), std::vector<uint8_t>{1, 2});
		}
		SUBCASE("Remove Vertices and Cartesian Components") {
			query::OperandDependencyGraph odg{};
			odg.add_operand({'a'});
			odg.add_operand({'b', 'c'});
			odg.add_operand({'d', 'e'});
			odg.add_dependency(0, 1);
			odg.add_dependency(1, 0);
			odg.add_dependency(0, 2);
			odg.add_dependency(2, 0);
			odg.add_dependency(1, 2);
			odg.add_dependency(2, 1);
			auto new_odg = odg.remove_vertices({0});
			CHECK_EQ(new_odg.operands_original_positions(), std::vector<uint8_t>{1, 2});
			auto cc = new_odg.cartesian_components();
			CHECK_EQ(cc.size(), 2);
			CHECK_EQ(cc[0].operands_original_positions(), std::vector<uint8_t>{0});
			CHECK_EQ(cc[1].operands_original_positions(), std::vector<uint8_t>{1});
		}
	}

	TEST_CASE("Simple Graph") {
		query::OperandDependencyGraph odg{};
		odg.add_operand({'a', 'b'});
		odg.add_operand({'b', 'c'});
		odg.add_operand({'e'});
		odg.add_operand({'c', 'd'});
		odg.add_dependency(0, 1, 'b');
		odg.add_dependency(1, 0, 'b');
		odg.add_dependency(1, 3, 'c');
		odg.add_dependency(3, 1, 'c');
		odg.add_dependency(0, 2);
		odg.add_dependency(2, 0);
		odg.add_dependency(1, 2);
		odg.add_dependency(2, 1);
		odg.add_dependency(2, 3);
		odg.add_dependency(3, 2);
		SUBCASE("without unlabelled edges") {
			// basic testing
			CHECK_EQ(odg.size(), 4);
			CHECK_EQ(odg.operands_original_positions(), std::vector<pos_type>{0, 1, 2, 3});
			CHECK_EQ(odg.operand_var_ids(0), std::vector<char>{'a', 'b'});
			CHECK_EQ(odg.operand_var_ids(1), std::vector<char>{'b', 'c'});
			CHECK_EQ(odg.operand_var_ids(2), std::vector<char>{'e'});
			CHECK_EQ(odg.operand_var_ids(3), std::vector<char>{'c', 'd'});
			CHECK_EQ(odg.var_ids_positions_in_operands('b'), std::vector<std::vector<pos_type>>{{1}, {0}, {}, {}});
			CHECK_EQ(odg.var_ids_positions_in_operands('c'), std::vector<std::vector<pos_type>>{{}, {1}, {}, {0}});
			// cartesian components
			auto &cart_comps = odg.cartesian_components();
			CHECK_EQ(cart_comps.size(), 2);
			CHECK_EQ(cart_comps[0].size(), 3);
			CHECK_EQ(cart_comps[1].size(), 1);
			CHECK_EQ(cart_comps[0].operands_original_positions(), std::vector<pos_type>{0, 1, 3});
			CHECK_EQ(cart_comps[1].operands_original_positions(), std::vector<pos_type>{2});
			CHECK_EQ(cart_comps[0].operand_var_ids(0), std::vector<char>{'a', 'b'});
			CHECK_EQ(cart_comps[0].operand_var_ids(1), std::vector<char>{'b', 'c'});
			CHECK_EQ(cart_comps[0].operand_var_ids(2), std::vector<char>{'c', 'd'});
			CHECK_EQ(cart_comps[1].operand_var_ids(0), std::vector<char>{'e'});
			// pruning
			auto &pruned1 = odg.prune_graph(std::vector<pos_type>{2});
			auto &pruned2 = odg.prune_graph(std::vector<pos_type>{0});
			auto &pruned3 = odg.prune_graph(std::vector<pos_type>{1});
			auto &pruned4 = odg.prune_graph(std::vector<pos_type>{3});
			CHECK_EQ(pruned1.size(), 0);
			CHECK_EQ(pruned2.size(), 0);
			CHECK_EQ(pruned3.size(), 0);
			CHECK_EQ(pruned4.size(), 0);
			// union components
			CHECK_EQ(odg.union_components().size(), 1);
		}
		SUBCASE("remove label a") {
			odg.isc_operands();// need to instantiate before calling removeLabel
			auto new_odg = odg.remove_var_id('a');
			CHECK_EQ(new_odg.size(), 4);
			CHECK_EQ(new_odg.operand_var_ids(0), std::vector<char>{'b'});
			CHECK_EQ(odg.operand_var_ids(0), std::vector<char>{'a', 'b'});
		}
		SUBCASE("remove label b") {
			odg.isc_operands();// need to instantiate before calling removeLabel
			auto new_odg = odg.remove_var_id('b');
			CHECK_EQ(new_odg.size(), 4);
			CHECK_EQ(new_odg.operand_var_ids(0), std::vector<char>{'a'});
			CHECK_EQ(new_odg.operand_var_ids(1), std::vector<char>{'c'});
			CHECK_EQ(odg.operand_var_ids(0), std::vector<char>{'a', 'b'});
			CHECK_EQ(odg.operand_var_ids(1), std::vector<char>{'b', 'c'});
			CHECK_EQ(new_odg.cartesian_components().size(), 2);
			CHECK_EQ(odg.cartesian_components().size(), 2);
		}
		SUBCASE("remove labels b and c") {
			odg.isc_operands();// need instantiate before calling removeLabel
			auto new_odg = odg.remove_var_id('b');
			new_odg = new_odg.remove_var_id('c');
			CHECK_EQ(new_odg.size(), 3);
			CHECK_EQ(new_odg.operand_var_ids(0), std::vector<char>{'a'});
			CHECK_EQ(new_odg.operand_var_ids(1), std::vector<char>{'e'});
			CHECK_EQ(new_odg.operand_var_ids(2), std::vector<char>{'d'});
			CHECK_EQ(odg.operand_var_ids(0), std::vector<char>{'a', 'b'});
			CHECK_EQ(odg.operand_var_ids(1), std::vector<char>{'b', 'c'});
			CHECK_EQ(new_odg.cartesian_components().size(), 2);
			CHECK_EQ(odg.cartesian_components().size(), 2);
		}
	}

	TEST_CASE("Independent Strong Component") {
		query::OperandDependencyGraph odg{};
		odg.add_operand({'a', 'b'});
		odg.add_operand({'b', 'c'});
		odg.add_operand({'c', 'd'});
		odg.add_dependency(0, 1, 'b');
		odg.add_dependency(1, 0, 'b');
		odg.add_dependency(1, 2, 'c');
		odg.add_dependency(2, 1, 'c');
		SUBCASE("Strongly Connected") {
			auto const &isc_operands = odg.isc_operands();
			CHECK_EQ(isc_operands.size(), odg.size());
			CHECK_EQ(odg.cartesian_components().size(), 1);
		}
		SUBCASE("Not Strongly Connected (2 Components)") {
			odg.add_operand({'d', 'e'});
			odg.add_operand({'e', 'f'});
			odg.add_dependency(2, 3, 'd');
			odg.add_dependency(3, 4, 'e');
			odg.add_dependency(4, 3, 'e');
			auto const &isc_operands = odg.isc_operands();
			CHECK_EQ(isc_operands, std::vector<pos_type>{0, 1, 2});
			CHECK_EQ(odg.cartesian_components().size(), 1);
		}
		SUBCASE("Not Strongly Connected (3 Components)") {
			odg.add_operand({'d', 'e'});
			odg.add_operand({'e', 'f'});
			odg.add_operand({'f'});
			odg.add_dependency(2, 3, 'd');
			odg.add_dependency(3, 4, 'e');
			odg.add_dependency(4, 3, 'e');
			odg.add_dependency(4, 5, 'f');
			auto const &isc_operands = odg.isc_operands();
			CHECK_EQ(isc_operands, std::vector<pos_type>{0, 1, 2});
			CHECK_EQ(odg.cartesian_components().size(), 1);
		}
	}

	TEST_CASE("Perm") {
		std::vector<std::vector<int>> test{{0, 1}, {0, 1, 2}};
		std::vector<int> perm{};
		perm.resize(test.size());
		size_t size = 1;
		for (auto &sub_test : test)
			size *= sub_test.size();
		std::vector<std::vector<int>> perms(size, perm);
		for (size_t i = 0; i < test.size(); i++) {
			size_t cur = 0;
			for (auto &p : perms) {
				p[i] = test[i][cur];
				cur++;
				if (cur == test[i].size())
					cur = 0;
			}
		}
		CHECK(perms.size() == 6);
		std::sort(perms.begin(), perms.end());
		CHECK(perms == std::vector<std::vector<int>>{{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}});
	}

}// namespace dice::einsum::tests