#include <Dice/hypertrie/hypertrie.hpp>
#include <catch2/catch.hpp>

namespace hypertrie::tests::leftjoin {

	TEST_CASE("join combination test-cases", "[join-combination]") {

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
        SECTION("one join outside left join, one left join, different labels") {
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
                    {2, 4, 6}
            };
        }
		// a,[ab,bc]->abc
        SECTION("one join inside left join, one left join, different labels") {
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
        SECTION("two joins ouside left join, one left join, different labels") {
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
        SECTION("join in optional, nested left join, different labels") {
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
		// [ab],ac->abc
        SECTION("one left join , one join, left-join before join, different labels") {
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
        auto subscript = std::make_shared<Subscript>(operands_labels, result_labels);
        auto einsum = Einsum<size_t>(subscript, operands);
        std::vector<std::vector<default_bool_Hypertrie_t::key_part_type>> actual_results{};
        for(auto entry : einsum) {
            for(auto key_part : entry.key)
                std::cout << key_part << " ";
            std::cout << std::endl;
            actual_results.push_back(entry.key);
        }
        std::cout << "---" << std::endl;
        REQUIRE(actual_results.size() == expected_results.size());
        for(auto actual_result : actual_results) {
            if(std::find(expected_results.begin(), expected_results.end(), actual_result) == expected_results.end()) {
                FAIL();
            }
        }
	}
}