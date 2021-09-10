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
	template<typename T>
	struct indirectly_readable_traits<boost::interprocess::offset_ptr<T>> {
		using value_type = typename boost::interprocess::offset_ptr<T>::value_type;
	};
}// namespace std


namespace details {
	template<typename T>
	using m_alloc_t = metall::manager::allocator_type<T>;
	using namespace hypertrie::internal::raw;
	template <typename NewAllocator, bool NewLSB, typename Key, typename Value, typename Allocator,
			template<typename, typename, typename> class Map,
			template<typename, typename> class Set, bool LSB_UNUSED>
	auto inject_allocator_type(Hypertrie_internal_t<hypertrie::Hypertrie_t<Key, Value, Allocator, Map, Set, LSB_UNUSED>>) {
		return Hypertrie_internal_t<hypertrie::Hypertrie_t<Key, Value, NewAllocator, Map, Set, NewLSB>>{};
	}
	using tri = ::hypertrie::internal::raw::lsbunused_bool_Hypertrie_internal_t;
	using offset_tri = decltype(inject_allocator_type<OffsetAllocator<std::byte>, true>(tri{}));
	using offset_tri_LSB_false = decltype(inject_allocator_type<OffsetAllocator<std::byte>, false>(tri{}));
	using metall_tri = decltype(inject_allocator_type<m_alloc_t<std::byte>, true>(tri{}));
	using metall_tri_LSB_false = decltype(inject_allocator_type<m_alloc_t<std::byte>, false>(tri{}));

	template<size_t depth>
	using LevelNodeStorage_t = LevelNodeStorage<depth, metall_tri>;
	template<size_t depth>
	using LevelNodeStorage_LSB_false_t = LevelNodeStorage<depth, metall_tri_LSB_false>;

	// N=1 not possible!
	template<size_t N>
	auto create_compressed_node(metall::manager::allocator_type<typename LevelNodeStorage_t<N>::CompressedNode_type> alloc) {
		auto address = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
		std::allocator_traits<decltype(alloc)>::construct(alloc, std::to_address(address));
		return address;
	}

	// N=1 not possible!
	template<size_t N>
	auto create_uncompressed_node(metall::manager::allocator_type<typename LevelNodeStorage_LSB_false_t<N>::UncompressedNode_type> alloc) {
		auto address = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
		std::allocator_traits<decltype(alloc)>::construct(alloc, std::to_address(address), 0, alloc);
		return address;
	}

	/** Contains all code to simplify interaction with metall.
	 */
	namespace m {
		void create_segment(std::string const& path) {
			metall::manager manager(metall::create_only, path.c_str());
		}

		struct Context {
			metall::manager manager;
			metall::manager::allocator_type<std::byte> allocator;
			Context(std::string const& path) : manager(metall::open_only, path.c_str()), allocator(manager.get_allocator()) {}

			template <typename T, typename ...Args>
			auto* construct(std::string const& name, Args&&... args) {
				return manager.template construct<T>(name.c_str())(std::forward<Args>(args)...);
			}
			template <typename T> auto* find(std::string const& name) {
				return manager.template find<T>(name.c_str()).first;
			}
			template <typename T> void destroy(std::string const& name){
				manager.template destroy<T>(name.c_str());
			}
		};
	}

	namespace bit{
		template <std::integral T>
		constexpr T msb_mask() noexcept {
			T mask = 1;
			return mask << (sizeof(T)*8 - 1);
		}

		template <std::integral T>
		constexpr T lsb_mask() noexcept {
			return static_cast<T>(1);
		}

		template <std::integral T>
		std::string binary(T value) {
			std::string result;
			result.reserve(sizeof(T)*8);
			for (T mask = msb_mask<T>(); mask != 0; mask >>= 1) {
				result.push_back((mask & value) != 0 ? '1' : '0');
			}
			return result;
		}

		template <std::integral T>
		auto set_single_bit(T value, size_t position, bool to_set) {
			assert(position < sizeof(value)*8 && "position value is too high");
			T mask = lsb_mask<T>() << position;
			if (to_set) {
				return value | mask;
			}
			return value & ~mask;
		}
	}
}// namespace details
using namespace details;


TEST_CASE("OffsetAllocator -- create uncompressed node", "[NodeStorage]") {
	using LevelNodeStorage_t = LevelNodeStorage<2, offset_tri>;
	OffsetAllocator<int> alloc;
	LevelNodeStorage_t level_node_storage(alloc);
}


template<size_t N>
void MetallAllocator_creat_compressed_nodes_in_LevelStorage() {
	using LevelNodeStorage_tt = LevelNodeStorage_t<N>;
	using map_key_type = typename LevelNodeStorage_tt::map_key_type;

	std::string path = "tmp";
	std::string name = "LevelNodeStorage_t_with_metall";

	m::create_segment(path);

	// write to segment (create level storage and add an entry)
	{
		m::Context con(path);
		con.construct<LevelNodeStorage_tt>(name, con.allocator);
		auto level_storage = con.find<LevelNodeStorage_tt>(name);
		auto &cnodes = level_storage->compressedNodes();
		auto hash = map_key_type{0};
		cnodes[hash] = create_compressed_node<N>(con.allocator);
		cnodes[hash]->key()[0] = 5;
	}

	// read and write
	{
		m::Context con(path);
		auto level_storage = con.find<LevelNodeStorage_tt>(name.c_str());
		auto &cnodes = level_storage->compressedNodes();
		auto hash = map_key_type{1};
		cnodes[hash] = create_compressed_node<N>(con.allocator);
		cnodes[hash]->key()[0] = 4;

		REQUIRE(cnodes.at(map_key_type{0})->key()[0] == 5);
		REQUIRE(cnodes.at(map_key_type{1})->key()[0] == 4);
	}

	{
		m::Context con(path);
		auto level_storage = con.find<LevelNodeStorage_tt>(name);
		auto &cnodes = level_storage->compressedNodes();

		REQUIRE(cnodes.at(map_key_type{0})->key()[0] == 5);
		REQUIRE(cnodes.at(map_key_type{1})->key()[0] == 4);
	}

	{
		m::Context con(path);
		con.destroy<LevelNodeStorage_tt>(name);
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
	using LevelNodeStorage_tt = LevelNodeStorage_LSB_false_t<N>;
	using map_key_type = typename LevelNodeStorage_tt::map_key_type;

	std::string path = "tmp";
	std::string name = "LevelNodeStorage_t_with_metall";

	m::create_segment(path);

	// write to segment (create level storage and add an entry)
	{
		m::Context con(path);
		con.construct<LevelNodeStorage_tt>(name, con.allocator);
		auto level_storage = con.find<LevelNodeStorage_tt>(name);

		auto &ucnodes = level_storage->uncompressedNodes();
		auto new_ucnode = create_uncompressed_node<N>(con.allocator);

		if constexpr (N >= 3) {
			new_ucnode->edges(0)[0] = TensorHash(5);
		} else if constexpr (N == 2) {
			new_ucnode->edges(0)[0] = TensorHash(TaggedTensorHash<tri>(55));
		} else {
			new_ucnode->edges().insert(0);
		}

		auto hash = map_key_type{0};
		ucnodes[hash] = new_ucnode;
	}

	// read and write
	{
		m::Context con(path);
		auto level_storage = con.find<LevelNodeStorage_tt>(name);
		auto &ucnodes = level_storage->uncompressedNodes();
		auto new_ucnode = create_uncompressed_node<N>(con.allocator);

		if constexpr (N >= 3) {
			new_ucnode->edges(0)[1] = TensorHash(6);
		} else if constexpr (N == 2) {
			new_ucnode->edges(0)[1] = TensorHash(TaggedTensorHash<tri>(66));
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
		m::Context con(path);
		auto level_storage = con.find<LevelNodeStorage_tt>(name);

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
		m::Context con(path);
		con.destroy<LevelNodeStorage_tt>(name);
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


TEST_CASE("NodeStorage constructor compiles with std::allocator", "[NodeStorage]") {
	std::allocator<int> alloc;
	NodeStorage<1> store(alloc);
}

TEST_CASE("NodeStorage constructor compiles with OffsetAllocator", "[NodeStorage]") {
	OffsetAllocator<int> alloc;
	NodeStorage<1, offset_tri> store(alloc);
}

TEST_CASE("NodeStorage newCompressedNode with std::allocator", "[NodeStorage]") {
	std::allocator<int> alloc;
	NodeStorage<1> store(alloc);
	NodeStorage<1>::RawKey<1> key{0};
	TensorHash hash{42};
	bool value = true;
	size_t ref_count = 0;
	store.newCompressedNode(key, value, ref_count, hash);
	auto container42 = store.getCompressedNode<1>(hash);
	REQUIRE(not container42.empty());
	REQUIRE(container42.compressed_node()->value() == true);
	auto container43 = store.getCompressedNode<1>(TensorHash{43});
	REQUIRE(container43.empty());
}

TEST_CASE("NodeStorage newCompressedNode with OffsetAllocator", "[NodeStorage]") {
	//create store
	OffsetAllocator<size_t> alloc;
	NodeStorage<1, offset_tri_LSB_false> store(alloc);
	//create value to store
	NodeStorage<1>::RawKey<1> key{0};
	TensorHash hash{42};
	bool value = true;
	size_t ref_count = 0;
	auto container42or = store.newCompressedNode(key, value, ref_count, hash);
	//get value back + check
	auto container42 = store.getCompressedNode<1>(hash);
	REQUIRE(container42.compressed_node() == container42or.compressed_node());
	REQUIRE(not container42.empty());
	REQUIRE(container42.compressed_node()->value() == true);
	//try not-set value + check
	auto container43 = store.getCompressedNode<1>(TensorHash{43});
	REQUIRE((container43.empty()));
//	REQUIRE(std::is_trivially_default_constructible_v<RawNodeContainer<OffsetAllocator<size_t>>>);
}

TEST_CASE("NodeStorage newCompressedNode with Metall", "[NodeStorage]") {
	// "globals"
	using NodeStorage_t = NodeStorage<1, Hypertrie_internal_t<>>;
	std::string path = "tmp";
	std::string name = "NodeStorage_with_metall";
	TensorHash true_hash{42};
	TensorHash false_hash{43};
	m::create_segment(path);
	// write to segment (create store)
	{
		m::Context con(path);
		auto store = con.construct<NodeStorage_t>(name, con.allocator);
		NodeStorage<1>::RawKey<1> key{0};
		store->newCompressedNode(key, true, 0, true_hash);
	}
	// read
	{
		m::Context con(path);
		auto store = con.find<NodeStorage_t>(name);
		auto container42 = store->getCompressedNode<1>(true_hash);
		REQUIRE(not container42.empty());
		REQUIRE(container42.compressed_node()->value() == true);
		auto container43 = store->getCompressedNode<1>(false_hash);
		REQUIRE((container43.empty()));
	}
	// destroy segment
	{
		m::Context con(path);
		con.destroy<NodeStorage_t>(name);
	}
}

TensorHash make_compressed(TensorHash const &hash) {
	return TensorHash(bit::set_single_bit(hash.hash(), hash.compression_tag_pos, hash.compressed_tag));
}

TEST_CASE("NodeStorage deleteNode with std::allocator", "[NodeStorage]") {
	std::allocator<int> alloc;
	NodeStorage<1> store(alloc);
	NodeStorage<1>::RawKey<1> key{0};
	auto hash = make_compressed(TensorHash(42));
	bool value = true;
	size_t ref_count = 0;
	store.newCompressedNode(key, value, ref_count, hash);
	store.deleteNode<1>(hash);
	auto container = store.getCompressedNode<1>(hash);
	REQUIRE((container.empty()));
}


TEST_CASE("NodeStorage deleteNode with OffsetAllocator", "[NodeStorage]") {
	using NodeStorage_t = NodeStorage<1, offset_tri_LSB_false>;
	OffsetAllocator<size_t> alloc;
	NodeStorage_t store(alloc);
	NodeStorage_t::RawKey<1> key{0};
	auto hash = make_compressed(TensorHash(42));
	bool value = true;
	size_t ref_count = 0;
	store.newCompressedNode(key, value, ref_count, hash);
	store.deleteNode<1>(hash);
	auto container = store.getCompressedNode<1>(hash);
	REQUIRE((container.empty()));
}

TEST_CASE("NodeStorage deleteNode with metall", "[NodeStorage]") {
	using NodeStorage_t = NodeStorage<1, Hypertrie_internal_t<>>;
	std::string path = "tmp";
	std::string name = "NodeStorage_with_metall_delete";
	auto hash = make_compressed(TensorHash(42));

	m::create_segment(path);

	//create data
	{
		m::Context con(path);
		auto store = con.construct<NodeStorage_t>(name, con.allocator);
		NodeStorage_t::RawKey<1> key{0};
		bool value = true;
		size_t ref_count = 0;
		store->newCompressedNode(key, value, ref_count, hash);
	}

	//delete data
	{
		m::Context con(path);
		auto store = con.find<NodeStorage_t>(name);
		store->deleteNode<1>(hash);
		auto container = store->getCompressedNode<1>(hash);
		REQUIRE((container.empty()));
	}

	//destroy segment
	{
		m::Context con(path);
		con.destroy<NodeStorage_t>(name);
	}
}

#endif//HYPERTRIE_TESTNODESTORAGE_HPP