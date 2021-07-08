#ifndef HYPERTRIE_TESTNODESTORAGE_HPP
#define HYPERTRIE_TESTNODESTORAGE_HPP
#include <catch2/catch.hpp>

#include <Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp>
#include <Dice/hypertrie/internal/raw/storage/NodeStorage.hpp>

#include <metall/metall.hpp>
#include <tsl/boost_offset_pointer.h>
#include "OffsetAllocator.hpp"




TEST_CASE("OffsetAllocator -- create uncompressed node", "[NodeStorage]") {

	using namespace hypertrie::internal::raw;

	using LevelNodeStorage_t = LevelNodeStorage<2, ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t, OffsetAllocator<int>>;

	LevelNodeStorage_t level_node_storage(OffsetAllocator<int>{});
}
template<typename T>
using m_alloc_t = metall::manager::allocator_type<T>;

using namespace hypertrie::internal::raw;
/*
using boost_type_trait = hypertrie::Hypertrie_t<unsigned long,
		bool,
		hypertrie::internal::container::boost_flat_map ,
		hypertrie::internal::container::boost_flat_set>;
using boost_internal_type_trait = Hypertrie_internal_t<boost_type_trait>;
*/

using LevelNodeStorage_t = LevelNodeStorage<2, ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t, m_alloc_t<int>>;
//using LevelNodeStorage_t = LevelNodeStorage<2, boost_internal_type_trait, m_alloc_t<int>>;

auto create_compressed_node(metall::manager::allocator_type<LevelNodeStorage_t::CompressedNode_type> alloc) {
	auto value = LevelNodeStorage_t::CompressedNode_type();
	value.key()[0]=5;
	auto address = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
	std::allocator_traits<decltype(alloc)>::construct(alloc, std::to_address(address), value);
	return address;
}

TEST_CASE("MetallAllocator -- create uncompressed node", "[NodeStorage]") {
	std::string path = "tmp";
	std::string name = "LevelNodeStorage_t_with_metall";
	// test write
	{
		metall::manager manager(metall::create_only, path.c_str());
		auto allocator = manager.get_allocator<>();
		auto level_storage = manager.construct<LevelNodeStorage_t>(name.c_str())(allocator);
	}

	{
		metall::manager manager(metall::open_only, path.c_str());
		auto level_storage = manager.find<LevelNodeStorage_t>(name.c_str()).first;
		auto allocator = manager.get_allocator<>();

		//WARN("is trivial " + std::to_string(std::is_trivial_v<TensorHash>));
		auto key = LevelNodeStorage_t::map_key_type{};
		auto value = create_compressed_node(allocator);
		auto pair = std::make_pair(key, value);

//		level_storage->compressedNodes()[LevelNodeStorage_t::map_key_type{}] = create_compressed_node(allocator);
		level_storage->compressedNodes().insert(pair);
		//level_storage->compressedNodes()[LevelNodeStorage_t::map_key_type{}]->key()[0] = 5;
		WARN(level_storage->compressedNodes()[LevelNodeStorage_t::map_key_type{}]->key()[0]);
		manager.flush();
	}
	std::cout << " " << std::endl;

	// test read
	{
		metall::manager manager(metall::open_only, path.c_str());
		auto level_storage = manager.find<LevelNodeStorage_t>(name.c_str()).first;
		auto &comNodes = level_storage->compressedNodes();
		auto alloc = comNodes.get_allocator();
		WARN("size at read:" + std::to_string(comNodes.size()));
		auto zw = comNodes.at(LevelNodeStorage_t::map_key_type{});
		auto key = zw->key();
		auto &value = key.at(0);
		WARN(value);
		REQUIRE(value == 5);
	}

	//destroy
	{
		metall::manager manager(metall::open_only, path.c_str());
		manager.destroy<LevelNodeStorage_t>(name.c_str());
	}
}
#endif//HYPERTRIE_TESTNODESTORAGE_HPP