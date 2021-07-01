#ifndef HYPERTRIE_TESTNODE_HPP
#define HYPERTRIE_TESTNODE_HPP
#include <catch2/catch.hpp>

#include <Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp>
#include <Dice/hypertrie/internal/raw/node/Node.hpp>

#include <boost/interprocess/offset_ptr.hpp>
#include <metall/metall.hpp>


TEST_CASE("OffsetAllocator -- create uncompressed node", "[Node]") {

	using namespace hypertrie::internal::raw;

	using Node_t = Node<3, NodeCompression::uncompressed, ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t, OffsetAllocator<int>>;

	Node_t node(0, OffsetAllocator<int>{});
}

TEST_CASE("MetallAllocator -- create uncompressed node", "[Node]") {

	using namespace hypertrie::internal::raw;

	using Node_t = Node<2, NodeCompression::uncompressed, ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t, m_alloc_t<int>>;

	std::string path = "tmp";
	std::string name = "Node_t_with_metall";
	// test write
	{
		metall::manager manager(metall::create_only, path.c_str());
		auto allocator = manager.get_allocator<>();
		auto node = manager.construct<Node_t>(name.c_str())(0, allocator);
		auto &edges_0 = node->edges(0);
		edges_0[5] = TensorHash{5};
		std::cout << edges_0.size() << std::endl;
	}

	// test read
	{
		metall::manager manager(metall::open_only, path.c_str());
		auto node = manager.find<Node_t>(name.c_str()).first;
		auto &edges_0 = node->edges(0);
		edges_0[5] = TensorHash{5};
		std::cout << edges_0.size() << std::endl;
	}

	//destroy
	{
		metall::manager manager(metall::open_only, path.c_str());
		manager.destroy<Node_t>(name.c_str());
	}
}

TEST_CASE("MetallAllocator -- create uncompressed node -- depth 1", "[Node]") {

	using namespace hypertrie::internal::raw;

	using Node_t = Node<1, NodeCompression::uncompressed, ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t, m_alloc_t<int>>;

	std::string path = "tmp";
	std::string name = "Node_t_with_metall";
	// test write
	{
		metall::manager manager(metall::create_only, path.c_str());
		auto allocator = manager.get_allocator<>();
		auto node = manager.construct<Node_t>(name.c_str())(0, allocator);
		auto &edges = node->edges();
		edges.insert(5);
		std::cout << edges.size() << std::endl;
	}

	// test read
	{
		metall::manager manager(metall::open_only, path.c_str());
		auto node = manager.find<Node_t>(name.c_str()).first;
		auto &edges = node->edges();
		edges.insert(5);
		std::cout << edges.size() << std::endl;
	}

	//destroy
	{
		metall::manager manager(metall::open_only, path.c_str());
		manager.destroy<Node_t>(name.c_str());
	}
}


#endif//HYPERTRIE_TESTNODE_HPP
