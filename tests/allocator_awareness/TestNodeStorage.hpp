#ifndef HYPERTRIE_TESTNODESTORAGE_HPP
#define HYPERTRIE_TESTNODESTORAGE_HPP
#include <catch2/catch.hpp>

#include <Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp>
#include <Dice/hypertrie/internal/raw/storage/NodeStorage.hpp>

//#include <boost/interprocess/offset_ptr.hpp>
//#include <metall/metall.hpp>
#include "OffsetAllocator.hpp"




TEST_CASE("OffsetAllocator -- create uncompressed node", "[Node]") {

	using namespace hypertrie::internal::raw;

	using LevelNodeStorage_t = LevelNodeStorage<2, ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t, OffsetAllocator<int>>;

	LevelNodeStorage_t level_node_storage(OffsetAllocator<int>{});
}

//template<typename T>
//using m_alloc_t = metall::manager::allocator_type<T>;

TEST_CASE("MetallAllocator -- create uncompressed node", "[Node]") {

	using namespace hypertrie::internal::raw;

//	using LevelNodeStorage_t = LevelNodeStorage<2, ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t, m_alloc_t<int>>;
//
//	std::string path = "tmp";
//	std::string name = "LevelNodeStorage_t_with_metall";
	// test write
	{
//		metall::manager manager(metall::create_only, path.c_str());
//		auto allocator = manager.get_allocator<>();
//		auto level_storage = manager.construct<LevelNodeStorage_t>(name.c_str())(0, allocator);
	}

//	// test read
//	{
//		metall::manager manager(metall::open_only, path.c_str());
//		auto node = manager.find<LevelNodeStorage_t>(name.c_str()).first;
//		auto &edges_0 = node->edges(0);
//		edges_0[5] = TensorHash{5};
//		std::cout << edges_0.size() << std::endl;
//	}
//
//	//destroy
//	{
//		metall::manager manager(metall::open_only, path.c_str());
//		manager.destroy<LevelNodeStorage_t>(name.c_str());
//	}
}
#endif//HYPERTRIE_TESTNODESTORAGE_HPP