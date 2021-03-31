#include <Dice/hypertrie/hypertrie.hpp>
#include <catch2/catch.hpp>

namespace hypertrie::tests::leftjoin {

    TEST_CASE("left-join") {

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
        auto default_key_part = std::numeric_limits<default_bool_Hypertrie_t::key_part_type>::max();
        SliceKey<unsigned long> s_key1{std::nullopt, 10, 20};
		SliceKey<unsigned long> s_key2{std::nullopt, 11, std::nullopt};
		SliceKey<unsigned long> s_key3{std::nullopt, 12, std::nullopt};
        SliceKey<unsigned long> s_key4{std::nullopt, 13, std::nullopt};
        SliceKey<unsigned long> s_key5{std::nullopt, 14, std::nullopt};
        SliceKey<unsigned long> s_key6{std::nullopt, 15, std::nullopt};
        SliceKey<unsigned long> s_key7{std::nullopt, 16, std::nullopt};
        auto ht1 = std::get<0>(ht[s_key1]);
        auto ht2 = std::get<0>(ht[s_key2]);
        auto ht3 = std::get<0>(ht[s_key3]);
        auto ht4 = std::get<0>(ht[s_key4]);
        auto ht5 = std::get<0>(ht[s_key5]);
        auto ht6 = std::get<0>(ht[s_key6]);
        auto ht7 = std::get<0>(ht[s_key7]);
		std::vector<const_Hypertrie<default_bool_Hypertrie_t>> operands{};
		std::vector<std::vector<char>> operands_labels{};
        std::vector<std::vector<default_bool_Hypertrie_t::key_part_type>> expected_results;
		std::vector<char> opt_begin{'['};
		std::vector<char> opt_end{']'};
        std::vector<char> result_labels{};

		SECTION("wd", "well-designed queries") {
            // a,[ab]->a / a,[ab]->ab
            SECTION("lj", "simple left join, project only non-optional variable") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                SECTION("lj1") {
                    result_labels.push_back('a');
                    expected_results = {
                            {1}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}
                    };
                }
                SECTION("lj2", "simple left join, project both variables") {
                    result_labels.push_back('a');
                    result_labels.push_back('b');
                    expected_results = {
                            {1, 3},
                            {1, 6},
                            {2, 4},
                            {3, default_key_part},
                            {4, default_key_part},
                            {5, default_key_part},
                            {6, default_key_part},
                            {7, default_key_part},
                            {8, default_key_part}
                    };
                }
            }
            // ab,[bc]->abc
            SECTION("lj3", "simple left join, join variable not at first position") {
                operands.push_back(ht2);
                operands.push_back(ht3);
                std::vector<char> op1_labels{'a', 'b'};
                std::vector<char> op2_labels{'b', 'c'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                expected_results = {
                        {1, 3, 5},
                        {1, 6, default_key_part},
                        {2, 4, 6}
                };
            }
            SECTION("mlj_sl", "multiple not nested left joins on the same label") {
                //a,[ab],[ac]->abc
                SECTION("mlj_sl2", "two optional operands") {
                    operands.push_back(ht1);
                    operands.push_back(ht2);
                    operands.push_back(ht3);
                    std::vector<char> op1_labels{'a'};
                    std::vector<char> op2_labels{'a', 'b'};
                    std::vector<char> op3_labels{'a', 'c'};
                    operands_labels.push_back(op1_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op2_labels);
                    operands_labels.push_back(opt_end);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op3_labels);
                    operands_labels.push_back(opt_end);
                    result_labels.push_back('a');
                    result_labels.push_back('b');
                    result_labels.push_back('c');
                    expected_results = {
                            {1, 3, 8},
                            {1, 6, 8},
                            {2, 4, default_key_part},
                            {3, default_key_part, 5},
                            {4, default_key_part, 6},
                            {5, default_key_part, default_key_part},
                            {6, default_key_part, default_key_part},
                            {7, default_key_part, default_key_part},
                            {8, default_key_part, default_key_part}
                    };
                }
                //a,[ab],[ac],[ad]->abcd
                SECTION("mlj_sl3", "three optional operands") {
                    operands.push_back(ht1);
                    operands.push_back(ht2);
                    operands.push_back(ht3);
                    operands.push_back(ht5);
                    std::vector<char> op1_labels{'a'};
                    std::vector<char> op2_labels{'a', 'b'};
                    std::vector<char> op3_labels{'a', 'c'};
                    std::vector<char> op4_labels{'a', 'd'};
                    operands_labels.push_back(op1_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op2_labels);
                    operands_labels.push_back(opt_end);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op3_labels);
                    operands_labels.push_back(opt_end);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op4_labels);
                    operands_labels.push_back(opt_end);
                    result_labels.push_back('a');
                    result_labels.push_back('b');
                    result_labels.push_back('c');
                    result_labels.push_back('d');
                    expected_results = {
                            {1, 3, 8, 35},
                            {1, 6, 8, 35},
                            {2, 4, default_key_part, default_key_part},
                            {3, default_key_part, 5, default_key_part},
                            {4, default_key_part, 6, default_key_part},
                            {5, default_key_part, default_key_part, 30},
                            {6, default_key_part, default_key_part, default_key_part},
                            {7, default_key_part, default_key_part, default_key_part},
                            {8, default_key_part, default_key_part, 30}
                    };
                }
            }
            SECTION("nlj_dl", "nested left joins on different labels") {
                //a,[ab,[bc]]->abc
                SECTION("nlj_dl1", "one nested level") {
                    operands.push_back(ht1);
                    operands.push_back(ht2);
                    operands.push_back(ht3);
                    std::vector<char> op1_labels{'a'};
                    std::vector<char> op2_labels{'a', 'b'};
                    std::vector<char> op3_labels{'b', 'c'};
                    operands_labels.push_back(op1_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op2_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op3_labels);
                    operands_labels.push_back(opt_end);
                    operands_labels.push_back(opt_end);
                    result_labels.push_back('a');
                    result_labels.push_back('b');
                    result_labels.push_back('c');
                    expected_results = {
                            {1, 3, 5},
                            {1, 6, default_key_part},
                            {2, 4, 6},
                            {3, default_key_part, default_key_part},
                            {4, default_key_part, default_key_part},
                            {5, default_key_part, default_key_part},
                            {6, default_key_part, default_key_part},
                            {7, default_key_part, default_key_part},
                            {8, default_key_part, default_key_part}
                    };
                }
                // a,[ab,[bc,[cd]]]->abcd
                SECTION("nlj_dl2", "two nested levels") {
                    operands.push_back(ht1);
                    operands.push_back(ht2);
                    operands.push_back(ht3);
                    operands.push_back(ht5);
                    std::vector<char> op1_labels{'a'};
                    std::vector<char> op2_labels{'a', 'b'};
                    std::vector<char> op3_labels{'b', 'c'};
                    std::vector<char> op4_labels{'c', 'd'};
                    operands_labels.push_back(op1_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op2_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op3_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op4_labels);
                    operands_labels.push_back(opt_end);
                    operands_labels.push_back(opt_end);
                    operands_labels.push_back(opt_end);
                    result_labels.push_back('a');
                    result_labels.push_back('b');
                    result_labels.push_back('c');
                    result_labels.push_back('d');
                    expected_results = {
                            {1, 3, 5, 30},
                            {1, 6, default_key_part, default_key_part},
                            {2, 4, 6, default_key_part},
                            {3, default_key_part, default_key_part, default_key_part},
                            {4, default_key_part, default_key_part, default_key_part},
                            {5, default_key_part, default_key_part, default_key_part},
                            {6, default_key_part, default_key_part, default_key_part},
                            {7, default_key_part, default_key_part, default_key_part},
                            {8, default_key_part, default_key_part, default_key_part}
                    };
                }
            }
            SECTION("nlj_sl", "nested left joins on the same label") {
                //a,[ab,[ac]]->abc
                SECTION("nlj_sl1", "one nested level") {
                    operands.push_back(ht1);
                    operands.push_back(ht2);
                    operands.push_back(ht3);
                    std::vector<char> op1_labels{'a'};
                    std::vector<char> op2_labels{'a', 'b'};
                    std::vector<char> op3_labels{'a', 'c'};
                    operands_labels.push_back(op1_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op2_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op3_labels);
                    operands_labels.push_back(opt_end);
                    operands_labels.push_back(opt_end);
                    result_labels.push_back('a');
                    result_labels.push_back('b');
                    result_labels.push_back('c');
                    expected_results = {
                            {1, 3, 8},
                            {1, 6, 8},
                            {2, 4, default_key_part},
                            {3, default_key_part, default_key_part},
                            {4, default_key_part, default_key_part},
                            {5, default_key_part, default_key_part},
                            {6, default_key_part, default_key_part},
                            {7, default_key_part, default_key_part},
                            {8, default_key_part, default_key_part}};
                }
                //a,[ab,[ac,[ad]]]->abcd
                SECTION("nlj_sl2", "two nested levels") {
                    operands.push_back(ht1);
                    operands.push_back(ht2);
                    operands.push_back(ht6);
                    operands.push_back(ht7);
                    std::vector<char> op1_labels{'a'};
                    std::vector<char> op2_labels{'a', 'b'};
                    std::vector<char> op3_labels{'a', 'c'};
                    std::vector<char> op4_labels{'a', 'd'};
                    operands_labels.push_back(op1_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op2_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op3_labels);
                    operands_labels.push_back(opt_begin);
                    operands_labels.push_back(op4_labels);
                    operands_labels.push_back(opt_end);
                    operands_labels.push_back(opt_end);
                    operands_labels.push_back(opt_end);
                    result_labels.push_back('a');
                    result_labels.push_back('b');
                    result_labels.push_back('c');
                    result_labels.push_back('d');
                    expected_results = {
                            {1, 3, 40, default_key_part},
                            {1, 6, 40, default_key_part},
                            {2, 4, default_key_part, default_key_part},
                            {3, default_key_part, default_key_part, default_key_part},
                            {4, default_key_part, default_key_part, default_key_part},
                            {5, default_key_part, default_key_part, default_key_part},
                            {6, default_key_part, default_key_part, default_key_part},
                            {7, default_key_part, default_key_part, default_key_part},
                            {8, default_key_part, default_key_part, default_key_part}
                    };
                }
            }
            // ab,[ac],[bd]->abcd
            SECTION("lj_ml", "left join on multiple labels") {
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht4);
                std::vector<char> op1_labels{'a', 'b'};
                std::vector<char> op2_labels{'a', 'c'};
                std::vector<char> op3_labels{'b', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, 3, 8, 25},
                        {1, 6, 8, default_key_part},
                        {2, 4, default_key_part, 25}
                };
            }
		}
        SECTION("wwd", "weakly well-defined queries") {
			// a,[ab],[ab]->ab
			SECTION("lj_lj", "if the 2nd operand does not yield result -> left_join") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'a', 'b'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                expected_results = {
                        {1, 3},
                        {1, 6},
                        {2, 4},
                        {3, 5},
                        {4, 6},
                        {5, default_key_part},
                        {6, default_key_part},
                        {7, default_key_part},
                        {8, default_key_part}
                };
		    }
            // ab,[bc],[cd]->abcd
            SECTION("lj_cart", "if the second operand does not yield result -> cartesian") {
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht4);
                std::vector<char> op1_labels{'a', 'b'};
                std::vector<char> op2_labels{'b', 'c'};
                std::vector<char> op3_labels{'c', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, 3, 5, default_key_part},
                        {1, 6, 3, 25},
                        {1, 6, 4, 25},
                        {2, 4, 6, default_key_part}
                };
            }
            // [ab],[ac]->abc
            SECTION("lj_all_opt", "left join with only optional operands") {
                operands.push_back(ht2);
                operands.push_back(ht3);
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'a', 'c'};
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                expected_results = {
                        {1, 3, 8},
                        {1, 6, 8},
                        {2, 4, default_key_part},
                };
            }
            // [ab,bd],[bc]->abc
            SECTION("lj_all_opt_elim", "left join without left operand followed by left join, the first left join does not yield results") {
                operands.push_back(ht2);
				operands.push_back(ht5);
                operands.push_back(ht3);
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'd'};
                std::vector<char> op4_labels{'b', 'c'};
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                expected_results = {
                        {default_key_part, 1, 8},
                        {default_key_part, 4, 6},
                        {default_key_part, 3, 5}
                };
            }
            // a,[ab,[bc],[cd]]->abcd
            SECTION("lj_nlj_lj", "nested left join followed by left join") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht4);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'c', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, 3, 5, default_key_part},
                        {1, 6, 4, 25},
                        {1, 6, 3, 25},
                        {2, 4, 6, default_key_part},
                        {3, default_key_part, default_key_part, default_key_part},
                        {4, default_key_part, default_key_part, default_key_part},
                        {5, default_key_part, default_key_part, default_key_part},
                        {6, default_key_part, default_key_part, default_key_part},
                        {7, default_key_part, default_key_part, default_key_part},
                        {8, default_key_part, default_key_part, default_key_part}
                };
            }
            // a,[ab,ac],[bd,ce] -> abcde
            SECTION("wwd_2", "two wwd edges") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht4);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'a', 'c'};
                std::vector<char> op4_labels{'b', 'd'};
                std::vector<char> op5_labels{'c', 'e'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(op5_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                result_labels.push_back('e');
                expected_results = {
                        {1, 3, 8, 25, 30},
                        {1, 6, 8, default_key_part, default_key_part}
                };
				for(std::size_t i = 2; i < 9; i ++) {
                    expected_results.push_back({i, 4, 8, 25, 30});
                    expected_results.push_back({i, 3, 8, 25, 30});
                    expected_results.push_back({i, 4, 1, 25, 35});
                    expected_results.push_back({i, 3, 1, 25, 35});
                    expected_results.push_back({i, 4, 5, 25, 30});
                    expected_results.push_back({i, 3, 5, 25, 30});
				}
            }
        }
		SECTION("no-wwd", "not weakly well-defined") {
			// [ab],bc->abc
            SECTION("no_left_op", "left join without left operand") {
                operands.push_back(ht2);
                operands.push_back(ht3);
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(op3_labels);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                expected_results = {
                        {1, 3, 5},
                        {2, 4, 6}
                };
            }
		}
		// prepare the einstein summation
        auto subscript = std::make_shared<Subscript>(operands_labels, result_labels);
        auto einsum = Einsum<size_t>(subscript, operands);
		// save the keys of the summation in a vector
        std::vector<std::vector<default_bool_Hypertrie_t::key_part_type>> actual_results{};
        for(const auto& entry : einsum)
			for(std::size_t i = 0; i < entry.value; i++)
                actual_results.push_back(entry.key);
		// the subscript will be printed in case of failure
        CAPTURE(subscript->to_string());
		// check first that the size of the results is equal to the size of the expected results
        REQUIRE(actual_results.size() == expected_results.size());
		// check that the contents of the result are equal to the contents of the expected result
        REQUIRE_THAT(actual_results, Catch::Matchers::UnorderedEquals(expected_results));
	}

}