#ifndef HYPERTRIE_HASHDIAGONAL_HPP
#define HYPERTRIE_HASHDIAGONAL_HPP

#include "Dice/hypertrie/internal/raw/iterator/Diagonal.hpp"
#include "Dice/hypertrie/internal/HypertrieContext.hpp"
#include "Dice/hypertrie/internal/old_hypertrie/BoolHypertrie.hpp"
#include "Dice/hypertrie/internal/Hypertrie_predeclare.hpp"

namespace hypertrie {

	template<HypertrieTrait tr_t = default_bool_Hypertrie_t>
	class HashDiagonal {
		using tr = tr_t;
		using tri = internal::raw::Hypertrie_internal_t<tr>;
		typedef typename internal::raw::RawNodeContainer NodeContainer;
		using KeyPositions = typename tr::KeyPositions;
		template <size_t depth>
		using RawKeyPositions = typename  tri::template DiagonalPositions<depth>;
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		typedef typename internal::raw::NodeCompression NodeCompression;
		template<pos_type depth, pos_type diag_depth>
		using RawHashDiagonal =  typename hypertrie::internal::interface::rawboolhypertrie<typename tri::key_part_type, tri::template map_type, tri::template set_type>::template RawHashDiagonal<diag_depth, depth>;
		template<pos_type depth>
		using RawBoolHypertrie = typename hypertrie::internal::interface::rawboolhypertrie<typename tri::key_part_type, tri::template map_type, tri::template set_type>::template RawBoolHypertrie<depth>;
	protected:
		using Key = typename tr::Key;

		struct RawMethods {
			void *(*construct)(const const_Hypertrie<tr> &hypertrie, const KeyPositions &diagonal_poss);

			void (*destruct)(void *);

			void (*begin)(void *);

			key_part_type (*currentKeyPart)(void const *);

			const_Hypertrie<tr> (*currentHypertrie)(void const *);

			value_type (*currentScalar)(void const *);

			bool (*find)(void *, key_part_type);

			void (*inc)(void *);

			bool (*ended)(void const *);

			size_t (*size)(void const *);
			RawMethods(void *(*construct)(const const_Hypertrie<tr> &, const KeyPositions &), void (*destruct)(void *), void (*begin)(void *), key_part_type (*currentKeyPart)(const void *), const_Hypertrie<tr> (*currentHypertrie)(const void *), value_type (*currentScalar)(const void *), bool (*find)(void *, key_part_type), void (*inc)(void *), bool (*ended)(const void *), size_t (*size)(const void *)) : construct(construct), destruct(destruct), begin(begin), currentKeyPart(currentKeyPart), currentHypertrie(currentHypertrie), currentScalar(currentScalar), find(find), inc(inc), ended(ended), size(size) {}
		};

		template <size_t diag_depth, size_t depth, NodeCompression compression>
		inline static RawMethods generateRawMethods() {
			[[maybe_unused]] constexpr static const size_t result_depth = depth - diag_depth;
			using RawDiagonalHash_t = RawHashDiagonal<diag_depth, depth>;
			return RawMethods(
					// construct
					[](const const_Hypertrie<tr> &hypertrie, const KeyPositions &diagonal_poss) -> void * {
						const auto &raw_boolhypertrie = *(static_cast<RawBoolHypertrie<depth> const *>(hypertrie.rawNode()));
						if constexpr (depth == diag_depth) {
							return new RawDiagonalHash_t(raw_boolhypertrie);
						} else {
							return new RawDiagonalHash_t(raw_boolhypertrie, diagonal_poss);
						}
					},
					// destruct
					[](void *raw_diagonal_ptr) {
						delete reinterpret_cast<RawDiagonalHash_t *>(raw_diagonal_ptr);
					},
					// begin
					&RawDiagonalHash_t::init,
					// currentKeyPart
					&RawDiagonalHash_t::currentKeyPart,
					// currentHypertrie
					[](void const *raw_diagonal_ptr) -> const_Hypertrie<tr> {
						if constexpr (diag_depth < depth) {
							const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_t *>(raw_diagonal_ptr);

							std::shared_ptr<RawBoolHypertrie<depth - diag_depth>> &value = *RawDiagonalHash_t::currentValue(raw_diagonal_ptr);

							return const_Hypertrie<tr>{result_depth, value};
						} else {
							assert(false);
						}
					},
					// currentScalar
					[]([[maybe_unused]] void const *raw_diagonal_ptr) -> value_type {
						return value_type(1);
					},
					// find
					&RawDiagonalHash_t::contains,
					// inc
					&RawDiagonalHash_t::inc,
					// ended
					&RawDiagonalHash_t::empty,
					// size
					&RawDiagonalHash_t::size
					);
		}

		inline static const std::vector<std::vector<std::vector<RawMethods>>> raw_method_cache = []() {
			using namespace internal;
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
		RawMethods const * raw_methods = nullptr;
		void* raw_hash_diagonal;

	public:
		HashDiagonal(const const_Hypertrie<tr> &hypertrie, const KeyPositions &diag_poss)
			: raw_methods(&getRawMethods(hypertrie.depth(), diag_poss.size(), hypertrie.size() == 1)),
			  raw_hash_diagonal(raw_methods->construct(hypertrie, diag_poss)) {}

		HashDiagonal(HashDiagonal &&other)
			: raw_methods(other.raw_methods),
			  raw_hash_diagonal(other.raw_hash_diagonal) {
			other.raw_hash_diagonal = nullptr;
		}


		HashDiagonal &operator=(HashDiagonal &&other) {
			if (raw_hash_diagonal != nullptr) {
				raw_methods->destruct(raw_hash_diagonal);
				raw_hash_diagonal = nullptr;
			}
			this->raw_methods = other.raw_methods;
			this->raw_hash_diagonal = other.raw_hash_diagonal;
			other.raw_hash_diagonal = nullptr;
			return *this;
		}

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
			return raw_methods->currentHypertrie(raw_hash_diagonal);
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

		bool operator<(const HashDiagonal &other) const {
			return this->size() < other.size();
		}

		size_t size() const {
			return raw_methods->size(raw_hash_diagonal);
		}

	};
}

#endif//HYPERTRIE_HASHDIAGONAL_HPP
