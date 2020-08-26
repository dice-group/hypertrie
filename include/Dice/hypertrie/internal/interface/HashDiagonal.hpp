#ifndef HYPERTRIE_HASHDIAGONAL_HPP
#define HYPERTRIE_HASHDIAGONAL_HPP

#include "Dice/hypertrie/internal/raw/iterator/Diagonal.hpp"
#include "Dice/hypertrie/internal/interface/HypertrieContext.hpp"
#include "Dice/hypertrie/internal/interface/Hypertrie_predeclare.hpp"

namespace hypertrie::internal::node_based {

	template<HypertrieTrait tr_t = default_bool_Hypertrie_t>
	class HashDiagonal {
		using tr = tr_t;
		using tri = raw::Hypertrie_internal_t<tr>;
		using NodeContainer = typename raw::RawNodeContainer;
		using KeyPositions = typename tr::KeyPositions;
		template <size_t depth>
		using RawKeyPositions = typename  tri::template DiagonalPositions<depth>;
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using NodeCompression = typename raw::NodeCompression;
		template <size_t diag_depth, size_t depth, NodeCompression compression>
		using RawHashDiagonal = typename raw::template HashDiagonal<diag_depth, depth, compression, tri>;
	protected:
		using Key = typename tr::Key;

		struct RawMethods {
			void *(*construct)(const const_Hypertrie<tr> &hypertrie, const KeyPositions &diagonal_poss);

			void (*destruct)(void *);

			void (*begin)(void *);

			key_part_type (*currentKeyPart)(void const *);

			const_Hypertrie<tr> (*currentHypertrie)(void const *, HypertrieContext<tr> *);

			value_type (*currentScalar)(void const *);

			bool (*find)(void *, key_part_type);

			void (*inc)(void *);

			bool (*ended)(void const *);

			size_t (*size)(void const *);
			RawMethods(void *(*construct)(const const_Hypertrie<tr> &, const KeyPositions &), void (*destruct)(void *), void (*begin)(void *), key_part_type (*currentKeyPart)(const void *), const_Hypertrie<tr> (*currentHypertrie)(const void *, HypertrieContext<tr> *), value_type (*currentScalar)(const void *), bool (*find)(void *, key_part_type), void (*inc)(void *), bool (*ended)(const void *), size_t (*size)(const void *)) : construct(construct), destruct(destruct), begin(begin), currentKeyPart(currentKeyPart), currentHypertrie(currentHypertrie), currentScalar(currentScalar), find(find), inc(inc), ended(ended), size(size) {}
		};

		template <size_t diag_depth, size_t depth, NodeCompression compression>
		inline static RawMethods generateRawMethods() {
			[[maybe_unused]] constexpr static const size_t result_depth = depth - diag_depth;
			using RawDiagonalHash_t = RawHashDiagonal<diag_depth, depth, compression>;
			return RawMethods(
					// construct
					[](const const_Hypertrie<tr> &hypertrie, const KeyPositions &diagonal_poss) -> void * {
				using NodecType = typename raw::template SpecificNodeContainer<depth, compression, tri>;

				RawKeyPositions<depth> raw_diag_poss;
				for (auto pos : diagonal_poss)
					raw_diag_poss[pos] = true;
				auto &nodec = *const_cast<NodecType *>(reinterpret_cast<const NodecType *>(hypertrie.rawNodeContainer()));
				if constexpr (bool(compression))
				return new RawHashDiagonal<diag_depth, depth, NodeCompression::compressed>(
						nodec,
						raw_diag_poss);
				else
					return new RawHashDiagonal<diag_depth, depth, NodeCompression::uncompressed>(
							nodec,
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
					if (value.is_managed) {
						return const_Hypertrie<tr>(result_depth, context, {value.nodec.hash().hash(), value.nodec.node()});
					} else {
						return const_Hypertrie<tr>(result_depth, nullptr, {value.nodec.hash().hash(), new raw::CompressedNode<result_depth, tri>(*value.nodec.compressed_node())});
					}
				} else {
					assert(false);
					// return {};
				}
					},
					// currentScalar
					[] (void const *raw_diagonal_ptr) -> value_type {
				const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_t *>(raw_diagonal_ptr);
				if constexpr (diag_depth == depth) {
					return raw_diagonal.currentValue();
				} else {
					assert(false);
//					return {};
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
		  std::vector<std::vector<std::vector<RawMethods>>> raw_methods(2);
			raw_methods[0] = std::vector<std::vector<RawMethods>>(hypertrie_depth_limit);
			raw_methods[1] = std::vector<std::vector<RawMethods>>(hypertrie_depth_limit);
			for (size_t depth : iter::range(1UL, hypertrie_depth_limit))
				for (size_t diag_depth : iter::range(1UL, depth + 1))
					for (int compression : iter::range(2))
						compiled_switch<hypertrie_depth_limit, 1>::switch_void(
								depth,
								[&](auto depth_arg) {//
									compiled_switch<depth_arg + 1, 1>::switch_void(
											diag_depth,
											[&](auto diag_depth_arg) {
												compiled_switch<2, 0>::switch_void(
														compression,
														[&](auto compression_arg) {
															raw_methods[compression_arg][depth_arg - 1].push_back(generateRawMethods<diag_depth_arg, depth_arg, static_cast<NodeCompression>(bool(compression_arg))>());
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
		HypertrieContext<tr> *context_;

	public:
		HashDiagonal(const const_Hypertrie<tr> &hypertrie, const KeyPositions &diag_poss)
			: raw_methods(&getRawMethods(hypertrie.depth(), diag_poss.size(), hypertrie.size() == 1)),
			  raw_hash_diagonal(raw_methods->construct(hypertrie, diag_poss)), context_(hypertrie.context()) {}

		~HashDiagonal() {
			if (raw_hash_diagonal != nullptr){
				raw_methods->destruct(raw_hash_diagonal);
				raw_hash_diagonal = nullptr;
			}
		}

		HashDiagonal & begin() {
			raw_methods->begin(raw_hash_diagonal);
			return *this;
		}

		bool end() const {
			return false;
		}

		key_part_type currentKeyPart() const {
			return raw_methods->currentKeyPart(raw_hash_diagonal);
		}

		const_Hypertrie<tr> currentHypertrie() const{
			return raw_methods->currentHypertrie(raw_hash_diagonal,context_);
		}


		value_type currentScalar() const {
			return raw_methods->currentScalar(raw_hash_diagonal);
		}

		bool find(key_part_type key_part){
			return raw_methods->find(raw_hash_diagonal, key_part);
		}

		HashDiagonal &operator++() {
			raw_methods->inc(raw_hash_diagonal);
			return *this;
		}

		explicit operator bool() const {
			return not raw_methods->ended(raw_hash_diagonal);
		}

		bool ended() const {
			return raw_methods->ended(raw_hash_diagonal);
		}

		bool operator==(bool other) const {
			return bool(*this) == other;
		}

		size_t size() const {
			raw_methods->size(raw_hash_diagonal);
		}

	};
}

#endif//HYPERTRIE_HASHDIAGONAL_HPP
