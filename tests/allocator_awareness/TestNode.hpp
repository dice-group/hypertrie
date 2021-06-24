#ifndef HYPERTRIE_TESTNODE_HPP
#define HYPERTRIE_TESTNODE_HPP
#include <catch2/catch.hpp>

#include <Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp>
#include <Dice/hypertrie/internal/raw/node/Node.hpp>


TEST_CASE("create compressed node", "[Node]") {

	using namespace hypertrie::internal::raw;

	Node<3,NodeCompression::uncompressed,  hypertrie::internal::lsbunused_bool_Hypertrie_internal_t> node;
}

#endif//HYPERTRIE_TESTNODE_HPP
