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
        SliceKey<unsigned long> s_key6{std::nullopt, 16, std::nullopt};
		auto ht1 = std::get<0>(ht[s_key1]);
		auto ht2 = std::get<0>(ht[s_key2]);
		auto ht3 = std::get<0>(ht[s_key3]);
		auto ht4 = std::get<0>(ht[s_key4]);
		auto ht5 = std::get<0>(ht[s_key5]);
        auto ht6 = std::get<0>(ht[s_key6]);
		std::vector<const_Hypertrie<default_bool_Hypertrie_t>> operands{};
		std::vector<std::vector<char>> operands_labels{};
		std::vector<std::vector<default_bool_Hypertrie_t::key_part_type>> expected_results;
		std::vector<char> result_labels{};
        std::vector<char> opt_begin{'['};
        std::vector<char> opt_end{']'};
        // a,ab,[bc]->abc
		SECTION("join_left-join", "combination of joins and left-joins") {
			SECTION("j_lj_dl", "join before left join, different join labels") {
				operands.push_back(ht1);
				operands.push_back(ht2);
				operands.push_back(ht3);
				std::vector<char> op1_labels{'a'};
				std::vector<char> op2_labels{'a', 'b'};
				std::vector<char> op3_labels{'b', 'c'};
				operands_labels.push_back(op1_labels);
				operands_labels.push_back(op2_labels);
				operands_labels.push_back(opt_begin);
				operands_labels.push_back(op3_labels);
				operands_labels.push_back(opt_begin);
				result_labels.push_back('a');
				result_labels.push_back('b');
				result_labels.push_back('c');
				expected_results = {
						{1, 3, 5},
						{1, 6, default_key_part},
						{2, 4, 6},
						{5, 7, default_key_part}};
			}
            // a,ab,[ac]->abc
            SECTION("j_lj_sl", "join before left join, on same labels") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'a', 'c'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
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
                        {5, 7, default_key_part}
                };
            }
            // a,[ab,bc]->abc
            SECTION("lj_j_dl" , "join after left join, on different labels") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
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
            SECTION("j_j_lj_dl", "two joins before left join, on different labels") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'c', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
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
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'c', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_end);
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
				std::vector<char> op2_labels{'a', 'b'};
				std::vector<char> op3_labels{'a', 'c'};
				operands_labels.push_back(op1_labels);
				operands_labels.push_back(opt_begin);
				operands_labels.push_back(op2_labels);
				operands_labels.push_back(op3_labels);
				operands_labels.push_back(opt_end);
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
            // a,[ab,bc],ad->abcd
            SECTION("lj_dl_j", "join label after left join") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'a', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(op4_labels);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, 3, 5, 35},
                        {5, default_key_part, default_key_part, 30},
                        {8, default_key_part, default_key_part, 30}
                };
            }
            // a,[ab,bc],[ad]->abcd
            SECTION("mlj_jdl", "multiple left joins on the same label, join after left join") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'a', 'd'};
                operands_labels.push_back(op1_labels);
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
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'a', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_end);
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
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'c', 'e'};
                std::vector<char> op5_labels{'a', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op5_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_end);
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
            // a,ab,bc,[ad]->abcd
            SECTION("mjdl_lj_jl", "multiple joins on different labels and left join on join label") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'a', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, 3, 5, 35},
                        {2, 4, 6, default_key_part}
                };
            }
            // a,ab,bc,[ad],[be]->abcde
            SECTION("mjdl_mlj_jl", "multiple joins on different labels and left join on join labels") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht5);
                operands.push_back(ht4);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'a', 'd'};
                std::vector<char> op5_labels{'b', 'e'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op5_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                result_labels.push_back('e');
                expected_results = {
                        {1, 3, 5, 35, default_key_part},
                        {2, 4, 6, default_key_part, 25}
                };
            }
            // a,ab,bc,[ad],be->abcde
            SECTION("mjdl_mlj_jl2", "multiple joins on different labels and left join on join labels") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht5);
                operands.push_back(ht4);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'a', 'd'};
                std::vector<char> op5_labels{'b', 'e'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(op5_labels);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                result_labels.push_back('e');
                expected_results = {
                        {2, 4, 6, default_key_part, 25}
                };
            }
            // a,ab,bc,ce,[bd]->abcde
            SECTION("mjdl_mlj_jl3", "multiple joins on different labels and left join on join labels") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht5);
                operands.push_back(ht4);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'c', 'e'};
                std::vector<char> op5_labels{'b', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op5_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                result_labels.push_back('e');
                expected_results = {
                        {1, 3, 5, default_key_part, 30}
                };
            }
		}
		SECTION("left-join_cartesian", "combination of left join and cartesian join") {
			// a,[ab,bc,[ad]],[ae,ef,[ag]]->abdeg
			SECTION("lj_cj", "cartesian join after left join") {
				operands.push_back(ht1);
				operands.push_back(ht3);
				operands.push_back(ht2);
				operands.push_back(ht5);
				operands.push_back(ht2);
				operands.push_back(ht4);
				operands.push_back(ht5);
				std::vector<char> op1_labels{'a'};
				std::vector<char> op2_labels{'a', 'b'};
				std::vector<char> op3_labels{'b', 'c'};
				std::vector<char> op4_labels{'a', 'd'};
				std::vector<char> op5_labels{'a', 'e'};
				std::vector<char> op6_labels{'e', 'f'};
				std::vector<char> op7_labels{'a', 'g'};
				operands_labels.push_back(op1_labels);
				operands_labels.push_back(opt_begin);
				operands_labels.push_back(op2_labels);
				operands_labels.push_back(op3_labels);
				operands_labels.push_back(opt_begin);
				operands_labels.push_back(op4_labels);
				operands_labels.push_back(opt_end);
				operands_labels.push_back(opt_end);
				operands_labels.push_back(opt_begin);
				operands_labels.push_back(op5_labels);
				operands_labels.push_back(op6_labels);
				operands_labels.push_back(opt_begin);
				operands_labels.push_back(op7_labels);
				operands_labels.push_back(opt_end);
				operands_labels.push_back(opt_end);
				result_labels.push_back('a');
				result_labels.push_back('b');
				result_labels.push_back('d');
				result_labels.push_back('e');
				result_labels.push_back('g');
				expected_results = {
						{1, default_key_part, default_key_part, 6, 35},
						{2, default_key_part, default_key_part, 4, default_key_part},
						{3, 5, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part, default_key_part}};
			}
            // a,[bc,cd]->abc
            SECTION("c_opt", "cartesian with optional operand") {
                operands.push_back(ht1);
                operands.push_back(ht4);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'b', 'c'};
                std::vector<char> op3_labels{'c', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, default_key_part, default_key_part, default_key_part},
                        {2, default_key_part, default_key_part, default_key_part},
                        {3, default_key_part, default_key_part, default_key_part},
                        {4, default_key_part, default_key_part, default_key_part},
                        {5, default_key_part, default_key_part, default_key_part},
                        {6, default_key_part, default_key_part, default_key_part},
                        {7, default_key_part, default_key_part, default_key_part},
                        {8, default_key_part, default_key_part, default_key_part}
                };
            }
            // [ab,bh],[cd],[eg,ef]->abcdef
            SECTION("c_full_opt", "optional cartesian") {
                operands.push_back(ht2);
                operands.push_back(ht5);
                operands.push_back(ht3);
                operands.push_back(ht4);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a', 'b'};
                std::vector<char> op2_labels{'b', 'h'};
                std::vector<char> op3_labels{'c', 'd'};
                std::vector<char> op4_labels{'e', 'g'};
                std::vector<char> op5_labels{'e', 'f'};
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_begin);
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
                result_labels.push_back('f');
                decltype(expected_results) expected_results1 = {
                        {default_key_part, default_key_part}
                };
                decltype(expected_results) expected_results2 = {
                        {1, 8},
                        {4, 6},
                        {3, 5}
                };
                decltype(expected_results) expected_results3 = {
                        {default_key_part, default_key_part}
                };
                for(auto& res1 : expected_results1) {
                    for(auto& res2 : expected_results2) {
                        for(auto& res3 : expected_results3) {
                            auto temp_res = res1;
                            temp_res.insert(temp_res.end(), res2.begin(), res2.end());
                            temp_res.insert(temp_res.end(), res3.begin(), res3.end());
                            expected_results.push_back(temp_res);
                        }
                    }
                }
            }
            // ab,bh,[cd],[eg,ef]->abcdef
            SECTION("c_nopt_opt", "optional cartesian") {
                operands.push_back(ht2);
                operands.push_back(ht5);
                operands.push_back(ht3);
                operands.push_back(ht4);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a', 'b'};
                std::vector<char> op2_labels{'b', 'h'};
                std::vector<char> op3_labels{'c', 'd'};
                std::vector<char> op4_labels{'e', 'g'};
                std::vector<char> op5_labels{'e', 'f'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_begin);
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
                result_labels.push_back('f');
                expected_results = {};
            }
            // a,[ab],c,[cd]->abcd
            SECTION("lj_cart", "left join and cartesian join") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht1);
                operands.push_back(ht3);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'c'};
                std::vector<char> op4_labels{'c', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                decltype(expected_results) expected_results_left{
                        {1, 3},
                        {1, 6},
                        {2, 4},
                        {3, default_key_part},
                        {4, default_key_part},
                        {5, 7},
                        {6, default_key_part},
                        {7, default_key_part},
                        {8, default_key_part}
                };
                decltype(expected_results) expected_results_right{
                        {1, 8},
                        {2, default_key_part},
                        {3, 5},
                        {4, 6},
                        {5, default_key_part},
                        {6, default_key_part},
                        {7, default_key_part},
                        {8, default_key_part}
                };
                for(const auto& left_res : expected_results_left) {
                    for(auto right_res : expected_results_right) {
                        auto temp_res = left_res;
                        temp_res.insert(temp_res.end(), right_res.begin(), right_res.end());
                        expected_results.push_back(temp_res);
                    }
                }
            }
            // a,[ab,cd]->abc
            SECTION("cart_in_opt" , "cartesian in optional with no results") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.push_back(ht6);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{'c', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                expected_results = {
                        {1, default_key_part},
                        {2, default_key_part},
                        {3, default_key_part},
                        {4, default_key_part},
                        {5, default_key_part},
                        {6, default_key_part},
                        {7, default_key_part},
                        {8, default_key_part},
                };
            }
            // a,[ab,]->ab
            SECTION("rank-0_opt" , "rank-0 tensor inside optional") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.emplace_back(const_Hypertrie());
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'a', 'b'};
                std::vector<char> op3_labels{};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                expected_results = {
                        {1, default_key_part},
                        {2, default_key_part},
                        {3, default_key_part},
                        {4, default_key_part},
                        {5, default_key_part},
                        {6, default_key_part},
                        {7, default_key_part},
                        {8, default_key_part},
                };
            }
            // a,,[ab]->abc
            SECTION("rank-0_non_opt" , "rank-0 tensor outside optional") {
                operands.push_back(ht1);
                operands.emplace_back(const_Hypertrie());
                operands.push_back(ht2);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op3_labels{};
                std::vector<char> op2_labels{'a', 'b'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                expected_results = {};
            }
            // a,,[ab]->abc
            SECTION("rank-0_non_opt_true" , "rank-0 tensor outside optional holding a true value") {
                Hypertrie<default_bool_Hypertrie_t> ht_true{0};
				ht_true.set(Key<unsigned long>(), true);
                operands.push_back(ht1);
                operands.emplace_back(ht_true);
                operands.push_back(ht2);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op3_labels{};
                std::vector<char> op2_labels{'a', 'b'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                expected_results = {
                        {1, 3},
                        {1, 6},
                        {2, 4},
                        {3, default_key_part},
                        {4, default_key_part},
                        {5, 7},
                        {6, default_key_part},
                        {7, default_key_part},
                        {8, default_key_part}
				};
            }
		}
		SECTION("left-join_union", "union queries with optional parts") {
			// a,[ab],[ac]->bc
			SECTION("lj_union", "one union between optional parts") {
                operands.push_back(ht1);
                operands.push_back(ht2);
                operands.emplace_back(ht3);
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
                result_labels.push_back('b');
                result_labels.push_back('c');
                expected_results = {
                        {3, default_key_part},
                        {6, default_key_part},
                        {default_key_part, 8},
                        {4, default_key_part},
                        {7, default_key_part},
                        {default_key_part, 6},
                        {default_key_part, 5},
                        {default_key_part, default_key_part},
                        {default_key_part, default_key_part},
                        {default_key_part, default_key_part}
                };
			}
		}
		SECTION("no_wwd", "not weakly well-designed queries") {
            // a,[ab,[bc],cd]->abcd
            SECTION("lj_nlj_j", "nested left join followed by join") {
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
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, default_key_part, default_key_part, default_key_part},
                        {1, default_key_part, default_key_part, default_key_part},
                        {2, 4, 6, 25},
                        {3, default_key_part, default_key_part, default_key_part},
                        {4, default_key_part, default_key_part, default_key_part},
                        {5, default_key_part, default_key_part, default_key_part},
                        {6, default_key_part, default_key_part, default_key_part},
                        {7, default_key_part, default_key_part, default_key_part},
                        {8, default_key_part, default_key_part, default_key_part}
                };
            }
            // a,[b,bc,[ad]]->abcd
            SECTION("no_wd", "not well defined") {
                operands.push_back(ht1);
                operands.push_back(ht1);
                operands.push_back(ht4);
                operands.push_back(ht3);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'b'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'a', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_end);
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
            // a,[be,bc,[ad]]->abcd
            SECTION("no_wd2", "not well defined, no results") {
                operands.push_back(ht1);
                operands.push_back(ht5);
                operands.push_back(ht4);
                operands.push_back(ht3);
                std::vector<char> op1_labels{'a'};
                std::vector<char> op2_labels{'b', 'e'};
                std::vector<char> op3_labels{'b', 'c'};
                std::vector<char> op4_labels{'a', 'd'};
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, default_key_part, default_key_part, default_key_part},
                        {2, default_key_part, default_key_part, default_key_part},
                        {3, default_key_part, default_key_part, default_key_part},
                        {4, default_key_part, default_key_part, default_key_part},
                        {5, default_key_part, default_key_part, default_key_part},
                        {6, default_key_part, default_key_part, default_key_part},
                        {7, default_key_part, default_key_part, default_key_part},
                        {8, default_key_part, default_key_part, default_key_part}
                };
            }
            // [ab,bc],a,ad->abcd
            SECTION("nlo_j", "no left operand, join") {
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht1);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a', 'b'};
                std::vector<char> op2_labels{'b', 'c'};
                std::vector<char> op3_labels{'a'};
                std::vector<char> op4_labels{'a', 'd'};
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(op4_labels);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, 3, 5, 35}
                };
            }
            // [ab,bc],a,[ad]->abcd
            SECTION("nlo_lj", "no left operand, left join") {
                operands.push_back(ht2);
                operands.push_back(ht3);
                operands.push_back(ht1);
                operands.push_back(ht5);
                std::vector<char> op1_labels{'a', 'b'};
                std::vector<char> op2_labels{'b', 'c'};
                std::vector<char> op3_labels{'a'};
                std::vector<char> op4_labels{'a', 'd'};
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op1_labels);
                operands_labels.push_back(op2_labels);
                operands_labels.push_back(opt_end);
                operands_labels.push_back(op3_labels);
                operands_labels.push_back(opt_begin);
                operands_labels.push_back(op4_labels);
                operands_labels.push_back(opt_end);
                result_labels.push_back('a');
                result_labels.push_back('b');
                result_labels.push_back('c');
                result_labels.push_back('d');
                expected_results = {
                        {1, 3, 5, 35},
                        {2, 4, 6, default_key_part}
                };
            }
		}
        auto subscript = std::make_shared<Subscript>(operands_labels, result_labels);
        auto einsum = Einsum<size_t>(subscript, operands);
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