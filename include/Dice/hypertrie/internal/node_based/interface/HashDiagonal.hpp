#ifndef HYPERTRIE_HASHDIAGONAL_HPP
#define HYPERTRIE_HASHDIAGONAL_HPP

#include "Dice/hypertrie/internal/node_based/raw/iterator/Diagonal.hpp"
#include "Dice/hypertrie/internal/node_based/interface/Hypertrie_predeclare.hpp"

namespace hypertrie::internal::node_based {

	template<HypertrieTrait tr_t = default_bool_Hypertrie_t>
	class HashDiagonal {
		using tr = tr_t;
		using tri = raw::Hypertrie_internal_t<tr>;
		using NodeContainer = typename raw::RawNodeContainer;
		using KeyPositions = typename tr::KeyPositions;
		template <size_t depth>
		using RawKeyPositions = typename  tri::DiagonalPositions;
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		template <size_t diag_depth, size_t depth, NodeCompression compression>
		using RawHashDiagonal = typename raw::template HashDiagonal<diag_depth, depth, tri>;
	protected:
		using Key = typename tr::Key;

		struct RawMethods {
			void *(*construct)(NodeContainer *, const KeyPositions &, HypertrieContext<tr> *);

			void (*destruct)(void *);

			void (*begin)(void *);

			key_part_type (*currentKeyPart)(void const *);

			value_type (*currentScalar)(void const *);

			NodeContainer (*currentHypertrie)(void const *);

			bool (*find)(void *, key_part_type);

			void (*inc)(void *);

			bool (*ended)(void const *);

			size_t (*size)(void const *);
		};

		template <size_t diag_depth, size_t depth, NodeCompression compression>
		inline static RawMethods generateRawMethods() {
			using RawDiagonalHash_t = RawHashDiagonal<diag_depth, depth, compression>;
			return RawMethods(
					// construct
					[](const const_Hypertrie<tr> &hypertrie, const KeyPositions &diagonal_poss) -> void * {
						using NodecType = typename raw::template SpecificNodeContainer<depth, compression, tri>;

						RawKeyPositions<depth> raw_diag_poss{};
						for (auto pos : diagonal_poss)
							raw_diag_poss[pos] = true;
						return new RawHashDiagonal<diag_depth, depth, compression>(
								*const_cast<NodecType *>(reinterpret_cast<const NodecType *>(hypertrie.rawNodeContainer())),
								raw_diag_poss,
								hypertrie.context()->rawContext());
					},
					// destruct
					[](void * raw_diagonal_ptr) {
						delete reinterpret_cast<RawDiagonalHash_t *>(raw_diagonal_ptr);
					},
					// begin
					[](void *raw_diagonal_ptr) {
					  	auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_t *>(raw_diagonal_ptr);
						raw_diagonal.begin();
					},
					// currentKeyPart
					[] (void const *raw_diagonal_ptr) -> key_part_type {
					  const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_t *>(raw_diagonal_ptr);
					  return raw_diagonal.currentKeyPart();
					},
					// currentHypertrie // TODO: go on here
					&RawIterator<depth>::value,
					&RawIterator<depth>::inc,
					&RawIterator<depth>::ended);
		}

		inline static const std::vector<RawMethods> raw_method_cache = [](){
		  std::vector<RawMethods> raw_methods;
		  for(size_t depth : iter::range(1UL,hypertrie_depth_limit))
			  raw_methods.push_back(compiled_switch<hypertrie_depth_limit, 1>::switch_(
					  depth,
					  [](auto depth_arg) -> RawMethods {
						return generateRawMethods<depth_arg>();
					  },
					  []() -> RawMethods { assert(false); throw std::logic_error{"Something is really wrong."}; }));
		  return raw_methods;
		}();

		static RawMethods const &getRawMethods(pos_type depth) {
			return raw_method_cache[depth - 1];
		};
	};
}

#endif//HYPERTRIE_HASHDIAGONAL_HPP
