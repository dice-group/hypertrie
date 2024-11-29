#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "dice/hypertrie/Hypertrie_default_traits.hpp"
#include "dice/query.hpp"

#include <algorithm>

namespace dice::query::tests {

	using htt_t = hypertrie::default_bool_Hypertrie_trait;
	using allocator_type = std::allocator<std::byte>;

	bool evaluate(Query<htt_t, allocator_type> &query,
				  std::vector<Key<size_t, htt_t>> &expected_results) {
		std::vector<Key<size_t, htt_t>> actual_results{};
		for (const auto &res : Evaluation::evaluate<htt_t, allocator_type>(query)) {
			std::cout << static_cast<std::string>(res) << std::endl;
			for (size_t i = 0; i < res.value(); i++) {
				actual_results.emplace_back(res.key());
			}
		}
		std::sort(expected_results.begin(), expected_results.end());
		std::sort(actual_results.begin(), actual_results.end());
		return (actual_results == expected_results);
	}

	bool evaluate_distinct(Query<htt_t, allocator_type> &query,
						   std::vector<Key<size_t, htt_t>> &expected_results) {
		std::vector<Key<size_t, htt_t>> actual_results{};
		Key<size_t, htt_t> entry;
		entry.resize(query.projected_vars().size());
		for (auto const &res : Evaluation::evaluate<htt_t, allocator_type , true>(query)) {
			std::copy(res.key().begin(), res.key().end(), entry.begin());
			std::cout << static_cast<std::string>(res) << std::endl;
			for (size_t i = 0; i < res.value(); i++) {
				actual_results.emplace_back(entry);
			}
		}
		std::sort(expected_results.begin(), expected_results.end());
		std::sort(actual_results.begin(), actual_results.end());
		return (actual_results == expected_results);
	}

	TEST_CASE("Join") {
		SUBCASE("Simple Join") {
			dice::query::OperandDependencyGraph odg{};
			odg.add_operand({'a', 'b'});
			odg.add_operand({'b', 'c'});
			odg.add_dependency(0, 1, 'b');
			odg.add_dependency(1, 0, 'b');
			hypertrie::Hypertrie<htt_t, allocator_type> ht1{2};
			hypertrie::Hypertrie<htt_t, allocator_type> ht2{2};
			ht1.set({1, 2}, true);
			ht1.set({10, 5}, true);
			ht2.set({2, 5}, true);
			ht2.set({2, 6}, true);
			Query<htt_t, allocator_type> query{odg, {ht1, ht2}, {'a', 'b', 'c'}};
			std::vector<Key<size_t, htt_t>> expected_results = {
					{1, 2, 5},
					{1, 2, 6}};
			CHECK(evaluate(query, expected_results));
		}
	}

	TEST_CASE("Left Join") {
		hypertrie::Hypertrie<htt_t, allocator_type> ht{3};
		ht.set({1, 10, 20}, true);
		ht.set({2, 10, 20}, true);
		ht.set({3, 10, 20}, true);
		ht.set({4, 10, 20}, true);
		ht.set({5, 10, 20}, true);
		ht.set({6, 10, 20}, true);
		ht.set({7, 10, 20}, true);
		ht.set({8, 10, 20}, true);
		ht.set({1, 11, 3}, true);
		ht.set({1, 11, 6}, true);
		ht.set({2, 11, 4}, true);
		ht.set({1, 12, 8}, true);
		ht.set({4, 12, 6}, true);
		ht.set({3, 12, 5}, true);
		ht.set({4, 13, 25}, true);
		ht.set({3, 13, 25}, true);
		ht.set({5, 14, 30}, true);
		ht.set({1, 14, 35}, true);
		ht.set({8, 14, 30}, true);
		ht.set({1, 15, 40}, true);
		ht.set({6, 15, 55}, true);
		ht.set({2, 16, 45}, true);
		ht.set({3, 16, 50}, true);
		detail::SliceKey<htt_t> s_key1{std::nullopt, 10, 20};
		detail::SliceKey<htt_t> s_key2{std::nullopt, 11, std::nullopt};
		detail::SliceKey<htt_t> s_key3{std::nullopt, 12, std::nullopt};
		detail::SliceKey<htt_t> s_key4{std::nullopt, 13, std::nullopt};
		detail::SliceKey<htt_t> s_key5{std::nullopt, 14, std::nullopt};
		detail::SliceKey<htt_t> s_key6{std::nullopt, 15, std::nullopt};
		detail::SliceKey<htt_t> s_key7{std::nullopt, 16, std::nullopt};
		auto ht1 = std::get<0>(ht[s_key1]);
		auto ht2 = std::get<0>(ht[s_key2]);
		auto ht3 = std::get<0>(ht[s_key3]);
		auto ht4 = std::get<0>(ht[s_key4]);
		auto ht5 = std::get<0>(ht[s_key5]);
		auto ht6 = std::get<0>(ht[s_key6]);
		auto ht7 = std::get<0>(ht[s_key7]);
		auto default_key_part = std::numeric_limits<htt_t::key_part_type>::min();
		SUBCASE("Single Right Operand") {
			SUBCASE("LJ(a, ab), Project: a") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_dependency(0, 1, 'a');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2}, {'a'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1},
						{1},
						{2},
						{3},
						{4},
						{5},
						{6},
						{7},
						{8}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a, ab), Project: ab") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_dependency(0, 1, 'a');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2}, {'a', 'b'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3},
						{1, 6},
						{2, 4},
						{3, default_key_part},
						{4, default_key_part},
						{5, default_key_part},
						{6, default_key_part},
						{7, default_key_part},
						{8, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(ab, bc), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_dependency(0, 1, 'b');
				Query<htt_t, allocator_type> query{odg, {ht2, ht3}, {'a', 'b', 'c'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5},
						{1, 6, default_key_part},
						{2, 4, 6}};
				CHECK(evaluate(query, expected_results));
			}
		}
		SUBCASE("Multiple Right Operands") {
			SUBCASE("LJ(LJ(a, ab), ac)), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'a', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2, 'a');
				odg.add_connection(1, 2);
				odg.add_connection(2, 1);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3}, {'a', 'b', 'c'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 8},
						{1, 6, 8},
						{2, 4, default_key_part},
						{3, default_key_part, 5},
						{4, default_key_part, 6},
						{5, default_key_part, default_key_part},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(LJ(LJ(a, ab), ac) ad), Project: abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'a', 'c'});
				odg.add_operand({'a', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2, 'a');
				odg.add_dependency(0, 3, 'a');
				odg.add_connection(1, 2);
				odg.add_connection(2, 1);
				odg.add_connection(1, 3);
				odg.add_connection(3, 1);
				odg.add_connection(2, 3);
				odg.add_connection(3, 2);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 8, 35},
						{1, 6, 8, 35},
						{2, 4, default_key_part, default_key_part},
						{3, default_key_part, 5, default_key_part},
						{4, default_key_part, 6, default_key_part},
						{5, default_key_part, default_key_part, 30},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, 30}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(LJ(ab, ac), bd)), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a', 'b'});
				odg.add_operand({'a', 'c'});
				odg.add_operand({'b', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2, 'b');
				odg.add_connection(1, 2);
				odg.add_connection(2, 1);
				Query<htt_t, allocator_type> query{odg, {ht2, ht3, ht4}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 8, 25},
						{1, 6, 8, default_key_part},
						{2, 4, default_key_part, 25}};
				CHECK(evaluate(query, expected_results));
			}
		}
		SUBCASE("Nested Operations") {
			SUBCASE("LJ(a, LJ(ab, bc)), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 2, 'b');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3}, {'a', 'b', 'c'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5},
						{1, 6, default_key_part},
						{2, 4, 6},
						{3, default_key_part, default_key_part},
						{4, default_key_part, default_key_part},
						{5, default_key_part, default_key_part},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a, LJ(ab, LJ(bc, cd)), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'c', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 3, 'c');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5, 30},
						{1, 6, default_key_part, default_key_part},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a, LJ(ab, ac)), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'a', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 2, 'a');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3}, {'a', 'b', 'c'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 8},
						{1, 6, 8},
						{2, 4, default_key_part},
						{3, default_key_part, default_key_part},
						{4, default_key_part, default_key_part},
						{5, default_key_part, default_key_part},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a, LJ(ab, LJ(ac, ad)), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'a', 'c'});
				odg.add_operand({'a', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 2, 'a');
				odg.add_dependency(2, 3, 'a');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht6, ht7}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 40, default_key_part},
						{1, 6, 40, default_key_part},
						{2, 4, default_key_part, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
		}
	}

	TEST_CASE("Join Combinations") {
		hypertrie::Hypertrie<htt_t, allocator_type> ht{3};
		ht.set({1, 10, 20}, true);
		ht.set({2, 10, 20}, true);
		ht.set({3, 10, 20}, true);
		ht.set({4, 10, 20}, true);
		ht.set({5, 10, 20}, true);
		ht.set({6, 10, 20}, true);
		ht.set({7, 10, 20}, true);
		ht.set({8, 10, 20}, true);
		ht.set({1, 11, 3}, true);
		ht.set({1, 11, 6}, true);
		ht.set({2, 11, 4}, true);
		ht.set({5, 11, 7}, true);
		ht.set({1, 12, 8}, true);
		ht.set({4, 12, 6}, true);
		ht.set({3, 12, 5}, true);
		ht.set({4, 13, 25}, true);
		ht.set({6, 13, 25}, true);
		ht.set({5, 14, 30}, true);
		ht.set({1, 14, 35}, true);
		ht.set({8, 14, 30}, true);
		ht.set({8, 15, 35}, true);
		ht.set({7, 15, 30}, true);
		detail::SliceKey<htt_t> s_key1{std::nullopt, 10, 20};
		detail::SliceKey<htt_t> s_key2{std::nullopt, 11, std::nullopt};
		detail::SliceKey<htt_t> s_key3{std::nullopt, 12, std::nullopt};
		detail::SliceKey<htt_t> s_key4{std::nullopt, 13, std::nullopt};
		detail::SliceKey<htt_t> s_key5{std::nullopt, 14, std::nullopt};
		detail::SliceKey<htt_t> s_key6{std::nullopt, 16, std::nullopt};
		detail::SliceKey<htt_t> s_key7{std::nullopt, 15, std::nullopt};
		auto ht1 = std::get<0>(ht[s_key1]);
		auto ht2 = std::get<0>(ht[s_key2]);
		auto ht3 = std::get<0>(ht[s_key3]);
		auto ht4 = std::get<0>(ht[s_key4]);
		auto ht5 = std::get<0>(ht[s_key5]);
		auto ht6 = std::get<0>(ht[s_key6]);
		auto ht7 = std::get<0>(ht[s_key7]);
		auto default_key_part = std::numeric_limits<htt_t::key_part_type>::min();
		SUBCASE("Join and Left Join") {
			SUBCASE("LJ(J(a,ab), bc), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 0, 'a');
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(0, 2);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3}, {'a', 'b', 'c'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5},
						{1, 6, default_key_part},
						{2, 4, 6},
						{5, 7, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(J(a,ab), ac), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'a', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 0, 'a');
				odg.add_dependency(0, 2, 'a');
				odg.add_dependency(1, 2, 'a');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3}, {'a', 'b', 'c'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 8},
						{1, 6, 8},
						{2, 4, default_key_part},
						{5, 7, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a, J(ab,bc)), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2);
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3}, {'a', 'b', 'c'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5},
						{2, 4, 6},
						{3, default_key_part, default_key_part},
						{4, default_key_part, default_key_part},
						{5, default_key_part, default_key_part},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a, J(ab,bc)), Project: a") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2);
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3}, {'a'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1},
						{2},
						{3},
						{4},
						{5},
						{6},
						{7},
						{8}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(J(a,J(ab, bc)), cd), Project: abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'c', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 0, 'a');
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(0, 2);
				odg.add_dependency(2, 0);
				odg.add_dependency(0, 3);
				odg.add_dependency(1, 3);
				odg.add_dependency(2, 3, 'c');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5, 30},
						{2, 4, 6, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a,LJ(J(ab, bc)), cd), Project: abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'c', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2);
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(1, 3);
				odg.add_dependency(2, 3, 'c');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5, 30},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a,J(ab, ac)), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'a', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2, 'a');
				odg.add_dependency(1, 2, 'a');
				odg.add_dependency(2, 1, 'a');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht5}, {'a', 'b', 'c'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 35},
						{1, 6, 35},
						{2, default_key_part, default_key_part},
						{3, default_key_part, default_key_part},
						{4, default_key_part, default_key_part},
						{5, 7, 30},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a,J(ab, ac)), Project: abc") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'a', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2, 'a');
				odg.add_dependency(1, 2, 'a');
				odg.add_dependency(2, 1, 'a');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht5}, {'a'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1},
						{1},
						{2},
						{3},
						{4},
						{5},
						{6},
						{7},
						{8}};
				std::vector<Key<size_t, htt_t>> expected_results_dist = {
						{1},
						{2},
						{3},
						{4},
						{5},
						{6},
						{7},
						{8}};
				CHECK(evaluate(query, expected_results));
				CHECK(evaluate_distinct(query, expected_results_dist));
			}
			SUBCASE("J(LJ(a,J(ab, bc)), ad), Project: abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'a', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2);
				odg.add_dependency(3, 1, 'a');
				odg.add_dependency(3, 2);
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(0, 3, 'a');
				odg.add_dependency(3, 0, 'a');
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5, 35},
						{5, default_key_part, default_key_part, 30},
						{8, default_key_part, default_key_part, 30}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(LJ(a,J(ab, bc)), ad), Project: abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'a', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2);
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(0, 3, 'a');
				odg.add_connection(2, 3);
				odg.add_connection(3, 2);
				odg.add_connection(1, 3);
				odg.add_connection(3, 1);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5, 35},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, 30},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, 30}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a,LJ(J(ab, bc), ad)), Project: abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'a', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(1, 3, 'a');
				odg.add_dependency(2, 3);
				odg.add_dependency(1, 3);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5, 35},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a,LJ(J(ab,J(bc,ce)),ad)), Project: abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'c', 'e'});
				odg.add_operand({'a', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(2, 3, 'c');
				odg.add_dependency(3, 2, 'c');
				odg.add_dependency(1, 3);
				odg.add_dependency(3, 1);
				odg.add_dependency(1, 4, 'a');
				odg.add_dependency(2, 4);
				odg.add_dependency(3, 4);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht4, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, default_key_part, default_key_part, default_key_part},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(J(a,J(ab,bc)),ad), Project: abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'a', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 0, 'a');
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(0, 2);
				odg.add_dependency(2, 0);
				odg.add_dependency(1, 3, 'a');
				odg.add_dependency(2, 3);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5, 35},
						{2, 4, 6, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(LJ(J(a,J(ab,bc)),ad),be), Project: abcde") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'a', 'd'});
				odg.add_operand({'b', 'e'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 0, 'a');
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(1, 3, 'a');
				odg.add_dependency(2, 3);
				odg.add_dependency(2, 4, 'b');
				odg.add_connection(3, 4);
				odg.add_connection(4, 3);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5, ht4}, {'a', 'b', 'c', 'd', 'e'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5, 35, default_key_part},
						{2, 4, 6, default_key_part, 25}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("J(LJ(J(a,J(ab,bc)),ad),be), Project: abcde") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'a', 'd'});
				odg.add_operand({'b', 'e'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 0, 'a');
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(1, 3, 'a');
				odg.add_dependency(2, 3);
				odg.add_dependency(2, 4, 'b');
				odg.add_dependency(4, 2, 'b');
				odg.add_dependency(4, 3);
				odg.add_dependency(4, 0);
				odg.add_dependency(0, 4);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5, ht4}, {'a', 'b', 'c', 'd', 'e'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{2, 4, 6, default_key_part, 25}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(J(a,J(ab,J(bc,ce))),bd), Project: abcde") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'c', 'e'});
				odg.add_operand({'b', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 0, 'a');
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(2, 3, 'c');
				odg.add_dependency(3, 2, 'c');
				odg.add_dependency(1, 3);
				odg.add_dependency(3, 1);
				odg.add_dependency(0, 3);
				odg.add_dependency(3, 0);
				odg.add_dependency(2, 4, 'b');
				odg.add_dependency(3, 4);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht3, ht5, ht4}, {'a', 'b', 'c', 'd', 'e'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, 3, 5, default_key_part, 30}};
				CHECK(evaluate(query, expected_results));
			}
		}
		SUBCASE("Left Join and Cartesian") {
			SUBCASE("LJ(a, J(bc,cd)), Project: abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'c', 'd'});
				odg.add_dependency(0, 1);
				odg.add_dependency(0, 2);
				odg.add_dependency(1, 2, 'c');
				odg.add_dependency(2, 1, 'c');
				Query<htt_t, allocator_type> query{odg, {ht1, ht4, ht5}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, default_key_part, default_key_part, default_key_part},
						{2, default_key_part, default_key_part, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(LJ(J(ab,bh),cd),J(eg,ef)), Project: abcdef") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'h'});
				odg.add_operand({'c', 'd'});
				odg.add_operand({'e', 'g'});
				odg.add_operand({'e', 'f'});
				// joins
				odg.add_dependency(0, 1, 'b');
				odg.add_dependency(1, 0, 'b');
				odg.add_dependency(3, 4, 'e');
				odg.add_dependency(4, 3, 'e');
				// cartesian optional
				odg.add_connection(0, 2);
				odg.add_connection(1, 2);
				odg.add_connection(2, 0);
				odg.add_connection(2, 1);
				odg.add_connection(2, 3);
				odg.add_connection(2, 4);
				odg.add_connection(3, 2);
				odg.add_connection(4, 2);
				odg.add_connection(0, 3);
				odg.add_connection(3, 0);
				odg.add_connection(0, 4);
				odg.add_connection(4, 0);
				odg.add_connection(1, 3);
				odg.add_connection(3, 1);
				odg.add_connection(1, 4);
				odg.add_connection(4, 1);
				Query<htt_t, allocator_type> query{odg, {ht2, ht5, ht3, ht4, ht5}, {'a', 'b', 'c', 'd', 'e', 'f'}};
				std::vector<Key<size_t, htt_t>> expected_results1 = {
						{default_key_part, default_key_part}};
				std::vector<Key<size_t, htt_t>> expected_results2 = {
						{1, 8},
						{4, 6},
						{3, 5}};
				std::vector<Key<size_t, htt_t>> expected_results3 = {
						{default_key_part, default_key_part}};
				std::vector<Key<size_t, htt_t>> expected_results{};
				for (auto &res1 : expected_results1) {
					for (auto &res2 : expected_results2) {
						for (auto &res3 : expected_results3) {
							auto temp_res = res1;
							temp_res.insert(temp_res.end(), res2.begin(), res2.end());
							temp_res.insert(temp_res.end(), res3.begin(), res3.end());
							expected_results.push_back(temp_res);
						}
					}
				}
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(a,J(ab,cd)), Project:ab") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'d', 'c'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(1, 2);
				odg.add_dependency(2, 1);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht6}, {'a', 'b'}};
				std::vector<Key<size_t, htt_t>> expected_results = {
						{1, default_key_part},
						{2, default_key_part},
						{3, default_key_part},
						{4, default_key_part},
						{5, default_key_part},
						{6, default_key_part},
						{7, default_key_part},
						{8, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("J(LJ(a,(ab),LJ(c,cd)), Project:abcd") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'c'});
				odg.add_operand({'c', 'd'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2);
				odg.add_dependency(2, 0);
				odg.add_dependency(2, 3, 'c');
				odg.add_connection(1, 3);
				odg.add_connection(3, 1);
				Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht1, ht3}, {'a', 'b', 'c', 'd'}};
				std::vector<Key<size_t, htt_t>> expected_results1 = {
						{1, 3},
						{1, 6},
						{2, 4},
						{3, default_key_part},
						{4, default_key_part},
						{5, 7},
						{6, default_key_part},
						{7, default_key_part},
						{8, default_key_part}};
				std::vector<Key<size_t, htt_t>> expected_results2 = {
						{1, 8},
						{2, default_key_part},
						{3, 5},
						{4, 6},
						{5, default_key_part},
						{6, default_key_part},
						{7, default_key_part},
						{8, default_key_part}};
				std::vector<Key<size_t, htt_t>> expected_results{};
				for (auto &res1 : expected_results1) {
					for (auto &res2 : expected_results2) {
						auto temp_res = res1;
						temp_res.insert(temp_res.end(), res2.begin(), res2.end());
						expected_results.push_back(temp_res);
					}
				}
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(LJ(J(ab,bh),cd),J(eg,ef)), Project:abcdef") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'h'});
				odg.add_operand({'c', 'd'});
				odg.add_operand({'e', 'g'});
				odg.add_operand({'e', 'f'});
				odg.add_dependency(0, 1, 'b');
				odg.add_dependency(1, 0, 'b');
				odg.add_dependency(1, 2);
				odg.add_dependency(1, 3);
				odg.add_connection(2, 3);
				odg.add_connection(3, 2);
				odg.add_dependency(3, 4, 'e');
				odg.add_dependency(4, 3, 'e');
				Query<htt_t, allocator_type> query{odg, {ht2, ht5, ht3, ht4, ht5}, {'a', 'b', 'c', 'd', 'e', 'f'}};
				std::vector<Key<size_t, htt_t>> expected_results{};
				CHECK(evaluate(query, expected_results));
			}
			SUBCASE("LJ(LJ(a,LJ(J(ab,bc),ad)),LJ(J(ae,ef),ag)), Project:abdeg") {
				dice::query::OperandDependencyGraph odg{};
				odg.add_operand({'a'});
				odg.add_operand({'a', 'b'});
				odg.add_operand({'b', 'c'});
				odg.add_operand({'a', 'd'});
				odg.add_operand({'a', 'e'});
				odg.add_operand({'e', 'f'});
				odg.add_operand({'a', 'g'});
				odg.add_dependency(0, 1, 'a');
				odg.add_dependency(0, 2);
				odg.add_dependency(1, 2, 'b');
				odg.add_dependency(2, 1, 'b');
				odg.add_dependency(1, 3, 'a');
				odg.add_dependency(2, 3);
				odg.add_dependency(0, 4, 'a');
				odg.add_dependency(0, 5);
				odg.add_dependency(4, 5, 'e');
				odg.add_dependency(5, 4, 'e');
				odg.add_dependency(4, 6, 'a');
				odg.add_dependency(5, 6);
				odg.add_connection(1, 4);
				odg.add_connection(4, 1);
				odg.add_connection(2, 4);
				odg.add_connection(4, 2);
				odg.add_connection(1, 5);
				odg.add_connection(5, 1);
				odg.add_connection(2, 5);
				odg.add_connection(5, 2);
				Query<htt_t, allocator_type> query{odg, {ht1, ht3, ht2, ht5, ht2, ht4, ht5}, {'a', 'b', 'd', 'e', 'g'}};
				std::vector<Key<size_t, htt_t>> expected_results{
						{1, default_key_part, default_key_part, 6, 35},
						{2, default_key_part, default_key_part, 4, default_key_part},
						{3, 5, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, expected_results));
			}
		}
	}

	TEST_CASE("Union") {
		auto default_key_part = std::numeric_limits<htt_t::key_part_type>::min();
		hypertrie::Hypertrie<htt_t, allocator_type> ht{3};
		ht.set({1, 10, 20}, true);
		ht.set({2, 10, 20}, true);
		ht.set({3, 10, 20}, true);
		ht.set({4, 10, 20}, true);
		ht.set({5, 10, 20}, true);
		ht.set({6, 10, 20}, true);
		ht.set({7, 10, 20}, true);
		ht.set({8, 10, 20}, true);
		ht.set({1, 11, 3}, true);
		ht.set({1, 11, 6}, true);
		ht.set({2, 11, 4}, true);
		ht.set({1, 12, 8}, true);
		ht.set({4, 12, 6}, true);
		ht.set({3, 12, 5}, true);
		ht.set({5, 13, 50}, true);
		ht.set({6, 13, 50}, true);
		ht.set({2, 13, 60}, true);
		ht.set({4, 13, 60}, true);
		detail::SliceKey<htt_t> s_key1{std::nullopt, 10, 20};
		detail::SliceKey<htt_t> s_key2{std::nullopt, 11, std::nullopt};
		detail::SliceKey<htt_t> s_key3{std::nullopt, 12, std::nullopt};
		detail::SliceKey<htt_t> s_key4{std::nullopt, 13, 50};
		detail::SliceKey<htt_t> s_key5{std::nullopt, 13, 60};
		detail::SliceKey<htt_t> s_key6{std::nullopt, 13, std::nullopt};
		auto ht1 = std::get<0>(ht[s_key1]);
		auto ht2 = std::get<0>(ht[s_key2]);
		auto ht3 = std::get<0>(ht[s_key3]);
		auto ht4 = std::get<0>(ht[s_key4]);
		auto ht5 = std::get<0>(ht[s_key5]);
		auto ht6 = std::get<0>(ht[s_key6]);
		SUBCASE("U(a,b), Project: a)") {
			// operand for 'a' is empty, still need to produce a mapping with 'a' being unbound
			detail::SliceKey<htt_t> s_key_empty{std::nullopt, 16, 20};
			detail::SliceKey<htt_t> s_key_nonempty{1, 11, std::nullopt};
			auto ht_empty = std::get<0>(ht[s_key_empty]);
			auto ht_nonempty = std::get<0>(ht[s_key_nonempty]);
			dice::query::OperandDependencyGraph odg{};
			odg.add_operand({'a'});
			odg.add_operand({'b'});
			Query<htt_t, allocator_type> query{odg, {ht_empty, ht_nonempty}, {'a'}};
			std::vector<Key<size_t, htt_t>> expected_results = {
					{default_key_part}};
			CHECK(evaluate_distinct(query, expected_results));
		}
		SUBCASE("J(a,U(ab,ac)), Project: abc") {
			dice::query::OperandDependencyGraph odg{};
			odg.add_operand({'a'});
			odg.add_operand({'a', 'b'});
			odg.add_operand({'a'});
			odg.add_operand({'a', 'c'});
			odg.add_dependency(0, 1, 'a');
			odg.add_dependency(1, 0, 'a');
			odg.add_dependency(2, 3, 'a');
			odg.add_dependency(3, 2, 'a');
			Query<htt_t, allocator_type> query{odg, {ht1, ht2, ht1, ht3}, {'a', 'b', 'c'}};
			std::vector<Key<size_t, htt_t>> expected_results = {
					{1, 3, default_key_part},
					{1, 6, default_key_part},
					{2, 4, default_key_part},
					{1, default_key_part, 8},
					{4, default_key_part, 6},
					{3, default_key_part, 5}};
			CHECK(evaluate(query, expected_results));
		}
		SUBCASE("J(ab,U(a,a)), Project: ab") {
			dice::query::OperandDependencyGraph odg{};
			odg.add_operand({'a', 'b'});
			odg.add_operand({'a'});
			odg.add_operand({'a', 'b'});
			odg.add_operand({'a'});
			odg.add_dependency(0, 1, 'a');
			odg.add_dependency(1, 0, 'a');
			odg.add_dependency(2, 3, 'a');
			odg.add_dependency(3, 2, 'a');
			Query<htt_t, allocator_type> query{odg, {ht2, ht4, ht2, ht5}, {'a', 'b'}};
			std::vector<Key<size_t, htt_t>> expected_results = {
					{2, 4}};
			CHECK(evaluate(query, expected_results));
		}
		SUBCASE("U(ab,ac), Project: abc") {
			dice::query::OperandDependencyGraph odg{};
			odg.add_operand({'a', 'b'});
			odg.add_operand({'a', 'c'});
			Query<htt_t, allocator_type> query{odg, {ht2, ht3}, {'a', 'b', 'c'}};
			std::vector<Key<size_t, htt_t>> expected_results = {
					{1, 3, default_key_part},
					{1, 6, default_key_part},
					{2, 4, default_key_part},
					{1, default_key_part, 8},
					{4, default_key_part, 6},
					{3, default_key_part, 5}};
			CHECK(evaluate(query, expected_results));
		}
		SUBCASE("LJ(U(ab,ac),ad), Project: abcd") {
			dice::query::OperandDependencyGraph odg{};
			odg.add_operand({'a', 'b'});
			odg.add_operand({'a', 'd'});
			odg.add_operand({'a', 'c'});
			odg.add_operand({'a', 'd'});
			odg.add_dependency(0, 1, 'a');
			odg.add_dependency(2, 3, 'a');
			Query<htt_t, allocator_type> query{odg, {ht2, ht6, ht3, ht6}, {'a', 'b', 'c', 'd'}};
			std::vector<Key<size_t, htt_t>> expected_results = {
					{1, 3, default_key_part, default_key_part},
					{1, 6, default_key_part, default_key_part},
					{2, 4, default_key_part, 60},
					{1, default_key_part, 8, default_key_part},
					{4, default_key_part, 6, 60},
					{3, default_key_part, 5, default_key_part}};
			CHECK(evaluate(query, expected_results));
		}
	}
}// namespace dice::query::tests