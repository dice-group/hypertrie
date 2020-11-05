#include <Dice/hypertrie/hypertrie.hpp>
#include <catch2/catch.hpp>

namespace hypertrie::tests::leftjoin {

	TEST_CASE("join-combination") {

		Hypertrie<default_bool_Hypertrie_t> ht{3};
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
		auto default_key_part = std::numeric_limits<default_bool_Hypertrie_t::key_part_type>::max();
		SliceKey<unsigned long> s_key1{std::nullopt, 10, 20};
		SliceKey<unsigned long> s_key2{std::nullopt, 11, std::nullopt};
		SliceKey<unsigned long> s_key3{std::nullopt, 12, std::nullopt};
		SliceKey<unsigned long> s_key4{std::nullopt, 13, std::nullopt};
		SliceKey<unsigned long> s_key5{std::nullopt, 14, std::nullopt};
		auto ht1 = std::get<0>(ht[s_key1]);
		auto ht2 = std::get<0>(ht[s_key2]);
		auto ht3 = std::get<0>(ht[s_key3]);
		auto ht4 = std::get<0>(ht[s_key4]);
		auto ht5 = std::get<0>(ht[s_key5]);
		std::vector<const_Hypertrie<default_bool_Hypertrie_t>> operands{};
		std::vector<std::vector<char>> operands_labels{};
		std::vector<std::vector<default_bool_Hypertrie_t::key_part_type>> expected_results;
		std::vector<char> result_labels{};

        // a,ab,[bc]->abc
        SECTION("j_lj_dl", "join before left join, on different labels") {
            operands.push_back(ht1);
            operands.push_back(ht2);
            operands.push_back(ht3);
            std::vector<char> op1_labels{'a'};
            std::vector<char> op2_labels{'a', 'b'};
            std::vector<char> op3_labels{'[', 'b', 'c', ']'};
            operands_labels.push_back(op1_labels);
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
            expected_results = {
                    {1, 3, 5},
                    {1, 6, default_key_part},
                    {2, 4, 6},
                    {5, 7, default_key_part}
            };
        }
		// a,[ab,bc]->abc
        SECTION("lj_j_dl" , "join after left join, on different labels") {
            operands.push_back(ht1);
            operands.push_back(ht2);
            operands.push_back(ht3);
            std::vector<char> op1_labels{'a'};
            std::vector<char> op2_labels{'[', 'a', 'b'};
            std::vector<char> op3_labels{'b', 'c', ']'};
            operands_labels.push_back(op1_labels);
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
            expected_results = {
                    {1, 3, 5},
                    {2, 4, 6},
                    {3, default_key_part, default_key_part},
                    {4, default_key_part, default_key_part},
                    {5, default_key_part, default_key_part},
                    {6, default_key_part, default_key_part},
                    {7, default_key_part, default_key_part},
                    {8, default_key_part, default_key_part},
            };
        }
		// a,ab,bc,[cd]->abcd
        SECTION("jj_lj_dl", "two joins before left join, on different labels") {
            operands.push_back(ht1);
            operands.push_back(ht2);
            operands.push_back(ht3);
			operands.push_back(ht5);
            std::vector<char> op1_labels{'a'};
            std::vector<char> op2_labels{'a', 'b'};
            std::vector<char> op3_labels{'b', 'c'};
            std::vector<char> op4_labels{'[', 'c', 'd', ']'};
            operands_labels.push_back(op1_labels);
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
			operands_labels.push_back(op4_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
			result_labels.push_back('d');
            expected_results = {
                    {1, 3, 5, 30},
                    {2, 4, 6, default_key_part}
            };
        }
        // a,[ab,bc,[cd]]->abcd
        SECTION("lj_j_nlj_dl", "join after left join and nested left join, on different labels") {
            operands.push_back(ht1);
            operands.push_back(ht2);
            operands.push_back(ht3);
            operands.push_back(ht5);
            std::vector<char> op1_labels{'a'};
            std::vector<char> op2_labels{'[','a', 'b'};
            std::vector<char> op3_labels{'b', 'c'};
            std::vector<char> op4_labels{'[', 'c', 'd', ']', ']'};
            operands_labels.push_back(op1_labels);
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
            operands_labels.push_back(op4_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
            result_labels.push_back('d');
            expected_results = {
                    {1, 3, 5, 30},
                    {2, 4, 6, default_key_part},
                    {3, default_key_part, default_key_part, default_key_part},
                    {4, default_key_part, default_key_part, default_key_part},
                    {5, default_key_part, default_key_part, default_key_part},
                    {6, default_key_part, default_key_part, default_key_part},
                    {7, default_key_part, default_key_part, default_key_part},
                    {8, default_key_part, default_key_part, default_key_part}
            };
        }
        // a,[ab,ac]->abc
        SECTION("lj_j", "join after left join, on same labels") {
            operands.push_back(ht1);
            operands.push_back(ht2);
            operands.push_back(ht5);
            std::vector<char> op1_labels{'a'};
            std::vector<char> op2_labels{'[','a', 'b'};
            std::vector<char> op3_labels{'a', 'c', ']'};
            operands_labels.push_back(op1_labels);
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
            expected_results = {
                    {1, 3, 35},
                    {1, 6, 35},
                    {2, default_key_part, default_key_part},
                    {3, default_key_part, default_key_part},
                    {4, default_key_part, default_key_part},
                    {5, 7, 30},
                    {6, default_key_part, default_key_part},
                    {7, default_key_part, default_key_part},
                    {8, default_key_part, default_key_part}
            };
        }
        // a,[ab,bc],[ad]->abcd
        SECTION("mlj_jdl", "multiple left joins on the same label, join after left join") {
            operands.push_back(ht1);
            operands.push_back(ht2);
            operands.push_back(ht3);
            operands.push_back(ht5);
            std::vector<char> op1_labels{'a'};
            std::vector<char> op2_labels{'[','a', 'b'};
            std::vector<char> op3_labels{'b', 'c', ']'};
            std::vector<char> op4_labels{'[', 'a', 'd', ']'};
            operands_labels.push_back(op1_labels);
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
            operands_labels.push_back(op4_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
            result_labels.push_back('d');
            expected_results = {
                    {1, 3, 5, 35},
                    {2, 4, 6, default_key_part},
                    {3, default_key_part, default_key_part, default_key_part},
                    {4, default_key_part, default_key_part, default_key_part},
                    {5, default_key_part, default_key_part, 30},
                    {6, default_key_part, default_key_part, default_key_part},
                    {7, default_key_part, default_key_part, default_key_part},
                    {8, default_key_part, default_key_part, 30}
            };
        }
        // a,[ab,bc,[ad]]->abcd
        SECTION("lj_jdl_nljsl", "join after left join on different label, nested left join on same label") {
            operands.push_back(ht1);
            operands.push_back(ht2);
            operands.push_back(ht3);
            operands.push_back(ht5);
            std::vector<char> op1_labels{'a'};
            std::vector<char> op2_labels{'[','a', 'b'};
            std::vector<char> op3_labels{'b', 'c'};
            std::vector<char> op4_labels{'[', 'a', 'd', ']', ']'};
            operands_labels.push_back(op1_labels);
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
            operands_labels.push_back(op4_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
            result_labels.push_back('d');
            expected_results = {
                    {1, 3, 5, 35},
                    {2, 4, 6, default_key_part},
                    {3, default_key_part, default_key_part, default_key_part},
                    {4, default_key_part, default_key_part, default_key_part},
                    {5, default_key_part, default_key_part, default_key_part},
                    {6, default_key_part, default_key_part, default_key_part},
                    {7, default_key_part, default_key_part, default_key_part},
                    {8, default_key_part, default_key_part, default_key_part}
            };
        }
        // a,[ab,bc,ce,[ad]]->abcd
        SECTION("lj_mjdl_nljsl", "multiple joins after left join on different label, nested left join on same label") {
            operands.push_back(ht1);
            operands.push_back(ht2);
            operands.push_back(ht3);
			operands.push_back(ht4);
            operands.push_back(ht5);
            std::vector<char> op1_labels{'a'};
            std::vector<char> op2_labels{'[','a', 'b'};
            std::vector<char> op3_labels{'b', 'c'};
            std::vector<char> op4_labels{'c', 'e'};
            std::vector<char> op5_labels{'[', 'a', 'd', ']', ']'};
            operands_labels.push_back(op1_labels);
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
            operands_labels.push_back(op4_labels);
            operands_labels.push_back(op5_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
            result_labels.push_back('d');
            expected_results = {
                    {1, default_key_part, default_key_part, default_key_part},
                    {2, 4, 6, default_key_part},
                    {3, default_key_part, default_key_part, default_key_part},
                    {4, default_key_part, default_key_part, default_key_part},
                    {5, default_key_part, default_key_part, default_key_part},
                    {6, default_key_part, default_key_part, default_key_part},
                    {7, default_key_part, default_key_part, default_key_part},
                    {8, default_key_part, default_key_part, default_key_part}
            };
        }
		// [ab],ac->abc
        SECTION("no_left_op", "left join without left operand") {
            operands.push_back(ht2);
            operands.push_back(ht3);
            std::vector<char> op2_labels{'[', 'a', 'b', ']'};
            std::vector<char> op3_labels{'b', 'c'};
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
            expected_results = {
                    {1, 3, 5},
                    {2, 4, 6}
            };
        }
        // a,[bc,[ad]]->abcd
        SECTION("no_wd", "non well defined") {
			operands.push_back(ht1);
            operands.push_back(ht4);
            operands.push_back(ht3);
            std::vector<char> op1_labels{'a'};
            std::vector<char> op2_labels{'[', 'b', 'c'};
            std::vector<char> op3_labels{'[', 'a', 'd', ']', ']'};
            operands_labels.push_back(op1_labels);
            operands_labels.push_back(op2_labels);
            operands_labels.push_back(op3_labels);
            result_labels.push_back('a');
            result_labels.push_back('b');
            result_labels.push_back('c');
            result_labels.push_back('d');
            expected_results = {
                    {1, 4, 25, 8},
                    {2, 4, 25, default_key_part},
                    {3, 4, 25, 5},
                    {4, 4, 25, 6},
                    {5, 4, 25, default_key_part},
                    {6, 4, 25, default_key_part},
                    {7, 4, 25, default_key_part},
                    {8, 4, 25, default_key_part},
                    {1, 6, 25, 8},
                    {2, 6, 25, default_key_part},
                    {3, 6, 25, 5},
                    {4, 6, 25, 6},
                    {5, 6, 25, default_key_part},
                    {6, 6, 25, default_key_part},
                    {7, 6, 25, default_key_part},
                    {8, 6, 25, default_key_part}
            };
        }
        auto subscript = std::make_shared<Subscript>(operands_labels, result_labels);
        auto einsum = Einsum<size_t>(subscript, operands);
        std::vector<std::vector<default_bool_Hypertrie_t::key_part_type>> actual_results{};
        for(auto entry : einsum) {
//            for(auto key_part : entry.key)
//                std::cout << key_part << " ";
//            std::cout << std::endl;
            actual_results.push_back(entry.key);
        }
//        std::cout << "---" << std::endl;
        // the subscript will be printed in case of failure
        CAPTURE(subscript->to_string());
        // check first that the size of the results is equal to the size of the expected results
        REQUIRE(actual_results.size() == expected_results.size());
        // check that the contents of the result are equal to the contents of the expected result
        REQUIRE_THAT(actual_results, Catch::Matchers::UnorderedEquals(expected_results));
	}
}