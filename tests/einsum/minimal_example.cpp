#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <boost/unordered_map.hpp>
#include <doctest/doctest.h>
//#include <metall/metall.hpp>
#include "../metall_mute_warnings.hpp"
#include "dice/hash/DiceHash.hpp"
#include "dice/hypertrie/internal/container/SparseSet.hpp"
#include "dice/hypertrie/internal/raw/node/Identifier.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include <memory>
#include <tuple>

template<typename Key, typename Mapped, typename Alloc = std::allocator<std::byte>>
using bmap = boost::unordered_map<Key,
								  Mapped,
								  dice::hash::DiceHash<Key>,
								  std::equal_to<Key>,
								  typename std::allocator_traits<Alloc>::template rebind_alloc<std::pair<Key, Mapped>>>;

template<typename T = std::byte>
using metall_allocator = metall::manager::allocator_type<T>;

struct ExampleStruct {
	int data;
	ExampleStruct() = delete;
	explicit ExampleStruct(int data) : data(data) {}
	ExampleStruct(ExampleStruct const &second) = delete;
	ExampleStruct(ExampleStruct &&second) = delete;
	ExampleStruct operator=(ExampleStruct const &second) = delete;
	ExampleStruct operator=(ExampleStruct &&second) = delete;

	friend std::ostream &operator<<(std::ostream &os, ExampleStruct &s) {
		return os << "ExampleStruct{" << s.data << "}";
	}
};

struct ComplicatedExampleStruct {
	using Alloc = metall_allocator<std::pair<int const, int>>;
	bmap<int, int, Alloc> data;

	ComplicatedExampleStruct() = delete;
	explicit ComplicatedExampleStruct(unsigned count, Alloc const &alloc) : data(alloc) {
		for (unsigned i = 0; i < count; ++i) {
			data.insert({i, i});
		}
	}
	ComplicatedExampleStruct(ExampleStruct const &second) = delete;
	ComplicatedExampleStruct(ExampleStruct &&second) = delete;
	ComplicatedExampleStruct operator=(ComplicatedExampleStruct const &second) = delete;
	ComplicatedExampleStruct operator=(ComplicatedExampleStruct &&second) = delete;

	friend std::ostream &operator<<(std::ostream &os, ComplicatedExampleStruct &s) {
		os << "[";
		for (auto [key, value] : s.data) {
			os << key << ":" << value << ", ";
		}
		return os << "\b\b]";
	}
};

namespace dice::hash {
	template<typename Policy>
	struct dice_hash_overload<Policy, ComplicatedExampleStruct> {
		static std::size_t dice_hash(ComplicatedExampleStruct const &x) noexcept {
			return DiceHash<decltype(x.data)>{}(x.data);
		}
	};
}// namespace dice::hash

TEST_SUITE("Minimal tests: is the rehashing itself the bug (in metall)?") {
	template<typename Allocator, typename... Args>
	auto generateEntry(Allocator & alloc, Args && ...args) {
		auto pointer = std::allocator_traits<Allocator>::allocate(alloc, 1);
		std::allocator_traits<Allocator>::construct(alloc, std::to_address(pointer), std::forward<Args>(args)...);
		return pointer;
	}

	template<typename Allocator, typename Pointer_t>
	void destroyEntry(Allocator & alloc, Pointer_t pointer) {
		std::allocator_traits<Allocator>::destroy(alloc, std::to_address(pointer));
		std::allocator_traits<Allocator>::deallocate(alloc, pointer, 1);
	}

	template<typename T>
	auto createFreshManagerAndAllocator() {
		//create segment
		std::string const path = "/tmp";
		{
			metall::manager manager(metall::create_only, path.c_str());
		}
		//create allocator and manager to use
		metall::manager manager(metall::open_only, path.c_str());
		auto metall_alloc = manager.get_allocator<T>();
		return std::make_tuple(std::move(manager), std::move(metall_alloc));
	}

	template<typename Map, typename Alloc>
	void inserting(Map & map, Alloc & alloc, int start, int end) {
		unsigned rehashCounter = 0;
		float load;
		for (int i = start; i < end; ++i) {
			load = map.load_factor();
			map.insert({i, generateEntry(alloc, i)});
			rehashCounter += (map.load_factor() < load);
		}
		std::cout << rehashCounter << " times rehashed" << std::endl;
	}


	template<typename Map, typename Alloc>
	void inserting_with_hint(Map & map, Alloc & alloc, int start, int end) {
		unsigned rehashCounter = 0;
		float load;
		map.insert({start, generateEntry(alloc, start)});
		for (int i = start + 1; i < end; ++i) {
			load = map.load_factor();
			map.insert(map.find(i - 1), {i, generateEntry(alloc, i)});
			rehashCounter += (map.load_factor() < load);
		}
		std::cout << rehashCounter << " times rehashed" << std::endl;
	}

	template<typename Map, typename Alloc>
	void destroying(Map & map, Alloc & alloc) {
		for (auto [key, pointer] : map) {
			destroyEntry(alloc, pointer);
		}
	}

	TEST_CASE("Generate boost::unordered_map of int->ptr") {
		auto [manager, alloc] = createFreshManagerAndAllocator<int>();
		boost::unordered_map<int, decltype(generateEntry(alloc, 42))> map;
		inserting(map, alloc, 0, 10'000);
		destroying(map, alloc);
	}

	TEST_CASE("Generate boost::unordered_map of int->ptr directly with metall") {
		auto [manager, alloc] = createFreshManagerAndAllocator<int>();
		using alloc_type = std::remove_cvref_t<decltype(alloc)>;
		using pointer_type = decltype(generateEntry(alloc, 42));
		using currentMap = bmap<int, pointer_type, alloc_type>;
		const std::string name = "mapName1";
		auto map = *manager.construct<currentMap>(name.c_str())(alloc);
		inserting(map, alloc, 0, 10'000);
		destroying(map, alloc);
		manager.destroy<currentMap>(name.c_str());
	}

	TEST_CASE("Generate boost::unordered_map of int->ptr directly with metall, let the map allocate the memory") {
		auto [manager, alloc] = createFreshManagerAndAllocator<int>();
		using alloc_type = std::remove_cvref_t<decltype(alloc)>;
		using pointer_type = decltype(generateEntry(alloc, 42));
		using currentMap = bmap<int, pointer_type, alloc_type>;
		const std::string name = "mapName1";
		auto map = *manager.construct<currentMap>(name.c_str())(alloc);
		inserting(map, alloc, 0, 10'000);
		destroying(map, alloc);
		manager.destroy<currentMap>(name.c_str());
	}


	TEST_CASE("Generate boost::unordered_map of int->ptr of example struct") {
		auto [manager, alloc] = createFreshManagerAndAllocator<ExampleStruct>();
		boost::unordered_map<int, decltype(generateEntry(alloc, 42))> map;
		inserting(map, alloc, 0, 10'000);
		destroying(map, alloc);
	}

	TEST_CASE("Generate boost::unordered_map of int->ptr of example struct directly with metall") {
		auto [manager, alloc] = createFreshManagerAndAllocator<ExampleStruct>();
		using alloc_type = std::remove_cvref_t<decltype(alloc)>;
		using pointer_type = decltype(generateEntry(alloc, 42));
		using currentMap = bmap<int, pointer_type, alloc_type>;
		const std::string name = "mapName2";
		auto map = *manager.construct<currentMap>(name.c_str())(alloc);
		inserting(map, alloc, 0, 10'000);
		destroying(map, alloc);
		manager.destroy<currentMap>(name.c_str());
	}

	TEST_CASE("Generate boost::unordered_map of int->ptr of example struct, inserting with hint iterator") {
		auto [manager, alloc] = createFreshManagerAndAllocator<ExampleStruct>();
		boost::unordered_map<int, decltype(generateEntry(alloc, 42))> map;
		inserting_with_hint(map, alloc, 0, 10'000);
		destroying(map, alloc);
	}

	TEST_CASE("Generate boost::unordered_map of int->ptr of example struct directly with metall, inserting with hint iterator") {
		auto [manager, alloc] = createFreshManagerAndAllocator<ExampleStruct>();
		using alloc_type = std::remove_cvref_t<decltype(alloc)>;
		using pointer_type = decltype(generateEntry(alloc, 42));
		using currentMap = bmap<int, pointer_type, alloc_type>;
		const std::string name = "mapName2";
		auto map = *manager.construct<currentMap>(name.c_str())(alloc);
		inserting_with_hint(map, alloc, 0, 10'000);
		destroying(map, alloc);
		manager.destroy<currentMap>(name.c_str());
	}

	template<typename Map, typename Alloc>
	void insertingWithAllocator(Map & map, Alloc & alloc, int start, int end) {
		unsigned rehashCounter = 0;
		float load;
		for (int i = start; i < end; ++i) {
			load = map.load_factor();
			map.insert({i, generateEntry(alloc, 50, alloc)});
			rehashCounter += (map.load_factor() < load);
		}
		std::cout << rehashCounter << " times rehashed" << std::endl;
	}

	template<typename Map, typename Alloc>
	void insertingWithAllocatorWithHint(Map & map, Alloc & alloc, int start, int end) {
		unsigned rehashCounter = 0;
		float load;
		map.insert({start, generateEntry(alloc, 50, alloc)});
		for (int i = start + 1; i < end; ++i) {
			load = map.load_factor();
			map.insert(map.find(i - 1), {i, generateEntry(alloc, 50, alloc)});
			rehashCounter += (map.load_factor() < load);
		}
		std::cout << rehashCounter << " times rehashed" << std::endl;
	}

	TEST_CASE("Generate boost::unordered_map of int->ptr of complicated example struct") {
		auto [manager, alloc] = createFreshManagerAndAllocator<ComplicatedExampleStruct>();
		boost::unordered_map<int, decltype(generateEntry(alloc, 10, alloc))> map;
		insertingWithAllocator(map, alloc, 0, 10'000);
		destroying(map, alloc);
	}

	TEST_CASE("Generate boost::unordered_map of int->ptr of example struct directly with metall, inserting with hint iterator") {
		auto [manager, alloc] = createFreshManagerAndAllocator<ComplicatedExampleStruct>();
		using alloc_type = std::remove_cvref_t<decltype(alloc)>;
		using pointer_type = decltype(generateEntry(alloc, 10, alloc));
		using currentMap = bmap<int, pointer_type, alloc_type>;
		const std::string name = "mapName3";
		auto map = *manager.construct<currentMap>(name.c_str())(alloc);
		insertingWithAllocatorWithHint(map, alloc, 0, 10'000);
		destroying(map, alloc);
		manager.destroy<currentMap>(name.c_str());
	}
}

TEST_SUITE("Minimal tests: with exactly the same map as in the error case") {
	using trait = dice::hypertrie::Hypertrie_t<
			unsigned long,
			bool,
			bmap,
			dice::hypertrie::internal::container::dice_sparse_set,
			-1>;
	using RawIdent = dice::hypertrie::internal::raw::RawIdentifier<5, trait>;
	using RawK = dice::hypertrie::internal::raw::RawKey<5, trait>;
	using SingleEntryN = dice::hypertrie::internal::raw::SingleEntryNode<5, trait, std::allocator<std::byte>>;
	using SingleE = dice::hypertrie::internal::raw::SingleEntry<5, trait>;
	using pointer = boost::interprocess::offset_ptr<
			SingleEntryN, long, unsigned long, 0>;
	using specificMap = bmap<RawIdent, pointer, metall_allocator<>>;
	/**
	 * It does not break at the rehashing.
	 * It should.
	 * So now we know that the problem is not based on the types alone,
	 * but might be a combination of different things.
	 */
	TEST_CASE("this should crash at the rehashing") {
		//create segment
		std::string const path = "\tmp";
		{
			metall::manager manager(metall::create_only, path.c_str());
		}
		//create allocator and manager to use
		metall::manager manager(metall::open_only, path.c_str());
		auto metall_alloc = manager.get_allocator<>();
		specificMap map(metall_alloc);

		auto createSingleEntryNode = [&metall_alloc](auto const &...parameter) {
			using basic_metall = std::remove_cvref_t<decltype(metall_alloc)>;
			using metall_rebind = std::allocator_traits<basic_metall>::rebind_alloc<SingleEntryN>;
			using rebind = std::allocator_traits<metall_rebind>;
			metall_rebind alloc = metall_alloc;
			auto ptr = rebind::allocate(alloc, 1);
			rebind::construct(alloc, std::to_address(ptr), parameter...);
			return ptr;
		};

		auto createMapElement = [&createSingleEntryNode](unsigned long lastKey) {
			RawK key{{0, 0, 0, 0, lastKey}};
			SingleE entry{key, true};
			RawIdent ident(entry);
			auto entryNode = createSingleEntryNode(entry);
			return std::make_pair(ident, std::move(entryNode));
		};

		auto refactorPrint = [](auto const &map) {
			static float last_current = 0.0;
			std::cout << std::fixed << map.load_factor() << ", " << map.max_load_factor() << (last_current > map.load_factor() ? " (rehashed)" : "") << std::endl;
			last_current = map.load_factor();
		};

		std::cout << "inserting with unique keys" << std::endl;
		for (unsigned long i = 0; i < 20; ++i) {
			map.insert(createMapElement(i));
			refactorPrint(map);
		}

		std::cout << "\ninserting with unique keys and a hint iterator (always end)" << std::endl;
		for (unsigned long i = 20; i < 40; ++i) {
			map.insert(map.end(), createMapElement(i));
			refactorPrint(map);
		}

		std::cout << "\ninserting with already used keys" << std::endl;
		for (unsigned long i = 0; i < 20; ++i) {
			map.insert(createMapElement(i));
			refactorPrint(map);
		}

		std::cout << "\ninserting with already used keys with an hint iterator" << std::endl;
		for (unsigned long i = 20; i < 40; ++i) {
			auto element = createMapElement(i);
			map.insert(map.find(element.first), element);
			refactorPrint(map);
		}

		std::cout << "\ninserting with unique keys and a hint iterator (always end), again" << std::endl;
		for (unsigned long i = 40; i < 80; ++i) {
			map.insert(map.end(), createMapElement(i));
			refactorPrint(map);
		}
	}
}