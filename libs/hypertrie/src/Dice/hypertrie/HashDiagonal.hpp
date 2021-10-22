#ifndef HYPERTRIE_HASHDIAGONAL_HPP
#define HYPERTRIE_HASHDIAGONAL_HPP

#include "Dice/hypertrie/HypertrieContext.hpp"
#include "Dice/hypertrie/Hypertrie_predeclare.hpp"
#include "Dice/hypertrie/internal/raw/iteration/RawHashDiagonal.hpp"
#include "Dice/hypertrie/internal/util/SwitchTemplateFunctions.hpp"

namespace Dice::hypertrie {

	template<HypertrieTrait tr_t>
	class HashDiagonal {
	public:
		using tr = tr_t;
		using tri = internal::raw::template Hypertrie_core_t<tr>;
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;
		using KeyPositions_t = internal::raw::RawKeyPositions<hypertrie_max_depth>;

	private:
		template<size_t diag_depth, size_t depth, template<size_t, typename> typename node_type>
		using RawHashDiagonal_t = typename internal::raw::template RawHashDiagonal<diag_depth, depth, node_type, tri, hypertrie_max_depth>;
		using max_sized_RawHashDiagonal_t = std::conditional_t<(sizeof(RawHashDiagonal_t<1, hypertrie_max_depth, internal::raw::FullNode>) > sizeof(RawHashDiagonal_t<1, hypertrie_max_depth, internal::raw::SingleEntryNode>)),
															   RawHashDiagonal_t<1, hypertrie_max_depth, internal::raw::FullNode>,
															   RawHashDiagonal_t<1, hypertrie_max_depth, internal::raw::SingleEntryNode>>;

	protected:
		struct RawMethods {
			void (*construct_)(const_Hypertrie<tr> const &, KeyPositions_t const &, void *) noexcept;

			void (*destruct_)(void *) noexcept;

			void (*begin_)(void *) noexcept;

			key_part_type (*current_key_part_)(void const *) noexcept;

			const_Hypertrie<tr> (*current_hypertrie_)(void const *, HypertrieContext<tr> *) noexcept;

			value_type (*current_scalar_)(void const *) noexcept;

			bool (*find_)(void *, key_part_type) noexcept;

			void (*inc_)(void *) noexcept;

			bool (*ended_)(void const *) noexcept;

			size_t (*size_)(void const *) noexcept;
			RawMethods() = default;
			RawMethods(void (*construct)(const_Hypertrie<tr> const &, KeyPositions_t const &, void *) noexcept,
					   void (*destruct)(void *) noexcept,
					   void (*begin)(void *) noexcept,
					   key_part_type (*currentKeyPart)(const void *) noexcept,
					   const_Hypertrie<tr> (*currentHypertrie)(const void *, HypertrieContext<tr> *) noexcept,
					   value_type (*currentScalar)(const void *) noexcept,
					   bool (*find)(void *, key_part_type) noexcept,
					   void (*inc)(void *) noexcept,
					   bool (*ended)(const void *) noexcept,
					   size_t (*size)(const void *) noexcept) : construct_(construct), destruct_(destruct), begin_(begin), current_key_part_(currentKeyPart), current_hypertrie_(currentHypertrie), current_scalar_(currentScalar), find_(find), inc_(inc), ended_(ended), size_(size) {}
		};

		template<size_t diag_depth, size_t depth, template<size_t, typename> typename node_type, bool with_tri_alloc = false>
		inline static RawMethods generateRawMethods() noexcept {
			using namespace ::Dice::hypertrie::internal::raw;
			using namespace ::Dice::hypertrie::internal::util;
			[[maybe_unused]] constexpr static const size_t result_depth = depth - diag_depth;

			static constexpr bool is_fn = std::is_same_v<node_type<depth, tri>, FullNode<depth, tri>>;
			static_assert(not(is_fn and not with_tri_alloc));

			using used_tri = std::conditional_t<(with_tri_alloc), tri, tri_with_stl_alloc<tri>>;
			using RawDiagonalHash_tt = RawHashDiagonal<diag_depth, depth, node_type, used_tri, hypertrie_max_depth>;
			return RawMethods(
					// construct
					[](const_Hypertrie<tr> const &hypertrie, KeyPositions_t const &diagonal_poss, void *raw_diagonal_ptr) noexcept {
						auto &raw_diag_poss = unsafe_cast<RawKeyPositions<depth> const>(diagonal_poss);
						if constexpr (is_fn) {
							auto &nodec = unsafe_cast<FNContainer<depth, used_tri> const>(hypertrie.template node_container<depth>());
							std::construct_at(reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr),
											  nodec,
											  raw_diag_poss,
											  hypertrie.context()->raw_context());
						} else {
							auto &nodec = unsafe_cast<SENContainer<depth, used_tri> const>(hypertrie.template node_container<depth>());
							std::construct_at(reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr),
											  nodec,
											  raw_diag_poss);
						}
					},
					// destruct
					[](void *raw_diagonal_ptr) noexcept {
						std::destroy_at(reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr));
					},
					// begin
					[](void *raw_diagonal_ptr) noexcept {
						auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr);
						raw_diagonal.begin();
					},
					// currentKeyPart
					[](void const *raw_diagonal_ptr) noexcept -> key_part_type {
						const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
						return raw_diagonal.current_key_part();
					},
					// currentHypertrie
					[](void const *raw_diagonal_ptr, HypertrieContext<tr> *context) noexcept -> const_Hypertrie<tr> {
						if constexpr (diag_depth < depth) {
							const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
							const auto &value = raw_diagonal.current_value();
							if (value.uses_tri_alloc())
								return {result_depth,
										context, true,
										value.get_with_tri_alloc()};
							else
								return {result_depth,
										nullptr, true, value.get_with_stl_alloc()};
						} else {
							assert(false);
							__builtin_unreachable();
						}
					},
					// currentScalar
					[](void const *raw_diagonal_ptr) noexcept -> value_type {
						const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
						if constexpr (diag_depth == depth) {
							return raw_diagonal.current_value();
						} else {
							assert(false);
							__builtin_unreachable();
						}
					},
					// find
					[](void *raw_diagonal_ptr, key_part_type key_part) noexcept -> bool {
						auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr);
						return raw_diagonal.find(key_part);
					},
					// inc
					[](void *raw_diagonal_ptr) noexcept {
						auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr);
						++raw_diagonal;
					},
					// ended
					[](const void *raw_diagonal_ptr) noexcept -> bool {
						const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
						return raw_diagonal.ended();
					},
					// size
					[](const void *raw_diagonal_ptr) noexcept -> size_t {
						const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
						return raw_diagonal.size();
					});
		}

		using RawMethosCache = std::vector<std::vector<std::tuple<RawMethods, RawMethods, RawMethods>>>;
		inline static const RawMethosCache raw_method_cache = []() noexcept {
			using namespace ::Dice::hypertrie::internal::raw;
			using namespace ::Dice::hypertrie::internal::util;
			RawMethosCache raw_methods;
			// depth = 1 ... hypertrie_max_depth
			// diag_depth = 1 ... depth
			// tuple = <FullNode(tri alloc), SingleEntryNode(tri alloc), SingleEntryNode(stl alloc)>
			raw_methods.resize(hypertrie_max_depth);
			for (size_t depth : iter::range(1UL, hypertrie_max_depth + 1)) {
				auto &raw_methods_with_depth = raw_methods[depth - 1];
				raw_methods_with_depth.resize(depth);
				for (size_t diag_depth : iter::range(1UL, depth + 1)) {
					raw_methods_with_depth[diag_depth - 1] =
							switch_cases<1UL, hypertrie_max_depth + 1>(
									depth,
									[&](auto depth_arg) -> std::tuple<RawMethods, RawMethods, RawMethods> {//
										return switch_cases<1UL, depth_arg + 1>(
												diag_depth,
												[&](auto diag_depth_arg) -> std::tuple<RawMethods, RawMethods, RawMethods> {
													return std::make_tuple(
															generateRawMethods<diag_depth_arg, depth_arg, FullNode, true>(),
															generateRawMethods<diag_depth_arg, depth_arg, SingleEntryNode, true>(),
															generateRawMethods<diag_depth_arg, depth_arg, SingleEntryNode, false>());
												},
												[&]() -> std::tuple<RawMethods, RawMethods, RawMethods> { assert(false); __builtin_unreachable(); });
									},
									[&]() -> std::tuple<RawMethods, RawMethods, RawMethods> { assert(false); __builtin_unreachable(); });
				}
			}
			return raw_methods;
		}();

		static RawMethods const &getRawMethods(size_t depth, size_t diag_depth, bool full_node, bool tri_alloc = true) noexcept {
			auto &resolve_depth_and_diag_depth = raw_method_cache[depth - 1][diag_depth - 1];
			assert(not(full_node and not tri_alloc));
			if (full_node)
				return std::get<0>(resolve_depth_and_diag_depth);
			else if (tri_alloc)// and not full node
				return std::get<1>(resolve_depth_and_diag_depth);
			else
				return std::get<2>(resolve_depth_and_diag_depth);
		};

	protected:
		RawMethods const *raw_methods = nullptr;
		std::array<std::byte, sizeof(max_sized_RawHashDiagonal_t)> raw_hash_diagonal;
		HypertrieContext<tr> *context_;// already imprinted into Hypertrie Context

	public:
		HashDiagonal(const const_Hypertrie<tr> &hypertrie, const KeyPositions_t &diag_poss) noexcept
			: raw_methods(&getRawMethods(hypertrie.depth(),
										 diag_poss.count(),
										 hypertrie.size() > 1,
										 hypertrie.size() > 1 and not hypertrie.contextless())),
			  context_(hypertrie.context()) {
			raw_methods->construct_(hypertrie, diag_poss, &raw_hash_diagonal);
		}

		HashDiagonal(HashDiagonal &&other) noexcept : raw_methods(other.raw_methods),
													  raw_hash_diagonal(other.raw_hash_diagonal),
													  context_(other.context_) {
			other.raw_hash_diagonal = nullptr;
			other.context_ = nullptr;
		}


		HashDiagonal &operator=(HashDiagonal &&other) noexcept {
			if (raw_hash_diagonal != nullptr) {
				raw_methods->destruct_(raw_hash_diagonal);
				raw_hash_diagonal = nullptr;
			}
			this->raw_methods = other.raw_methods;
			this->raw_hash_diagonal = other.raw_hash_diagonal;
			this->context_ = other.context_;
			other.raw_hash_diagonal = nullptr;
			other.context_ = nullptr;
			return *this;
		}

		~HashDiagonal() noexcept {
			if (raw_methods != nullptr) {
				raw_methods->destruct_(&raw_hash_diagonal);
			}
			raw_methods = nullptr;
		}


		HashDiagonal &begin() {
			raw_methods->begin_(&raw_hash_diagonal);
			return *this;
		}

		[[nodiscard]] bool end() const {
			return false;
		}

		[[nodiscard]] key_part_type current_key_part() const {
			return raw_methods->current_key_part_(&raw_hash_diagonal);
		}

		[[nodiscard]] const_Hypertrie<tr> current_hypertrie() const {
			return raw_methods->current_hypertrie_(&raw_hash_diagonal, context_);
		}


		[[nodiscard]] value_type current_scalar() const {
			return raw_methods->current_scalar_(&raw_hash_diagonal);
		}

		[[nodiscard]] bool find(key_part_type key_part) {
			return raw_methods->find_(&raw_hash_diagonal, key_part);
		}

		HashDiagonal &operator++() noexcept {
			raw_methods->inc_(&raw_hash_diagonal);
			return *this;
		}

		HashDiagonal operator++(int) const {
			auto old = *this;
			++(*this);
			return old;
		}

		[[nodiscard]] explicit operator bool() const noexcept {
			return not raw_methods->ended_(&raw_hash_diagonal);
		}

		[[nodiscard]] [[nodiscard]] bool ended() const noexcept {
			return raw_methods->ended_(&raw_hash_diagonal);
		}

		bool operator==(bool other) const noexcept {
			return bool(*this) == other;
		}

		bool operator<(const HashDiagonal &other) const noexcept {
			return this->size() < other.size();
		}

		[[nodiscard]] size_t size() const noexcept {
			return raw_methods->size_(&raw_hash_diagonal);
		}
	};
}// namespace Dice::hypertrie

#endif//HYPERTRIE_HASHDIAGONAL_HPP
