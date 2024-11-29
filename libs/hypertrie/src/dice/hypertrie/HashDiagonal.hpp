#ifndef HYPERTRIE_HASHDIAGONAL_HPP
#define HYPERTRIE_HASHDIAGONAL_HPP

#include "dice/hypertrie/HypertrieContext.hpp"
#include "dice/hypertrie/Hypertrie_predeclare.hpp"
#include "dice/hypertrie/internal/raw/iteration/RawHashDiagonal.hpp"
#include "dice/template-library/switch_cases.hpp"

namespace dice::hypertrie {

	/**
	 * A Diagonal implementation for hypertries which use a map_type in htt_t that is similar to std::unordered_map (typically a hash map).
	 * The diagonal provides access to non-zero slices where given position (diag_poss) have the same key_part.
	 * <p>Example: Consider diag_poss = [0,3,4] for a depth 5 hypertrie T. T has a diagonal with key_part 11 if the slice T[11,:,:,11,11] is not empty.</p>
	 * We say "full diagonal", if diag_poss contains all possible positions (|diag_poss| = depth of hypertrie).
	 * <p>HashDiagonal has two modes of operation for a given hypertrie and diagonal positions:</p>
	 * <p>(1) Iterate through the non-zero diagonal slices.</p>
	 * <p>(2) Probe for given key_parts, if a non-zero diagonal slice exists.<p>
	 *
	 * @tparam htt_t
	 * @tparam allocator_type
	 */
	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class HashDiagonal {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		using RawKeyPositions_t = internal::raw::RawKeyPositions<hypertrie_max_depth>;

	private:
		template<size_t diag_depth, size_t depth, template<size_t, typename, typename> typename node_type>
		using RawHashDiagonal_t = typename internal::raw::template RawHashDiagonal<diag_depth, depth, node_type, htt_t, allocator_type, hypertrie_max_depth>;
		using max_sized_RawHashDiagonal_t = std::conditional_t<(sizeof(RawHashDiagonal_t<1, hypertrie_max_depth, internal::raw::FullNode>) > sizeof(RawHashDiagonal_t<1, hypertrie_max_depth, internal::raw::SingleEntryNode>)),
															   RawHashDiagonal_t<1, hypertrie_max_depth, internal::raw::FullNode>,
															   RawHashDiagonal_t<1, hypertrie_max_depth, internal::raw::SingleEntryNode>>;

	protected:
		struct RawMethods {
			void (*construct)(const_Hypertrie<htt_t, allocator_type> const &, RawKeyPositions_t const &, void *) noexcept;

			void (*copy)(void const *, void *) noexcept;

			void (*move)(void *, void *) noexcept;

			void (*destruct)(void *) noexcept;

			void (*begin)(void *) noexcept;

			key_part_type (*current_key_part)(void const *) noexcept;

			const_Hypertrie<htt_t, allocator_type> (*current_hypertrie)(void const *, HypertrieContext_ptr<htt_t, allocator_type>) noexcept;

			value_type (*current_scalar)(void const *) noexcept;

			bool (*find)(void *, key_part_type) noexcept;

			void (*inc)(void *) noexcept;

			bool (*ended)(void const *) noexcept;

			size_t (*size)(void const *) noexcept;

			void (*update_sen_cache_ptr)(void *) noexcept;
		};

		template<size_t diag_depth, size_t depth, template<size_t, typename, typename> typename node_type, bool uses_provided_alloc = false>
		inline static RawMethods generate_raw_methods() noexcept {
			using namespace ::dice::hypertrie::internal::raw;
			using namespace ::dice::hypertrie::internal::util;
			[[maybe_unused]] constexpr static const size_t result_depth = depth - diag_depth;

			static constexpr bool is_fn = is_FullNode_v<node_type>;
			static_assert(not(is_fn and not uses_provided_alloc));

			using used_alloc = std::conditional_t<(uses_provided_alloc), allocator_type, std::allocator<std::byte>>;
			using RawDiagonalHash_tt = RawHashDiagonal<diag_depth, depth, node_type, htt_t, used_alloc, hypertrie_max_depth>;
			return RawMethods{
					.construct =
							[](const_Hypertrie<htt_t, allocator_type> const &hypertrie, RawKeyPositions_t const &diagonal_poss, void *raw_diagonal_ptr) noexcept {
								auto &raw_diag_poss = unsafe_cast<RawKeyPositions<depth> const>(diagonal_poss);
								if constexpr (is_fn) {
									FNContainer<depth, htt_t, used_alloc> nodec{hypertrie.template node_container<depth>()};
									// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
									std::construct_at(reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr),
													  nodec,
													  raw_diag_poss,
													  hypertrie.context()->raw_context());
								} else {
									SENContainer<depth, htt_t, used_alloc> nodec = *hypertrie.raw_node_container();
									// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
									std::construct_at(reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr),
													  nodec,
													  raw_diag_poss);
								}
							},
					.copy =
							[](void const *old, void *new_) noexcept {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								std::construct_at(reinterpret_cast<RawDiagonalHash_tt *>(new_), *reinterpret_cast<RawDiagonalHash_tt const *>(old));
							},
					.move =
							[](void *old, void *new_) noexcept {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								std::construct_at(reinterpret_cast<RawDiagonalHash_tt *>(new_), std::move(*reinterpret_cast<RawDiagonalHash_tt *>(old)));
							},
					.destruct =
							[](void *raw_diagonal_ptr) noexcept {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								std::destroy_at(reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr));
							},
					.begin =
							[](void *raw_diagonal_ptr) noexcept {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr);
								raw_diagonal.begin();
							},
					.current_key_part =
							[](void const *raw_diagonal_ptr) noexcept -> key_part_type {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
								return raw_diagonal.current_key_part();
							},
					.current_hypertrie =
							[](void const *raw_diagonal_ptr, HypertrieContext_ptr<htt_t, allocator_type> context) noexcept -> const_Hypertrie<htt_t, allocator_type> {
								if constexpr (diag_depth < depth) {
									// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
									const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
									const auto &value = raw_diagonal.current_value();
									if (value.uses_provided_alloc()) {
										return {result_depth,
												context, true,
												value.get_raw_nodec()};
									}
									return {result_depth,
											nullptr, true, value.get_stl_alloc_sen()};
								} else {
									assert(false);
									__builtin_unreachable();
								}
							},
					.current_scalar =
							[](void const *raw_diagonal_ptr) noexcept -> value_type {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
								if constexpr (diag_depth == depth) {
									return raw_diagonal.current_value();
								} else {
									assert(false);
									__builtin_unreachable();
								}
							},
					// find
					.find =
							[](void *raw_diagonal_ptr, key_part_type key_part) noexcept -> bool {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr);
								return raw_diagonal.find(key_part);
							},
					.inc =
							[](void *raw_diagonal_ptr) noexcept {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr);
								++raw_diagonal;
							},
					.ended =
							[](const void *raw_diagonal_ptr) noexcept -> bool {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
								return raw_diagonal.ended();
							},
					.size =
							[](const void *raw_diagonal_ptr) noexcept -> size_t {
								// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
								const auto &raw_diagonal = *reinterpret_cast<const RawDiagonalHash_tt *>(raw_diagonal_ptr);
								return raw_diagonal.size();
							},
					.update_sen_cache_ptr = [](void *raw_diagonal_ptr) noexcept {
						// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
					  auto &raw_diagonal = *reinterpret_cast<RawDiagonalHash_tt *>(raw_diagonal_ptr);
						raw_diagonal.update_sen_cache_ptr(); }};
		}

		using RawMethosCache = std::vector<std::vector<std::tuple<RawMethods, RawMethods, RawMethods>>>;
		inline static const RawMethosCache raw_method_cache = []() noexcept {
			using namespace ::dice::hypertrie::internal::raw;
			using namespace ::dice::hypertrie::internal::util;
			RawMethosCache raw_methods;
			// depth = 1 ... hypertrie_max_depth
			// diag_depth = 1 ... depth
			// tuple = <FullNode<..., allocator_type>, SingleEntryNode<..., allocator_type>, SingleEntryNode<..., std::allocator<std::byte>>>
			raw_methods.resize(hypertrie_max_depth);
			for (size_t depth = 1; depth < hypertrie_max_depth + 1; ++depth) {
				auto &raw_methods_with_depth = raw_methods[depth - 1];
				raw_methods_with_depth.resize(depth);
				for (size_t diag_depth = 1; diag_depth < depth + 1; ++diag_depth) {
					raw_methods_with_depth[diag_depth - 1] =
							template_library::switch_cases<1UL, hypertrie_max_depth + 1>(
									depth,
									[&](auto depth_arg) -> std::tuple<RawMethods, RawMethods, RawMethods> {//
										return template_library::switch_cases<1UL, depth_arg + 1>(
												diag_depth,
												[&](auto diag_depth_arg) -> std::tuple<RawMethods, RawMethods, RawMethods> {
													return std::make_tuple(
															generate_raw_methods<diag_depth_arg, depth_arg, FullNode, true>(),
															generate_raw_methods<diag_depth_arg, depth_arg, SingleEntryNode, true>(),
															generate_raw_methods<diag_depth_arg, depth_arg, SingleEntryNode, false>());
												},
												[&]() -> std::tuple<RawMethods, RawMethods, RawMethods> { assert(false); __builtin_unreachable(); });
									},
									[&]() -> std::tuple<RawMethods, RawMethods, RawMethods> { assert(false); __builtin_unreachable(); });
				}
			}
			return raw_methods;
		}();

		static RawMethods const &get_raw_methods(size_t depth, size_t diag_depth, bool full_node, bool tri_alloc = true) noexcept {
			auto &resolve_depth_and_diag_depth = raw_method_cache[depth - 1][diag_depth - 1];
			assert(not(full_node and not tri_alloc));
			if (full_node) {
				return std::get<0>(resolve_depth_and_diag_depth);
			}
			if (tri_alloc) {// and not full node
				return std::get<1>(resolve_depth_and_diag_depth);
			}
			return std::get<2>(resolve_depth_and_diag_depth);
		};

		RawMethods const *raw_methods = nullptr;
		std::array<std::byte, sizeof(max_sized_RawHashDiagonal_t)> raw_hash_diagonal;
		HypertrieContext_ptr<htt_t, allocator_type> context_;// already imprinted into Hypertrie Context

	public:
		/**
		 * Constructor for a HashDiagonal.
		 * @param hypertrie The hypertrie from that the diagonal is extracted.
		 * @param diag_poss The Key position of the diagonal.
		 */
		HashDiagonal(const_Hypertrie<htt_t, allocator_type> const &hypertrie, const RawKeyPositions_t &diag_poss) noexcept
			: raw_methods(&get_raw_methods(hypertrie.depth(),
										   diag_poss.count(),
										   hypertrie.size() > 1,
										   not hypertrie.contextless())),
			  context_(hypertrie.context()) {
			raw_methods->construct(hypertrie, diag_poss, &raw_hash_diagonal);
		}

		HashDiagonal(HashDiagonal const &other) noexcept
			: raw_methods(other.raw_methods),
			  context_(other.context_) {
			raw_methods->copy(&other.raw_hash_diagonal, &raw_hash_diagonal);
			raw_methods->update_sen_cache_ptr(&raw_hash_diagonal);
		}

		HashDiagonal &operator=(HashDiagonal const &other) noexcept {
			if (this == &other)
				return *this;
			if (raw_methods != nullptr) {
				raw_methods->destruct(&raw_hash_diagonal);
			}
			raw_methods = other.raw_methods;
			raw_methods->copy(&other.raw_hash_diagonal, &raw_hash_diagonal);
			context_ = other.context_;
			raw_methods->update_sen_cache_ptr(&raw_hash_diagonal);

			return *this;
		}

		HashDiagonal(HashDiagonal &&other) noexcept
			: raw_methods(other.raw_methods),
			  context_(other.context_) {
			assert(this != &other);

			raw_methods->move(&other.raw_hash_diagonal, &raw_hash_diagonal);
			raw_methods->update_sen_cache_ptr(&raw_hash_diagonal);

			other.raw_methods = nullptr;
			other.raw_hash_diagonal = {};
			other.context_ = nullptr;
		}


		HashDiagonal &operator=(HashDiagonal &&other) noexcept {
			if (this == &other)
				return *this;
			if (raw_methods != nullptr) {
				raw_methods->destruct(&raw_hash_diagonal);
			}
			raw_methods = other.raw_methods;
			raw_methods->move(&other.raw_hash_diagonal, &raw_hash_diagonal);
			context_ = other.context_;
			raw_methods->update_sen_cache_ptr(&raw_hash_diagonal);

			other.raw_methods = nullptr;
			other.raw_hash_diagonal = {};
			other.context_ = nullptr;
			return *this;
		}

		~HashDiagonal() noexcept {
			if (raw_methods != nullptr) {
				raw_methods->destruct(&raw_hash_diagonal);
			}
			raw_methods = nullptr;
			context_ = nullptr;
		}

		/**
		 * Begin the iteration over non-zero diagonal slices for the given diagonal positions (diagonal_poss).
		 * @return reference to self
		 */
		HashDiagonal &begin() noexcept {
			raw_methods->begin(&raw_hash_diagonal);
			return *this;
		}

		[[nodiscard]] bool end() const noexcept {
			return false;
		}

		/**
		 * This method will only return correct results if begin() was called on this and this has not ended(). key_parts from find() are not reflected here.
		 * @return The key part for which the Diagonal iterator currently exposes the diagonal slice.
		 */
		[[nodiscard]] key_part_type current_key_part() const noexcept {
			assert(not ended());
			return raw_methods->current_key_part(&raw_hash_diagonal);
		}

		/**
		 * This method will only return correct results if either
		 * <p>- begin() was called on this and this has not ended()</p>
		 * <p>- or find() was called and returned true. </p>
		 * Further, this must only be called if diag_depth < depth.
		 * @return the hypertrie that results from slicing the the hypertrie with current_key_part() at diagonal positions (diagonal_poss).
		 */
		[[nodiscard]] const_Hypertrie<htt_t, allocator_type> current_hypertrie() const noexcept {
			return raw_methods->current_hypertrie(&raw_hash_diagonal, context_);
		}

		/**
		 * This method will only return correct results if either
		 * <p>- begin() was called on this and this has not ended()</p>
		 * <p>- or find() find was called and returned true. </p>
		 * Further, this must only be called if diag_depth == depth.
		 * @return the scalar value the hypertrie holds for the key where all positions are set to corrent_key_part().
		 */
		[[nodiscard]] value_type current_scalar() const noexcept {
			return raw_methods->current_scalar(&raw_hash_diagonal);
		}

		[[nodiscard]] const_Hypertrie<htt_t, allocator_type> current_scalar_as_tensor() const noexcept {
			Hypertrie<htt_t, allocator_type> scalar_tensor = Hypertrie<htt_t, allocator_type>(0, context_);
			scalar_tensor.set({}, raw_methods->current_scalar(&raw_hash_diagonal));
			return scalar_tensor;
		}

		/** If true, either current_hypertrie() (diag_depth < depth) or current_scalar() (diag_depth == depth) will be safe to use.
		 * @return if there is a non-zero diagonal for key_part
		 */
		[[nodiscard]] bool find(key_part_type key_part) noexcept {
			return raw_methods->find(&raw_hash_diagonal, key_part);
		}

		/**
		 * Prefix increment for the Diagonal iterator.
		 * @return
		 */
		HashDiagonal &operator++() noexcept {
			raw_methods->inc(&raw_hash_diagonal);
			return *this;
		}

		/**
		 * Postfix increment is expensive. Use prefix increment if postfix is not explicitly necessary.
		 * @return
		 */
		HashDiagonal operator++(int) const noexcept {
			auto old = *this;
			++(*this);
			return old;
		}

		/**
		 * inverse of ended()
		 */
		[[nodiscard]] explicit operator bool() const noexcept {
			return not raw_methods->ended(&raw_hash_diagonal);
		}

		/**
		 * If the iteration hash ended. Must only be called after begin() was called.
		 * @return
		 */
		[[nodiscard]] [[nodiscard]] bool ended() const noexcept {
			return raw_methods->ended(&raw_hash_diagonal);
		}

		/**
		 * <divHashDiagonal</div>s are compared by their size().
		 * @param other
		 * @return
		 */
		bool operator==(bool other) const noexcept {
			return bool(*this) == other;
		}

		/**
		 * <divHashDiagonal</div>s are compared by their size().
		 * @param other
		 * @return
		 */
		bool operator<(const HashDiagonal &other) const noexcept {
			return this->size() < other.size();
		}

		/**
		 * Returns an upper bound for the number of diagonal slices.
		 * @return
		 */
		[[nodiscard]] size_t size() const noexcept {
			return raw_methods->size(&raw_hash_diagonal);
		}
	};
}// namespace dice::hypertrie

#endif//HYPERTRIE_HASHDIAGONAL_HPP
