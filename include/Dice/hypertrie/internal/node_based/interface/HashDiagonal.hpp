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
		template <size_t diag_depth, size_t depth, typename raw::NodeCompression compression>
		using RawHashDiagonal = typename raw::template HashDiagonal<diag_depth, depth, compression, tri>;
	protected:
		using Key = typename tr::Key;

		struct RawMethods {
			void *(*construct)(NodeContainer *, const KeyPositions &, HypertrieContext<tr> *);

			void (*destruct)(void *);

			void (*begin)(void *);

			key_part_type (*currentKeyPart)(void const *);

			const_Hypertrie<tr> (*currentHypertrie)(void const *);

			value_type (*currentScalar)(void const *);

			bool (*find)(void *, key_part_type);

			void (*inc)(void *);

			bool (*ended)(void const *);

			size_t (*size)(void const *);
		};

		template <size_t diag_depth, size_t depth, typename raw::NodeCompression compression>
		inline static RawMethods generateRawMethods() {
			[[maybe_unused]] constexpr static const size_t result_depth = depth - diag_depth;
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
					[] (void const *raw_diagonal_ptr, HypertrieContext<tr> *context) -> const_Hypertrie<tr> {
				if constexpr (diag_depth < depth) {
					const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_t *>(raw_diagonal_ptr);
					const auto &value = raw_diagonal.currentValue();
					if (value.second) {
						return const_Hypertrie<tr>(depth, context, {value.first.hash().hash(), value.fist.node()});
					} else {
						return const_Hypertrie<tr>(depth, nullptr, {value.first.hash().hash(), new raw::CompressedNode<depth, tri>(*value.fist.compressed_node())});
					}
				} else {
					assert(false);
					return {};
				}
					},
					// currentScalar
					[] (void const *raw_diagonal_ptr) -> value_type {
				const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_t *>(raw_diagonal_ptr);
				if constexpr (diag_depth < depth) {
					return raw_diagonal.currentValue();
				} else {
					assert(false);
					return {};
				}
					},
				    // find
					[](void *raw_diagonal_ptr, key_part_type key_part) -> bool {
				auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_t *>(raw_diagonal_ptr);
				return raw_diagonal.find(key_part);
				    },
				   // inc
					   [](void *raw_diagonal_ptr) {
				auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_t *>(raw_diagonal_ptr);
				++raw_diagonal;
					   },
					   // ended
					   [](const void *raw_diagonal_ptr) -> bool {
				const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_t *>(raw_diagonal_ptr);
				return raw_diagonal.ended();
					},
					// size
					[](const void *raw_diagonal_ptr) -> size_t {
				const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_t *>(raw_diagonal_ptr);
				return raw_diagonal.size();
					}
					);
		}

		inline static const std::vector<std::vector<std::vector<RawMethods>>> raw_method_cache = []() {
			std::vector<std::vector<RawMethods>> raw_methods(2);
			raw_methods[0] = std::vector<std::vector<RawMethods>>(hypertrie_depth_limit);
			raw_methods[1] = std::vector<std::vector<RawMethods>>(hypertrie_depth_limit);
			for (size_t depth : iter::range(1UL, hypertrie_depth_limit))
				for (size_t diag_depth : iter::range(1UL, depth + 1))
					for (int compression : iter::range(2))
						compiled_switch<hypertrie_depth_limit, 1>::switch_void(
								depth,
								[&](auto depth_arg) {//
									compiled_switch<hypertrie_depth_limit, 1>::switch_void(
											diag_depth,
											[&](auto diag_depth_arg) {
												compiled_switch<1, 0>::switch_void(
														compression,
														[&](auto compression_arg) {
															raw_methods[compression_arg][depth_arg - 1].push_back(generateRawMethods<diag_depth_arg, depth_arg, compression_arg>());
														});
											});
								});
			return raw_methods;
		}();

		static RawMethods const &getRawMethods(size_t depth, size_t diag_depth, bool compression) {
			return raw_method_cache[compression][depth - 1][diag_depth-1];
		};

	protected:
		RawMethods const * const raw_methods = nullptr;
		void* raw_hash_diagonal;

	public:

		HashDiagonal & begin() {
			raw_methods->begin(raw_hash_diagonal);
			return *this;
		}

	};
}

#endif//HYPERTRIE_HASHDIAGONAL_HPP
