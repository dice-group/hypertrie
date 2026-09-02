#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest/doctest.h>

#include <fmt/format.h>

#include <cppitertools/itertools.hpp>

#include <dice/hypertrie/internal/util/name_of_type.hpp>
#include <utils/AssetGenerator.hpp>
#include <utils/Node_test_configs.hpp>

#include <dice/hypertrie/internal/raw/node/CartesianNode.hpp>
#include <dice/hypertrie/internal/raw/node/fmt_CartesianNode.hpp>


namespace dice::hypertrie::tests::core::node {
	TEST_SUITE("cartesian node") {
		using namespace ::dice::hypertrie::internal::raw;
		using namespace ::dice::hypertrie::internal::util;

		template<typename T>
		using BD_t = typename CartesianNode<T::depth, typename T::htt_t, std::allocator<std::byte>>::discriminant_type;

		template<typename T>
		BD_t<T> make(std::initializer_list<size_t> init) {
			BD_t<T> d;

			size_t ix = 0;
			for (auto const &depth : init) {
				d.set(ix, depth);
				ix += 1;
			}

			return d;
		}

		TEST_CASE("discriminant") {
			SUBCASE("depth 3") {
				auto d0 = make<bool_cfg<3>>({0, 0, 3});
				CHECK(d0[0] == 0);
				CHECK(d0[1] == 0);
				CHECK(d0[2] == 3);

				auto d1 = make<bool_cfg<3>>({3, 0, 0});
				CHECK(d1[0] == 3);
				CHECK(d1[1] == 0);
				CHECK(d1[2] == 0);

				auto d2 = make<bool_cfg<3>>({2, 1, 0});
				CHECK(d2[0] == 2);
				CHECK(d2[1] == 1);
				CHECK(d2[2] == 0);

				auto d3 = make<bool_cfg<3>>({1, 1, 1});
				CHECK(d3[0] == 1);
				CHECK(d3[1] == 1);
				CHECK(d3[2] == 1);

				auto d4 = make<bool_cfg<3>>({1, 2, 0});
				CHECK(d4[0] == 1);
				CHECK(d4[1] == 2);
				CHECK(d4[2] == 0);
			}

			SUBCASE("depth 4") {
				auto d0 = make<bool_cfg<4>>({0, 0, 0, 4});
				CHECK(d0[0] == 0);
				CHECK(d0[1] == 0);
				CHECK(d0[2] == 0);
				CHECK(d0[3] == 4);

				auto d1 = make<bool_cfg<4>>({4, 0, 0, 0});
				CHECK(d1[0] == 4);
				CHECK(d1[1] == 0);
				CHECK(d1[2] == 0);
				CHECK(d1[3] == 0);

				auto d2 = make<bool_cfg<4>>({2, 0, 2, 0});
				CHECK(d2[0] == 2);
				CHECK(d2[1] == 0);
				CHECK(d2[2] == 2);
				CHECK(d2[3] == 0);

				auto d3 = make<bool_cfg<4>>({1, 1, 1, 1});
				CHECK(d3[0] == 1);
				CHECK(d3[1] == 1);
				CHECK(d3[2] == 1);
				CHECK(d3[3] == 1);

				auto d4 = make<bool_cfg<4>>({1, 2, 1, 0});
				CHECK(d4[0] == 1);
				CHECK(d4[1] == 2);
				CHECK(d4[2] == 1);
				CHECK(d4[3] == 0);
			}
		}

		TEST_CASE("discriminant slicing") {
			auto d0 = make<bool_cfg<3>>({2, 1, 0});
			fmt::print("{:b}\n", d0.repr());
			CHECK(d0.repr() == 0b100100);
			CHECK(d0.n_encoded_operands() == 2);

			auto d1 = d0.drop(0);
			fmt::print("{:b}\n", d1.repr());
			CHECK(d1.repr() == 0b0100);
			CHECK(d1.n_encoded_operands() == 1);

			auto d2 = d0.drop(1);
			fmt::print("{:b}\n", d2.repr());
			CHECK(d2.repr() == 0b1000);
			CHECK(d2.n_encoded_operands() == 1);

			auto d3 = d0.drop(2);
			fmt::print("{:b}\n", d3.repr());
			CHECK(d3.repr() == 0b1001);
			CHECK(d3.n_encoded_operands() == 2);
		}

		TEST_CASE("visit") {
			SUBCASE("depth 3") {
				using htt_t = default_bool_Hypertrie_trait;

				using CartesianNode_t = CartesianNode<3, htt_t, std::allocator<std::byte>>;

				CartesianNode_t xnode;
				xnode.discriminant().set(0, 2);
				xnode.discriminant().set(1, 1);
				xnode.discriminant().set(2, 0);
				xnode.operand(0) = NodePtr<2, htt_t, std::allocator<std::byte>>{};
				xnode.operand(1) = NodePtr<1, htt_t, std::allocator<std::byte>>{};
				xnode.operand(2) = NodePtr<0, htt_t, std::allocator<std::byte>>{};

				xnode.for_each_operand([]<size_t, size_t operand_depth>(NodePtr<operand_depth, htt_t, std::allocator<std::byte>> operand) {
					CHECK(operand == NodePtr<operand_depth, htt_t, std::allocator<std::byte>>{});
				});
			}
		}
	}
}
