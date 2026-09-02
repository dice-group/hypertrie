#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <dice/einsum.hpp>
#include <dice/hypertrie/Hypertrie_default_traits.hpp>

#include "einsum2map.hpp"

#include <fmt/format.h>


namespace dice::einsum::tests {

	using namespace fmt::literals;

	TEST_SUITE("Testing Einsum value type support") {

		using allocator_type = std::allocator<std::byte>;

		using namespace ::dice::hypertrie;

		using namespace std::literals::chrono_literals;
		using time_point = std::chrono::steady_clock::time_point;

		TEST_CASE("bool to bool - failing 001") {
			using ht_type = Hypertrie<default_bool_Hypertrie_trait, allocator_type>;
			using const_ht_type = const_Hypertrie<default_bool_Hypertrie_trait, allocator_type>;
			using IEntry_t = hypertrie::Entry<default_bool_Hypertrie_trait>;
			using OEntry_t = hypertrie::Entry<default_bool_Hypertrie_trait>;
			std::vector<std::vector<IEntry_t>> const op_entries =
					{{IEntry_t{{2, 1}, 1},
					  IEntry_t{{2,2}, 1}}};
			std::vector<OEntry_t> const expected_result =
					{{IEntry_t{{2, 1}, 1},
					  IEntry_t{{2,2}, 1}}};
			std::vector<ht_type> operands;
			for (const auto &item : op_entries) {
				auto &ht = operands.emplace_back(item[0].size());
				for (const auto &entry : item)
					ht.set(entry.key(), entry.value());
			}
			std::vector<const_ht_type> const_operands;
			const_operands.assign(operands.begin(), operands.end());

			auto subscript = std::make_shared<Subscript>("ab->ab");

			auto actual_result = einsum2map<bool>(subscript, const_operands);
			CHECK_EQ(expected_result.size(), actual_result.size());
			for (const auto &result_entry : expected_result) {
				CHECK(actual_result.contains(result_entry.key()));
				if (actual_result.contains(result_entry.key()))
					CHECK_EQ(actual_result[result_entry.key()], result_entry.value());
			}
		};


		TEST_CASE("bool to uint64_t") {
			using ht_type = Hypertrie<default_bool_Hypertrie_trait, allocator_type>;
			ht_type ht_0(1);
			ht_0[{1}] = true;

			ht_type ht_1(1);
			ht_1[{1}] = true;
			ht_1[{2}] = true;

			ht_type ht_2(1);
			ht_2[{1}] = true;
			SUBCASE("j,k->j") {
				auto result = einsum2map<uint64_t>(std::make_shared<Subscript>("j,k->j"), {ht_0, ht_1});
				CHECK_EQ(result[{1}], 2L);
				CHECK_EQ(result.size(), 1UL);
			}

			SUBCASE("a,b,c->bc [0,1,2]") {
				// a,b,c->bc
				auto result = einsum2map<uint64_t>(std::make_shared<Subscript>("a,b,c->bc"), {ht_0, ht_1, ht_2});
				CHECK_EQ(result[{2, 1}], 1);
				CHECK_EQ(result[{1, 1}], 1);
				CHECK_EQ(result.size(), 2UL);
			}

			SUBCASE("a,b,c->bc [1,0,2]") {
				// a,b,c->bc
				auto result = einsum2map<uint64_t>(std::make_shared<Subscript>("a,b,c->bc"), {ht_1, ht_0, ht_2});
				for (auto const &entry : result)
					std::cout << fmt::format("({})", fmt::join(entry.first, ", ")) << " " << entry.second << std::endl;
				CHECK_EQ(result[{1, 1}], 2);
				CHECK_EQ(result.size(), 1UL);
			}
		};

		TEST_CASE("bool to uint64_t [empty]") {
			using ht_type = Hypertrie<default_bool_Hypertrie_trait, allocator_type>;
			ht_type ht_0(1);
			ht_type ht_1(1);
			ht_type ht_2(1);

			SUBCASE("j,k->j") {
				auto result = einsum2map<uint64_t>(std::make_shared<Subscript>("j,k->j"), {ht_0, ht_1});
				for (auto const &entry : result)
					std::cout << fmt::format("({})", fmt::join(entry.first, ", ")) << " " << entry.second << std::endl;
				CHECK_EQ(result.size(), 0UL);
			}

			SUBCASE("a,b,c->bc [0,1,2]") {
				// a,b,c->bc
				auto result = einsum2map<uint64_t>(std::make_shared<Subscript>("a,b,c->bc"), {ht_0, ht_1, ht_2});
				for (auto const &entry : result)
					std::cout << fmt::format("({})", fmt::join(entry.first, ", ")) << " " << entry.second << std::endl;
				CHECK_EQ(result.size(), 0UL);
			}

			SUBCASE("a,b,c->bc [1,0,2]") {
				// a,b,c->bc
				auto result = einsum2map<uint64_t>(std::make_shared<Subscript>("a,b,c->bc"), {ht_1, ht_0, ht_2});
				for (auto const &entry : result)
					std::cout << fmt::format("({})", fmt::join(entry.first, ", ")) << " " << entry.second << std::endl;
				CHECK_EQ(result.size(), 0UL);
			}
		};

		TEST_CASE("int64_t to int64_t") {
			using ht_type = Hypertrie<default_long_Hypertrie_trait, allocator_type>;

			ht_type ht_0(1);
			ht_0[{0}] = 3L;

			ht_type ht_1(1);
			ht_1[{0}] = 5L;
			SUBCASE("a") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("j,k->j"), {ht_0, ht_1});
				CHECK_EQ(result[{0}], 3 * 5L);
				CHECK_EQ(result.size(), 1UL);
			}
			SUBCASE("b") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("j,j->j"), {ht_0, ht_1});
				CHECK_EQ(result[{0}], 3 * 5L);
				CHECK_EQ(result.size(), 1UL);
			}


			ht_1[{1}] = 7L;
			SUBCASE("c") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("j,k->j"), {ht_0, ht_1});
				CHECK_EQ(result[{0}], 3L * 5L + 3L * 7L);
				CHECK_EQ(result.size(), 1UL);
			}
			SUBCASE("d") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("j,j->j"), {ht_0, ht_1});
				CHECK_EQ(result[{0}], 3L * 5L);
				CHECK_EQ(result.size(), 1UL);
			}

			ht_0[{1}] = -1L;
			SUBCASE("e") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("j,k->j"), {ht_0, ht_1});
				CHECK_EQ(result[{0}], 3L * 5L + 3L * 7L);
				CHECK_EQ(result[{1}], -1L * 5L + -1L * 7L);
				CHECK_EQ(result.size(), 2UL);
			}
			SUBCASE("f") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("j,j->j"), {ht_0, ht_1});
				CHECK_EQ(result[{0}], 3L * 5L);
				CHECK_EQ(result[{1}], -1L * 7L);
				CHECK_EQ(result.size(), 2UL);
			}

			ht_type ht_2(2);
			ht_2[{0, 0}] = -11L;
			ht_2[{1, 1}] = 3L;
			SUBCASE("g") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("jj->j"), {ht_2});
				CHECK_EQ(result[{0}], -11L);
				CHECK_EQ(result[{1}], 3L);
				CHECK_EQ(result.size(), 2UL);
			}

			SUBCASE("h") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("j,jj->j"), {ht_0, ht_2});
				CHECK_EQ(result[{0}], 3L * -11L);
				CHECK_EQ(result[{1}], -1 * 3L);
				CHECK_EQ(result.size(), 2UL);
			}
			SUBCASE("i") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("j,jj->j"), {ht_1, ht_2});
				CHECK_EQ(result[{0}], 5L * -11L);
				CHECK_EQ(result[{1}], 7L * 3L);
				CHECK_EQ(result.size(), 2UL);
			}
			ht_2[{0, 1}] = 13L;
			SUBCASE("j") {
				auto result = einsum2map<int64_t, default_long_Hypertrie_trait>(std::make_shared<Subscript>("i,ij->j"), {ht_1, ht_2});
				CHECK_EQ(result[{0}], -55);
				CHECK_EQ(result[{1}], 86);
				CHECK_EQ(result.size(), 2UL);
			}
		};

		TEST_CASE("double -> double sanity check") {
			using ht_type = Hypertrie<default_double_Hypertrie_trait, allocator_type>;

			ht_type ht_0(1);
			ht_0[{0}] = 3.2;

			ht_type ht_1(1);
			ht_1[{0}] = 5.6;

			auto result = einsum2map<double, default_double_Hypertrie_trait>(std::make_shared<Subscript>("j,k->j"), {ht_0, ht_1});
			CHECK_EQ(result[{0}], 3.2 * 5.6);
			CHECK_EQ(result.size(), 1UL);
		}

		TEST_CASE("int64 -> double sanity check") {
			using ht_type = Hypertrie<default_long_Hypertrie_trait, allocator_type>;

			ht_type ht_0(1);
			ht_0[{0}] = 3L;

			ht_type ht_1(1);
			ht_1[{0}] = 5L;

			auto result = einsum2map<double, default_long_Hypertrie_trait>(std::make_shared<Subscript>("j,k->j"), {ht_0, ht_1});
			CHECK_EQ(result[{0}], 3.0 * 5.0);
			CHECK_EQ(result.size(), 1UL);
		}
	}
};// namespace dice::einsum::tests