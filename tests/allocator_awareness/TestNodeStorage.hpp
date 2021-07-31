#ifndef HYPERTRIE_TESTNODESTORAGE_HPP
#define HYPERTRIE_TESTNODESTORAGE_HPP
#include <catch2/catch.hpp>

#include <Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp>
#include <Dice/hypertrie/internal/raw/storage/NodeStorage.hpp>

#include "OffsetAllocator.hpp"
#include <metall/metall.hpp>
#include <tsl/boost_offset_pointer.h>

//gcc-10 would not compile without this specialisation
namespace std {
    template <typename T>
    struct indirectly_readable_traits<boost::interprocess::offset_ptr<T>> {
        using value_type = typename boost::interprocess::offset_ptr<T>::value_type;
    };
}

TEST_CASE("OffsetAllocator -- create uncompressed node", "[NodeStorage]") {

	using namespace hypertrie::internal::raw;

	using LevelNodeStorage_t = LevelNodeStorage<2, ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t, OffsetAllocator<int>>;

	LevelNodeStorage_t level_node_storage(OffsetAllocator<int>{});
}

namespace details {
	template<typename T>
	using m_alloc_t = metall::manager::allocator_type<T>;

	using namespace hypertrie::internal::raw;
	/*
    using tri = hypertrie::Hypertrie_t<unsigned long,
		bool,
		hypertrie::internal::container::boost_flat_map ,
		hypertrie::internal::container::boost_flat_set>;
    using boost_internal_type_trait = Hypertrie_internal_t<tri>;
    */
	using tri = ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t;

	template<size_t depth>
	using LevelNodeStorage_t = LevelNodeStorage<depth, tri, m_alloc_t<int>>;

    // N=1 not possible!
    template<size_t N>
    auto create_compressed_node(metall::manager::allocator_type<typename LevelNodeStorage_t<N>::CompressedNode_type> alloc) {
        auto address = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
        std::allocator_traits<decltype(alloc)>::construct(alloc, std::to_address(address));
        return address;
    }

    // N=1 not possible!
    template<size_t N>
    auto create_uncompressed_node(metall::manager::allocator_type<typename LevelNodeStorage_t<N>::UncompressedNodeMap_type> alloc) {
        auto address = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
        std::allocator_traits<decltype(alloc)>::construct(alloc, std::to_address(address), 0, alloc);
        return address;
    }
}
using namespace details;



template<size_t N>
void MetallAllocator_creat_compressed_nodes_in_LevelStorage() {
	using LevelNodeStorage_tt = LevelNodeStorage_t<N>;
	using map_key_type = typename LevelNodeStorage_tt::map_key_type;

	std::string path = "tmp";
	std::string name = "LevelNodeStorage_t_with_metall";
	// create segment
	{
		metall::manager manager(metall::create_only, path.c_str());
	}

	// write to segment (create level storage and add an entry)
	{
		metall::manager manager(metall::open_only, path.c_str());
		auto allocator = manager.get_allocator<>();
		manager.construct<LevelNodeStorage_tt>(name.c_str())(allocator);
		auto level_storage = manager.find<LevelNodeStorage_tt>(name.c_str()).first;

		auto &cnodes = level_storage->compressedNodes();
		auto hash = map_key_type{0};
		cnodes[hash] = create_compressed_node<N>(allocator);
		cnodes[hash]->key()[0] = 5;
	}

	// read and write
	{
		metall::manager manager(metall::open_only, path.c_str());
		auto level_storage = manager.find<LevelNodeStorage_tt>(name.c_str()).first;
		auto allocator = manager.get_allocator<>();

		auto &cnodes = level_storage->compressedNodes();
		auto hash = map_key_type{1};
		cnodes[hash] = create_compressed_node<N>(allocator);
		cnodes[hash]->key()[0] = 4;

		REQUIRE(cnodes.at(map_key_type{0})->key()[0] == 5);
		REQUIRE(cnodes.at(map_key_type{1})->key()[0] == 4);
	}

	// read
	{
		metall::manager manager(metall::open_only, path.c_str());
		auto level_storage = manager.find<LevelNodeStorage_tt>(name.c_str()).first;

		auto &cnodes = level_storage->compressedNodes();

		REQUIRE(cnodes.at(map_key_type{0})->key()[0] == 5);
		REQUIRE(cnodes.at(map_key_type{1})->key()[0] == 4);
	}

	// destroy segment
	{
		metall::manager manager(metall::open_only, path.c_str());
		manager.destroy<LevelNodeStorage_tt>(name.c_str());
	}
}

TEST_CASE("MetallAllocator -- create compressed node depth 2", "[NodeStorage]") {
	MetallAllocator_creat_compressed_nodes_in_LevelStorage<2>();
}

TEST_CASE("MetallAllocator -- create compressed node depth 3", "[NodeStorage]") {
	MetallAllocator_creat_compressed_nodes_in_LevelStorage<3>();
}


template<size_t N>
void MetallAllocator_creat_uncompressed_nodes_in_LevelStorage() {
	using LevelNodeStorage_tt = LevelNodeStorage_t<N>;
	using map_key_type = typename LevelNodeStorage_tt::map_key_type;

	std::string path = "tmp";
	std::string name = "LevelNodeStorage_t_with_metall";
	// create segment
	{
		metall::manager manager(metall::create_only, path.c_str());
	}

	// write to segment (create level storage and add an entry)
	{
		metall::manager manager(metall::open_only, path.c_str());
		auto allocator = manager.get_allocator<>();
		manager.construct<LevelNodeStorage_tt>(name.c_str())(allocator);
		auto level_storage = manager.find<LevelNodeStorage_tt>(name.c_str()).first;

		auto &ucnodes = level_storage->uncompressedNodes();
		auto new_ucnode = create_uncompressed_node<N>(allocator);

		if constexpr (N >= 3) {
			new_ucnode->edges(0)[0] = TensorHash(5);
		} else if constexpr (N == 2) {
			new_ucnode->edges(0)[0] = TaggedTensorHash<tri>(55);
		} else {
			new_ucnode->edges().insert(0);
		}

		auto hash = map_key_type{0};
		ucnodes[hash] = new_ucnode;
	}

	// read and write
	{
		metall::manager manager(metall::open_only, path.c_str());
		auto level_storage = manager.find<LevelNodeStorage_tt>(name.c_str()).first;
		auto allocator = manager.get_allocator<>();

		auto &ucnodes = level_storage->uncompressedNodes();
		auto new_ucnode = create_uncompressed_node<N>(allocator);

		if constexpr (N >= 3) {
			new_ucnode->edges(0)[1] = TensorHash(6);
		} else if constexpr (N == 2) {
			new_ucnode->edges(0)[1] = TaggedTensorHash<tri>(66);
		} else {
			new_ucnode->edges().insert(1);
		}
		auto hash = map_key_type{1};
		ucnodes[hash] = new_ucnode;

		if constexpr (N >= 3) {
			REQUIRE(ucnodes.at(map_key_type{0})->edges(0).at(0) == TensorHash{5});
			REQUIRE(ucnodes.at(map_key_type{1})->edges(0).at(1) == TensorHash{6});
		} else if constexpr (N == 2) {
			REQUIRE(ucnodes.at(map_key_type{0})->edges(0).at(0) == TaggedTensorHash<tri>{55});
			REQUIRE(ucnodes.at(map_key_type{1})->edges(0).at(1) == TaggedTensorHash<tri>{66});
		} else {
			REQUIRE(ucnodes.at(map_key_type{0})->edges().count(0));
			REQUIRE(ucnodes.at(map_key_type{1})->edges().count(1));
		}

	}

	// read
	{
		metall::manager manager(metall::open_only, path.c_str());
		auto level_storage = manager.find<LevelNodeStorage_tt>(name.c_str()).first;

		auto &ucnodes = level_storage->uncompressedNodes();

		if constexpr (N >= 3) {
			REQUIRE(ucnodes.at(map_key_type{0})->edges(0).at(0) == TensorHash{5});
			REQUIRE(ucnodes.at(map_key_type{1})->edges(0).at(1) == TensorHash{6});
		} else if constexpr (N == 2) {
			REQUIRE(ucnodes.at(map_key_type{0})->edges(0).at(0) == TaggedTensorHash<tri>{55});
			REQUIRE(ucnodes.at(map_key_type{1})->edges(0).at(1) == TaggedTensorHash<tri>{66});
		} else {
			REQUIRE(ucnodes.at(map_key_type{0})->edges().count(0));
			REQUIRE(ucnodes.at(map_key_type{1})->edges().count(1));
		}
	}

	// destroy segment
	{
		metall::manager manager(metall::open_only, path.c_str());
		manager.destroy<LevelNodeStorage_tt>(name.c_str());
	}
}


TEST_CASE("MetallAllocator -- create uncompressed node depth 3", "[NodeStorage]") {
	MetallAllocator_creat_uncompressed_nodes_in_LevelStorage<3>();
}
TEST_CASE("MetallAllocator -- create uncompressed node depth 2", "[NodeStorage]") {
	MetallAllocator_creat_uncompressed_nodes_in_LevelStorage<2>();
}

TEST_CASE("MetallAllocator -- create uncompressed node depth 1", "[NodeStorage]") {
	MetallAllocator_creat_uncompressed_nodes_in_LevelStorage<1>();
}


TEST_CASE("NodeStorage constructor compiles with std::allocator", "[NodeStorage]"){
	std::allocator<int> alloc;
	NodeStorage<1> store(alloc);
}

TEST_CASE("NodeStorage constructor compiles with OffsetAllocator", "[NodeStorage]"){
    OffsetAllocator<int> alloc;
    NodeStorage<1, Hypertrie_internal_t<>, OffsetAllocator<int>> store(alloc);
}

TEST_CASE("NodeStorage newCompressedNode with std::allocator", "[NodeStorage]"){
    using hypertrie::internal::RawKey;
    std::allocator<int> alloc;
    NodeStorage<1> store(alloc);
    NodeStorage<1>::RawKey<1> key {0};
    TensorHash hash {42};
    bool value = true;
    size_t ref_count = 0;
    store.newCompressedNode(key, value, ref_count, hash);
	auto container42 = store.getCompressedNode<1>(hash);
	REQUIRE(not container42.empty());
	REQUIRE(container42.compressed_node()->value() == true);
	auto container43 = store.getCompressedNode<1>(TensorHash{43});
	REQUIRE(container43.empty());
}

TEST_CASE("NodeStorage newCompressedNode with OffsetAllocator", "[NodeStorage]"){
using hypertrie::internal::RawKey;
OffsetAllocator<int> alloc;
NodeStorage<1, Hypertrie_internal_t<>, OffsetAllocator<int>> store(alloc);
NodeStorage<1>::RawKey<1> key {0};
TensorHash hash {42};
bool value = true;
size_t ref_count = 0;
store.newCompressedNode(key, value, ref_count, hash);

/*
REQUIRE((store.getCompressedNode<1>(hash).compressed_node()->value() == true));
//why is this true???
REQUIRE((store.getCompressedNode<1>(TensorHash{43}).compressed_node()->value() == true));
*/
}


#endif//HYPERTRIE_TESTNODESTORAGE_HPP