#ifndef HYPERTRIE_TESTNODE_HPP
#define HYPERTRIE_TESTNODE_HPP

#include <catch2/catch.hpp>

#include <dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp>
#include <dice/hypertrie/internal/raw/node/Node.hpp>

#include "HypertrieTraits_with_allocators.hpp"
#include <boost/interprocess/offset_ptr.hpp>
#include <metall/metall.hpp>


namespace tests::dice::hypertrie::allocator_awareness{

	TEST_CASE("OffsetAllocator -- create uncompressed node", "[Node]") {

		using namespace ::hypertrie::internal::raw;

		using Node_t = Node<3, NodeCompression::uncompressed, offset_tri_LSB_false>;

		Node_t node(0, OffsetAllocator<std::byte>{});
	}

	TEST_CASE("MetallAllocator -- create uncompressed node", "[Node]") {

		using namespace ::hypertrie::internal::raw;

		using Node_t = Node<2, NodeCompression::uncompressed, metall_tri_LSB_false>;

		std::string path = "tmp";
		std::string name = "Node_t_with_metall";
		// test write
		{
			metall::manager manager(metall::create_only, path.c_str());
			auto allocator = manager.get_allocator<>();
			auto node = manager.construct<Node_t>(name.c_str())(0, allocator);
			auto &edges_0 = node->edges(0);
			edges_0[5] = TensorHash{5};
			//std::cout << edges_0.size() << std::endl;
			REQUIRE(edges_0.size() == 1);
		}

		// test read
		{
			metall::manager manager(metall::open_only, path.c_str());
			auto node = manager.find<Node_t>(name.c_str()).first;
			auto &edges_0 = node->edges(0);
			edges_0[5] = TensorHash{5};
			//std::cout << edges_0.size() << std::endl;
			REQUIRE(edges_0.size() == 1);
		}

		//destroy
		{
			metall::manager manager(metall::open_only, path.c_str());
			manager.destroy<Node_t>(name.c_str());
		}
	}

	TEST_CASE("MetallAllocator -- create uncompressed node -- depth 1", "[Node]") {

		using namespace ::hypertrie::internal::raw;

		using Node_t = Node<1, NodeCompression::uncompressed, metall_tri_LSB_false>;

		std::string path = "tmp";
		std::string name = "Node_t_with_metall";
		// test write
		{
			metall::manager manager(metall::create_only, path.c_str());
			auto allocator = manager.get_allocator<>();
			auto node = manager.construct<Node_t>(name.c_str())(0, allocator);
			auto &edges = node->edges();
			edges.insert(5);
			//std::cout << edges.size() << std::endl;
			REQUIRE(edges.size() == 1);
		}

		// test read
		{
			metall::manager manager(metall::open_only, path.c_str());
			auto node = manager.find<Node_t>(name.c_str()).first;
			auto &edges = node->edges();
			edges.insert(5);
			//std::cout << edges.size() << std::endl;
			REQUIRE(edges.size() == 1);
		}

		//destroy
		{
			metall::manager manager(metall::open_only, path.c_str());
			manager.destroy<Node_t>(name.c_str());
		}
	}
}// namespace

#endif//HYPERTRIE_TESTNODE_HPP
