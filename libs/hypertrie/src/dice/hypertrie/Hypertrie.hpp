#ifndef HYPERTRIE_HYPERTRIE_HPP
#define HYPERTRIE_HYPERTRIE_HPP

#include "dice/hypertrie/BulkUpdater_predeclare.hpp"
#include "dice/hypertrie/HashDiagonal.hpp"
#include "dice/hypertrie/HypertrieContext.hpp"
#include "dice/hypertrie/Hypertrie_predeclare.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/Iterator.hpp"
#include "dice/hypertrie/internal/util/Unreachable.hpp"
#include "dice/hypertrie/internal/container/AllContainer.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawBulkUpdater.hpp"
#include "dice/hypertrie/internal/raw/node_context/SynchronousRawBulkUpdater.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"
#include "dice/template-library/switch_cases.hpp"

#include <optional>
#include <variant>
#include <vector>

namespace dice::hypertrie {

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct IndexProxy {
		friend Hypertrie<htt_t, allocator_type>;

		using value_type = typename htt_t::value_type;

	private:
		template<size_t depth>
		using RawIndexProxy_t = internal::raw::RawIndexProxy<depth, hypertrie_max_depth, htt_t, allocator_type>;

		using max_sized_RawIndexProxy_t = RawIndexProxy_t<hypertrie_max_depth>;

		// Insurance that there is no need to put relevant constructors and destructor into the vtable
		static_assert(std::is_trivially_destructible_v<max_sized_RawIndexProxy_t>);
		static_assert(std::is_trivially_copyable_v<max_sized_RawIndexProxy_t>);
		static_assert(std::is_trivially_move_constructible_v<max_sized_RawIndexProxy_t>);
		static_assert(std::is_trivially_move_assignable_v<max_sized_RawIndexProxy_t>);

		struct VTable {
			void (*set)(void *self, value_type value) noexcept;
			value_type (*get)(void const *self) noexcept;

			template<size_t depth>
			static consteval VTable from_concrete() noexcept {
				return VTable{
						.set = [](void *self, value_type const value) noexcept {
							reinterpret_cast<RawIndexProxy_t<depth> *>(self)->set(value);
						},
						.get = [](void const *self) noexcept {
							return reinterpret_cast<RawIndexProxy_t<depth> const *>(self)->get();
						}};
			}
		};

		template<size_t ...ixs>
		static consteval std::array<VTable, sizeof...(ixs)> make_vtables(std::index_sequence<ixs...>) {
			return {VTable::template from_concrete<ixs + 1>()...};
		}

		static constexpr std::array<VTable, hypertrie_max_depth - 1> vtable_instances = make_vtables(std::make_index_sequence<hypertrie_max_depth - 1>{});

		union {
			struct {
				alignas(max_sized_RawIndexProxy_t) std::byte raw_[sizeof(max_sized_RawIndexProxy_t)];
				VTable const *vtable_;
			}; // active if depth_ > 0

			value_type *scalar_; // active if depth_ == 0
		};

		uint8_t depth_;

		template<size_t depth> requires (depth > 0)
		explicit IndexProxy(internal::raw::RawIndexProxy<depth, hypertrie_max_depth, htt_t, allocator_type> const &raw_proxy) noexcept : depth_{depth} {
			// SAFETY: as per static_assert above this is fine
			new (raw_) internal::raw::RawIndexProxy<depth, hypertrie_max_depth, htt_t, allocator_type>{raw_proxy};
			vtable_ = &vtable_instances[depth - 1];
		}

		explicit IndexProxy(value_type *scalar) noexcept : depth_{0} {
			scalar_ = scalar;
		}

	public:
		IndexProxy(IndexProxy const &other) noexcept = default;
		IndexProxy &operator=(IndexProxy const &other) noexcept  = default;
		IndexProxy(IndexProxy &&other) noexcept = default;
		IndexProxy &operator=(IndexProxy &&other) noexcept = default;
		~IndexProxy() noexcept = default;

		value_type get() const noexcept {
			if (depth_ == 0) {
				return *scalar_;
			}

			return vtable_->get(raw_);
		}

		void set(value_type const value) noexcept {
			if (depth_ == 0) {
				*scalar_ = value;
			} else {
				vtable_->set(raw_, value);
			}
		}

		operator value_type() const noexcept {
			return this->get();
		}

		IndexProxy &operator=(value_type const value) noexcept {
			this->set(value);
			return *this;
		}
	};

	// TODO: rename this class to HypertrieView
	template<HypertrieTrait htt_t_, ByteAllocator allocator_type>
	class const_Hypertrie {
	public:
		using htt_t = htt_t_;
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;

		friend HashDiagonal<htt_t, allocator_type>;
		friend Iterator<htt_t, allocator_type>;

		friend AsyncBulkInserter<htt_t, allocator_type>;
		friend SyncBulkInserter<htt_t, allocator_type>;
		friend AsyncBulkRemover<htt_t, allocator_type>;
		friend SyncBulkRemover<htt_t, allocator_type>;
	protected:
		template<HypertrieTrait, ByteAllocator>
		friend class Hypertrie;

		using Ownership = internal::raw::Ownership;

		template<size_t depth>
		using RawKey_t = internal::raw::RawKey<depth, htt_t>;

		template<size_t depth>
		using RawSliceKey_t = internal::raw::RawSliceKey<depth, htt_t>;

		using RawNodePtr_t = internal::raw::RawNodePtr<htt_t, allocator_type>;

		template<size_t depth>
		using NodePtr_t = internal::raw::NodePtr<depth, htt_t, allocator_type>;

		using HypertrieContext_ptr_t = HypertrieContext_ptr<htt_t, allocator_type>;

		/**
		 * Boolean flag if the hypertrie is bool valued and if the key_parts are taggable.
		 */
		static constexpr bool bool_valued_and_taggable_key_part = HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>;

		/**
		 * Minimum depth of <dev>SingleEntryNode</dev> (SEN). SENs of smaller depths must not exist (e.g. depth 0) or are stored in-place (e.g. depth 1 if bool_valued_and_taggable_key_part).
		 */
		static constexpr size_t sen_min_depth = (bool_valued_and_taggable_key_part) ? 2 : 1;

		template<size_t depth>
		[[nodiscard]] NodePtr_t<depth> &node_ptr() noexcept requires (depth >= 1) {
			return static_cast<NodePtr_t<depth> &>(node_ptr_);
		}

		template<size_t depth>
		[[nodiscard]] NodePtr_t<depth> const &node_ptr() const noexcept requires (depth >= 1) {
			return static_cast<NodePtr_t<depth> const &>(node_ptr_);
		}

		[[nodiscard]] Ownership ownership() const noexcept {
			return ownership_;
		}

		// General Maintenance note:
		// Unions with non-primitive _need_ placement new (i.e. new (ptr) T{}) to switch their active member.
		// Not doing so results in undefined behaviour because a simple assignment does not in general create an object in the C++ memory model.
		// As per C++ Standard Sec 11.5: "[Note: In general, one must use explicit destructor calls and placement new-expression to change the active member of a union. — end note]" (Referring to non-primitive unions)
		// We can omit destructors here because of the two static asserts below.
		union {
			static_assert(std::is_trivially_destructible_v<RawNodePtr_t>);
			static_assert(std::is_trivially_destructible_v<value_type>);

			RawNodePtr_t node_ptr_; // active if depth > 0
			value_type scalar_;     // active if depth == 0
		};
		HypertrieContext_ptr_t context_ = nullptr;
		uint8_t depth_ = 0;
		Ownership ownership_ = Ownership::ContextBorrowed;

		/**
		 * @return if this hypertrie is responsible for freeing the node it references
		 * @note true for owned nodes
		 */
		[[nodiscard]] bool needs_destruction() const noexcept {
			return ownership_ == Ownership::Owned && depth_ > 0 && node_ptr_ != nullptr;
		}

		/**
		 * @return if the node this hypertrie references needs to be copied when this hypertrie is copied
		 * @note true for non-context-borrowed nodes
		 */
		[[nodiscard]] bool needs_copy() const noexcept {
			assert(depth_ > 0);
			return (ownership_ == Ownership::Owned || ownership_ == Ownership::EphemeralBorrowed) && node_ptr_ != nullptr;
		}

		/**
		 * @return if the node this hypertrie references needs to be copied if this hypertrie is moved
		 * @note true for short lived nodes
		 */
		[[nodiscard]] bool needs_copy_on_move() const noexcept {
			assert(depth_ > 0);
			return ownership_ == Ownership::EphemeralBorrowed && node_ptr_ != nullptr;
		}

		/**
		 * destroys the node this hypertrie references if it is necessary
		 * to do so on destruction of this hypertrie
		 */
		void destroy_if_required() noexcept {
			using namespace internal::util;
			using namespace internal::raw;

			if constexpr (hypertrie_max_depth >= sen_min_depth) {
				if (needs_destruction()) {
					template_library::switch_cases<sen_min_depth, hypertrie_max_depth + 1>(
							depth_,
							[this](auto depth_arg) noexcept {
								assert(node_ptr_ != nullptr && (node_ptr_.is_sen() || node_ptr_.is_xn())); // unmanaged nodes are either XN or SEN

								// TODO: when we switch over to a pool for SingleEntryNodes, we need to use here AllocateNode::delete_ instead of delete
								switch (auto ptr = this->template node_ptr<depth_arg>(); ptr.tag()) {
									case IdentifierTag::SEN: {
										delete std::to_address(ptr.template specific_ptr<SingleEntryNode>());
										break;
									}
									case IdentifierTag::XN: {
										delete std::to_address(ptr.template specific_ptr<CartesianNode>());
										break;
									}
									default: {
										HYPERTRIE_UNREACHABLE;
									}
								}
							});
				}
			}
		}

		/**
		 * makes a copy of the node this hypertrie references and assigns itself the new copy
		 * @warning the old node will become dangling if nothing else references it at this point
		 */
		void do_copy() noexcept {
			using namespace internal::util;
			using namespace internal::raw;

			template_library::switch_cases<sen_min_depth, hypertrie_max_depth + 1>(
					depth_,
					[this](auto depth_arg) noexcept {
						assert(node_ptr_ != nullptr && (node_ptr_.is_sen() || node_ptr_.is_xn())); // unmanaged nodes are either XN or SEN

						ownership_ = Ownership::Owned;

						// TODO: when we switch over to a pool for SingleEntryNodes, we need to use here AllocateNode::new_ instead of new_
						switch (auto const ptr = node_ptr<depth_arg>(); ptr.tag()) {
							case IdentifierTag::SEN: {
								new (&node_ptr_) RawNodePtr_t{NodePtr_t<depth_arg>{new SingleEntryNode<depth_arg, htt_t>{*ptr.template specific_ptr<SingleEntryNode>()}}};
								break;
							}
							case IdentifierTag::XN: {
								new (&node_ptr_) RawNodePtr_t{NodePtr_t<depth_arg>{new CartesianNode<depth_arg, htt_t, allocator_type>{*ptr.template specific_ptr<CartesianNode>()}}};
								break;
							}
							default: {
								HYPERTRIE_UNREACHABLE;
							}
						}
					});
		}

		/**
		 * makes a copy of the node this hypertrie references if it is required for copy construction (i.e. this->needs_copy())
		 */
		void copy_if_required() noexcept {
			if constexpr (hypertrie_max_depth >= sen_min_depth) {
				if (needs_copy()) {
					do_copy();
				}
			}
		}

		/**
		 * makes a copy of the node this hypertrie references if it is required for move construction (i.e. this->needs_copy_on_move())
		 */
		void copy_if_required_for_move() noexcept {
			if constexpr (hypertrie_max_depth >= sen_min_depth) {
				if (needs_copy_on_move()) {
					do_copy();
				}
			}
		}

		const_Hypertrie(size_t depth,
						HypertrieContext_ptr_t context,
						Ownership ownership,
						RawNodePtr_t node_ptr = {}) noexcept : context_{context},
															   depth_{static_cast<uint8_t>(depth)},
															   ownership_{ownership} {
			if (depth_ == 0) {
				assert(node_ptr == RawNodePtr_t{});
				new (&scalar_) value_type{};
			} else {
				new (&node_ptr_) RawNodePtr_t{node_ptr};
			}
		}

	public:
		const_Hypertrie() noexcept : depth_{0} {
			new (&scalar_) value_type{};
		}

		explicit const_Hypertrie(size_t depth) noexcept : depth_{static_cast<uint8_t>(depth)} {
			if (depth_ == 0) {
				new (&scalar_) value_type{};
			} else {
				new (&node_ptr_) RawNodePtr_t{};
			}
		}

		static const_Hypertrie from_scalar(value_type const scalar) noexcept {
			const_Hypertrie hyp{0};
			hyp.scalar_ = scalar;
			return hyp;
		}

		const_Hypertrie(const_Hypertrie const &other) noexcept : context_{other.context_},
																 depth_{other.depth_},
																 ownership_{other.ownership_} {
			if (depth_ == 0) {
				new (&scalar_) value_type{other.scalar_};
			} else {
				new (&node_ptr_) RawNodePtr_t{other.node_ptr_};
				copy_if_required();
			}
		}

		const_Hypertrie(const_Hypertrie &&other) noexcept : context_{other.context_},
															depth_{other.depth_},
															ownership_{std::exchange(other.ownership_, Ownership::ContextBorrowed)} {
			if (depth_ == 0) {
				new (&scalar_) value_type{std::exchange(other.scalar_, {})};
			} else {
				new (&node_ptr_) RawNodePtr_t{std::exchange(other.node_ptr_, {})};
				copy_if_required_for_move();
			}
		}

		const_Hypertrie &operator=(const_Hypertrie const &other) noexcept {
			if (this == &other) {
				return *this;
			}

			destroy_if_required();
			context_ = other.context_;
			depth_ = other.depth_;
			ownership_ = other.ownership_;

			if (depth_ == 0) {
				new (&scalar_) value_type{other.scalar_};
			} else {
				new (&node_ptr_) RawNodePtr_t{other.node_ptr_};
				copy_if_required();
			}

			return *this;
		}

		const_Hypertrie &operator=(const_Hypertrie &&other) noexcept {
			assert(this != &other);

			destroy_if_required();
			context_ = other.context_;
			depth_ = other.depth_;
			ownership_ = std::exchange(other.ownership_, Ownership::ContextBorrowed);

			if (depth_ == 0) {
				new (&scalar_) value_type{std::exchange(other.scalar_, {})};
			} else {
				new (&node_ptr_) RawNodePtr_t{std::exchange(other.node_ptr_, {})};
				copy_if_required_for_move();
			}

			return *this;
		}

		~const_Hypertrie() noexcept {
			destroy_if_required();
		}

		[[nodiscard]] RawNodePtr_t const &raw_node_ptr() const noexcept {
			assert(depth_ > 0);
			return node_ptr_;
		}

		[[nodiscard]] HypertrieContext_ptr_t context() const noexcept {
			return context_;
		}
		[[nodiscard]] size_t depth() const noexcept {
			return depth_;
		}

		[[nodiscard]] size_t size() const noexcept {
			if (depth_ == 0) {
				return static_cast<size_t>(scalar_ != value_type{});
			}

			if (node_ptr_ == nullptr) {
				return 0;
			}

			return template_library::switch_cases<1, hypertrie_max_depth + 1>(
					this->depth_,
					[&](auto depth_arg) noexcept -> size_t {
						return node_ptr<depth_arg>().size();
					});
		}

		/**
		 * @return The hash of this hypertrie
		 */
		[[nodiscard]] size_t hash() const noexcept {
			if (depth_ == 0) {
				using namespace internal::raw;
				return scalar_ == value_type{} ? RawIdentifier<0, htt_t>{}.hash()
											   : RawIdentifier<0, htt_t>{SingleEntry<0, htt_t>{{}, scalar_}}.hash();
			}

			return template_library::switch_cases<1, hypertrie_max_depth + 1>(depth_, [this](auto depth_arg) noexcept {
				return this->context_->raw_context().hash(this->template node_ptr<depth_arg>(), this->ownership_);
			});
		}

		[[nodiscard]] bool empty() const noexcept {
			if (depth_ == 0) {
				return scalar_ == value_type{};
			}

			return node_ptr_ == nullptr;
		}

	private:
		// TODO: move to definition to the file of compile time switch case
		/**
		 * Wrapper for nested switch_cases where the outer counter dictates the inner one (Outer >= Inner).
		 * So for example with Max=2, the values (1,1), (2,1), (2,2) will be generated (Outer, Inner).
		 * @tparam max The maximum value for the outer counter.
		 * @tparam F Type of the function to execute.
		 * @param a The runtime outer variable.
		 * @param b The runtime inner variable.
		 * @param f The function to execute.
		 * @return f(a,b), but a and b are compile time values.
		 */
		template<size_t max, typename F>
		static decltype(auto) upper_triangle_matrix(size_t a, size_t b, F f) noexcept {
			return template_library::switch_cases<1, max + 1>(a, [&](auto ca) noexcept  {
				return template_library::switch_cases<1, ca + 1>(b, [&](auto cb) noexcept {
					return f(ca, cb);
				});
			});
		}

	public:
		[[nodiscard]] value_type to_scalar() const noexcept {
			assert(depth_ == 0);
			return scalar_;
		}

		[[nodiscard]] value_type operator[](Key<htt_t> const &key) const noexcept {
			assert(key.size() == this->depth());
			if (depth_ == 0) {
				return scalar_;
			}

			return template_library::switch_cases<1, hypertrie_max_depth + 1>(
					depth_,
					[this, &key](auto depth_arg) noexcept {
						internal::raw::RawKey<depth_arg, htt_t> raw_key; {
							std::copy_n(key.begin(), depth_arg, raw_key.begin());
						}

						return context_->raw_context().get(node_ptr<depth_arg>(), raw_key);
					});
		}

		template<size_t depth>
		[[nodiscard]] value_type operator[](RawKey_t<depth> const &raw_key) const noexcept {
			assert(depth == depth_);

			if constexpr (depth == 0) {
				return scalar_;
			} else {
				return context_->raw_context().get(node_ptr<depth>(), raw_key);
			}
		}

		[[nodiscard]] const_Hypertrie operator[](SliceKey<htt_t> const &slice_key) const noexcept {
			assert(slice_key.size() == depth_);
			size_t const fixed_depth = slice_key.get_fixed_depth();

			if (fixed_depth == 0) {
				return const_Hypertrie{*this};
			}

			if (empty()) {
				return const_Hypertrie{depth_ - fixed_depth};
			}

			if (fixed_depth == depth_) {
				auto key = slice_key.into_fixed_key_unchecked();
				return const_Hypertrie::from_scalar((*this)[key]);
			}

			return upper_triangle_matrix<hypertrie_max_depth>(depth_, fixed_depth, [this, &slice_key](auto depth_arg, auto slice_key_depth_arg) noexcept -> const_Hypertrie {
				if constexpr (depth_arg != slice_key_depth_arg) {
					static constexpr size_t result_depth = depth_arg - slice_key_depth_arg;
					RawSliceKey_t<slice_key_depth_arg> raw_slice_key(slice_key);

					auto slice_result = context_->raw_context().template slice(node_ptr<depth_arg>(), raw_slice_key);
					if (slice_result.empty()) {
						return const_Hypertrie<htt_t, allocator_type>(depth_arg - slice_key_depth_arg);
					}

					auto const ownership = slice_result.ownership();
					auto slice_result_node = slice_result.release_node_ptr();

					return const_Hypertrie{result_depth,
										   context_,
										   ownership,
										   slice_result_node};
				} else {
					HYPERTRIE_UNREACHABLE;
				}
			});
		}

		[[nodiscard]] std::vector<size_t> get_cards(std::vector<internal::pos_type> const &positions) const noexcept {
			assert(positions.size() <= depth_);
			if (depth_ == 0 || positions.empty()) {
				return {};
			}
			if (empty()) {
				return std::vector<size_t>(positions.size(), 0);
			}
			if (depth_ == 1) {
				return {size()};
			}
			if (size() == 1) {
				return std::vector<size_t>(positions.size(), 1);
			}

			return template_library::switch_cases<2, hypertrie_max_depth + 1>(
					depth_,
					[this, &positions](auto depth_arg) noexcept {
						return context_->raw_context().get_cards(node_ptr<depth_arg>(), positions);
					});
		}

		using iterator = Iterator<htt_t, allocator_type>;
		using const_iterator = iterator;

		[[nodiscard]] iterator begin() const noexcept { return iterator{*this}; }
		[[nodiscard]] const_iterator cbegin() const noexcept { return const_iterator{*this}; }
		[[nodiscard]] std::default_sentinel_t end() const noexcept { return std::default_sentinel; }
		[[nodiscard]] std::default_sentinel_t cend() const noexcept { return std::default_sentinel; }

		[[nodiscard]] bool operator==(const_Hypertrie<htt_t, allocator_type> const &other) const noexcept {
			if (depth_ != other.depth_) {
				return false;
			}

			if (depth_ == 0) {
				return scalar_ == other.scalar_;
			}

			return template_library::switch_cases<1, hypertrie_max_depth + 1>(depth_, [this, &other](auto depth_arg) noexcept {
				return context_->raw_context().equal(this->template node_ptr<depth_arg>(), this->ownership_,
													 other.template node_ptr<depth_arg>(), other.ownership_);
			});
		}

		[[nodiscard]] bool operator==(Hypertrie<htt_t, allocator_type> const &other) const noexcept {
			return *this == static_cast<const_Hypertrie<htt_t, allocator_type> const &>(other);
		}

		explicit operator std::string() const noexcept {
			std::string out = "[ ";
			out += "depth: " + std::to_string(depth()) +
				   ", hash: " + std::to_string(hash()) +
				   ", context: " + std::to_string(uintptr_t(std::to_address(context()))) +
				   ", ownership: " + to_string(ownership_);
			bool first = true;
			for (const auto &entry : *this) {
				if (first)
					first = false;
				else {
					out += ",";
				}
				out += "\n" + to_string(entry);
			}
			return out + " ]";
		}
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	std::ostream &operator<<(std::ostream &os, const_Hypertrie<htt_t, allocator_type> const &hyp) {
		os << static_cast<std::string>(hyp);
		return os;
	}

	template<HypertrieTrait htt_t_, ByteAllocator allocator_type>
	class Hypertrie : public const_Hypertrie<htt_t_, allocator_type> {
	public:
		using htt_t = htt_t_;
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;
		using index_proxy_type = IndexProxy<htt_t, allocator_type>;

		using HypertrieContext_ptr_t = HypertrieContext_ptr<htt_t, allocator_type>;

	private:
		using super_t = const_Hypertrie<htt_t, allocator_type>;
		using Ownership = typename super_t::Ownership;
		using RawNodePtr_t = typename super_t::RawNodePtr_t;

		void inc_ref() noexcept {
			if (this->depth_ == 0) {
				return;
			}

			if (this->node_ptr_ == nullptr) {
				return;
			}

			if constexpr (HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				if (this->depth_ == 1 && this->node_ptr_.is_sen()) {
					return;
				}
			}

			template_library::switch_cases<1, hypertrie_max_depth + 1>(
					this->depth_,
					[this](auto depth_arg) noexcept {
						this->context()->raw_context().inc_ref_count(this->template node_ptr<depth_arg>());
					});
		}

		void dec_ref() noexcept {
			if (this->depth_ == 0) {
				return;
			}

			if (this->node_ptr_ == nullptr) {
				return;
			}

			if constexpr (HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				if (this->depth_ == 1 && this->node_ptr_.is_sen()) {
					return;
				}
			}

			template_library::switch_cases<1, hypertrie_max_depth + 1>(
					this->depth_,
					[this](auto depth_arg) noexcept {
						this->context_->raw_context().decr_ref_count(this->template node_ptr<depth_arg>());
					});
		}

	public:
		Hypertrie() noexcept
			: super_t{0, nullptr, Ownership::ContextBorrowed} {
		}

		explicit Hypertrie(size_t depth) noexcept
			: super_t{depth, &DefaultHypertrieContext<htt_t, allocator_type>::instance(), Ownership::ContextBorrowed} {
		}

		explicit Hypertrie(size_t depth, HypertrieContext_ptr_t context) noexcept
			: super_t{depth, context, Ownership::ContextBorrowed} {
		}

		Hypertrie(const_Hypertrie<htt_t, allocator_type> const &hypertrie) : super_t{hypertrie} {
			if (hypertrie.ownership_ != Ownership::ContextBorrowed) {// TODO: add copying for non-context-managed hypertries
				throw std::logic_error{"Copying from non-context-managed const_Hypertries is not yet supported."};
			}

			inc_ref();
		}

		Hypertrie(Hypertrie<htt_t, allocator_type> const &hypertrie) noexcept : super_t{hypertrie} {
			inc_ref();
		}

		Hypertrie(Hypertrie<htt_t, allocator_type> &&other) noexcept : super_t{std::move(other)} {
		}

		Hypertrie &operator=(Hypertrie<htt_t, allocator_type> const &other) noexcept {
			if (this == &other) {
				return *this;
			}

			dec_ref();
			this->context_ = other.context_;
			this->depth_ = other.depth_;
			this->ownership_ = other.ownership_;

			if (this->depth_ == 0) {
				new (&this->scalar_) value_type{other.scalar_};
			} else {
				new (&this->node_ptr_) RawNodePtr_t{other.node_ptr_};
			}

			inc_ref();
			return *this;
		}

		Hypertrie &operator=(Hypertrie<htt_t, allocator_type> &&other) noexcept {
			assert(this != &other);

			dec_ref();
			this->context_ = std::exchange(other.context_, nullptr);
			this->depth_ = other.depth_;
			this->ownership_ = other.ownership_;

			if (this->depth_ == 0) {
				new (&this->scalar_) value_type{std::exchange(other.scalar_, {})};
			} else {
				new (&this->node_ptr_) RawNodePtr_t{std::exchange(other.node_ptr_, {})};
			}

			return *this;
		}

		~Hypertrie() noexcept {
			dec_ref();
		}

		[[nodiscard]] static Hypertrie from_scalar(value_type const scalar) noexcept {
			using namespace internal::raw;

			Hypertrie hyp{0};
			hyp.scalar_ = scalar;
			return hyp;
		}

		struct default_value_converter {
			template<typename Src>
			typename htt_t::value_type operator()(Src const value) const noexcept {
				return static_cast<typename htt_t::value_type>(value);
			}
		};

		/**
		 * Inserts all entries (internal::raw::SingleEntry, Entry or NonZeroEntry) contained in the given range (iter..sent) into this hypertrie.
		 * Optionally converts each value using the given conversion function (convert_func).
		 *
		 * @note The hypertrie trait of the yielded entries does not have to match the hypertrie trait of *this exactly,
		 * 		only their key_part_types have to match.
		 *
		 * @note Takes the first non-zero value for any given key, all subsequent (and zero) values are ignored
		 *
		 * @param iter some input iterator yielding internal::raw::SingleEntry, Entry or NonZeroEntry
		 * @param sent the sentinel value for iter (e.g. something like `container.end()`)
		 * @param convert_func unary function to convert the values of the yielded entries into the value_type of *this
		 * 		by default this is equivalent to a static_cast to the value_type of *this.
		 */
		template<typename Iter, typename Sentinel, typename F = default_value_converter>
			requires ((is_Entry_v<std::remove_cv_t<std::iter_value_t<Iter>>> || is_NonZeroEntry_v<std::remove_cv_t<std::iter_value_t<Iter>>> || internal::raw::is_SingleEntry_v<std::remove_cv_t<std::iter_value_t<Iter>>>)
					 && std::is_same_v<typename std::remove_cv_t<std::iter_value_t<Iter>>::htt_t::key_part_type, typename htt_t::key_part_type>
					 && std::is_invocable_r_v<typename htt_t::value_type, F, typename std::remove_cv_t<std::iter_value_t<Iter>>::htt_t::value_type>)
		void extend_from_iter(Iter iter, Sentinel sent, F &&convert_func = {}) noexcept(internal::raw::is_SingleEntry_v<std::remove_cv_t<std::iter_value_t<Iter>>>) {

			using iter_value_type = std::remove_cv_t<std::iter_value_t<Iter>>;
			using iter_htt_t = typename iter_value_type::htt_t;

			if constexpr (internal::raw::is_SingleEntry_v<iter_value_type>) {
				assert(iter_value_type::depth == this->depth_);
			}

			if (iter == sent) {
				return;
			}

			if (this->depth_ == 0) {
				for (; iter != sent; ++iter) {
					if constexpr (!internal::raw::is_SingleEntry_v<iter_value_type>) {
						if ((*iter).key().size() != 0) {
							throw std::logic_error{"Hypertrie::extend_from_iter depth mismatch"};
						}
					}

					auto new_value = std::invoke(convert_func, (*iter).value());
					if (new_value == typename htt_t::value_type{}) [[unlikely]] {
						continue;
					}

					this->set(internal::raw::RawKey<0, htt_t>{}, new_value);
					return;
				}

				return;
			}

			template_library::switch_cases<1, hypertrie_max_depth + 1>(this->depth(), [this, &iter, &sent, &convert_func](auto depth_arg) {
				using Inserter_t = internal::raw::SynchronousRawHypertrieBulkInserter<depth_arg, htt_t, allocator_type, hypertrie_max_depth>;
				Inserter_t inserter{this->template node_ptr<depth_arg>(),
									this->context()->raw_context()};

				for (; iter != sent; ++iter) {
					auto new_value = std::invoke(convert_func, (*iter).value());
					if (new_value == typename htt_t::value_type{}) [[unlikely]] {
						continue;
					}

					if constexpr (is_Entry_v<iter_value_type> || is_NonZeroEntry_v<iter_value_type>) {
						static_assert(sizeof(Key<htt_t>) == sizeof(Key<iter_htt_t>)
									  && alignof(Key<htt_t>) == alignof(Key<iter_htt_t>));

						// SAFETY: ensured by requires clause and static_assert
						inserter.add(NonZeroEntry<htt_t>{*reinterpret_cast<Key<htt_t> const *>(&(*iter).key()),
														 new_value});
					} else /* is_SingleEntry_v<iter_value_type> */ {
						static_assert(sizeof(internal::raw::RawKey<depth_arg, htt_t>) == sizeof(internal::raw::RawKey<depth_arg, iter_htt_t>)
									  && alignof(internal::raw::RawKey<depth_arg, htt_t>) == alignof(internal::raw::RawKey<depth_arg, iter_htt_t>));

						// SAFETY: ensured by requires clause and static_assert
						inserter.add(internal::raw::SingleEntry<depth_arg, htt_t>{*reinterpret_cast<internal::raw::RawKey<depth_arg, htt_t> const *>(&(*iter).key()),
																				  new_value});
					}
				}
			});
		}

		/**
		 * Inserts all entries from the given hypertrie into *this.
		 * Optionally converts each value using the given conversion function (convert_func).
		 *
		 * @note The hypertrie trait of the given "source" hypertrie does not have to match the hypertrie trait of *this exactly,
		 * 		only their key_part_types have to match.
		 *
		 * @note Takes the first non-zero value for any given key, all subsequent (and zero) values are ignored
		 *
		 * @tparam src_htt_t hypertrie trait of the source hypertrie
		 * @param src hypertrie to read entries from
		 * @param convert_func unary function to convert the values of the source hypertrie to the value_type of *this
		 * 		by default this is equivalent to a static_cast to the value_type of *this.
		 */
		template<HypertrieTrait src_htt_t, typename F = default_value_converter>
			requires std::is_same_v<typename htt_t::key_part_type, typename src_htt_t::key_part_type>
					 && std::is_invocable_r_v<typename htt_t::value_type, F, typename src_htt_t::value_type>
		void extend(const_Hypertrie<src_htt_t, allocator_type> const &src, F &&convert_func = {}) {
			if (this->depth() != src.depth()) {
				throw std::invalid_argument{"Hypertrie::extend depth mismatch"};
			}

			if (src.empty()) {
				return;
			}

			if (this->depth() == 0) {
				auto new_value = std::invoke(convert_func, src.to_scalar());
				if (new_value == typename htt_t::value_type{}) [[unlikely]] {
					return;
				}

				this->set(internal::raw::RawKey<0, htt_t>{}, new_value);
				return;
			}

			template_library::switch_cases<1, hypertrie_max_depth + 1>(this->depth(), [this, &src, &convert_func](auto depth_arg) noexcept {
				using Inserter_t = internal::raw::SynchronousRawHypertrieBulkInserter<depth_arg, htt_t, allocator_type, hypertrie_max_depth>;
				using Iter_t = internal::raw::RawIterator<depth_arg, true, src_htt_t, allocator_type>;

				Iter_t src_iter{src.template node_ptr<depth_arg>()};

				Inserter_t inserter{this->template node_ptr<depth_arg>(),
									this->context()->raw_context()};

				for (; src_iter != std::default_sentinel; ++src_iter) {
					auto new_value = std::invoke(convert_func, src_iter->value());
					if (new_value == typename htt_t::value_type{}) [[unlikely]] {
						continue;
					}

					inserter.add(internal::raw::SingleEntry<depth_arg, htt_t>{*reinterpret_cast<internal::raw::RawKey<depth_arg, htt_t> const *>(&src_iter->key()),
																			  new_value});
				}
			});
		}

		template<size_t depth>
		value_type set(internal::raw::RawKey<depth, htt_t> const &key, value_type value) noexcept {
			assert(depth == this->depth_);

			if constexpr (depth == 0) {
				return std::exchange(this->scalar_, value);
			} else {
				return this->context()->raw_context().set(this->template node_ptr<depth>(), key, value);
			}
		}

		value_type set(Key<htt_t> const &key, value_type value) noexcept {
			using namespace internal::util;
			using namespace internal::raw;
			assert(key.size() == this->depth_);

			if (this->depth_ == 0) {
				return std::exchange(this->scalar_, value);
			}

			return template_library::switch_cases<1, hypertrie_max_depth + 1>(
					this->depth_,
					[this, &key, value](auto depth_arg) noexcept {
						RawKey<depth_arg, htt_t> raw_key; {
							std::copy_n(key.begin(), depth_arg, raw_key.begin());
						}

						return this->context()->raw_context().set(this->template node_ptr<depth_arg>(), raw_key, value);
					});
		}

		using super_t::operator[];

		index_proxy_type operator[](Key<htt_t> const &key) noexcept {
			return template_library::switch_cases<0, hypertrie_max_depth + 1>(
					key.size(),
					[&](auto depth_arg) -> index_proxy_type {
						internal::raw::RawKey<depth_arg, htt_t> raw_key;
						std::copy_n(key.begin(), depth_arg, raw_key.begin());

						return (*this)[raw_key];
					});
		}

		template<size_t depth>
		index_proxy_type operator[](internal::raw::RawKey<depth, htt_t> const &raw_key) noexcept {
			using namespace internal::raw;
			assert(this->depth() == depth);

			if constexpr (depth == 0) {
				return index_proxy_type{&this->scalar_};
			} else {
				RawIndexProxy<depth, hypertrie_max_depth, htt_t, allocator_type> raw_proxy{
						&this->context()->raw_context(),
						&this->template node_ptr<depth>(),
						raw_key};

				return index_proxy_type{raw_proxy};
			}
		}

		bool operator==(Hypertrie<htt_t, allocator_type> const &other) const noexcept {
			return static_cast<const_Hypertrie<htt_t, allocator_type> const &>(*this) == static_cast<const_Hypertrie<htt_t, allocator_type> const &>(other);
		}

		bool operator==(const_Hypertrie<htt_t, allocator_type> const &other) const noexcept {
			return static_cast<const_Hypertrie<htt_t, allocator_type> const &>(*this) == other;
		}
	};

}// namespace dice::hypertrie

namespace std {
	template<::dice::hypertrie::HypertrieTrait htt_t, ::dice::hypertrie::ByteAllocator allocator_type>
	struct hash<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> {
		size_t operator()(::dice::hypertrie::const_Hypertrie<htt_t, allocator_type> const &hyp) const noexcept {
			return hyp.hash();
		}
	};

	template<::dice::hypertrie::HypertrieTrait htt_t, ::dice::hypertrie::ByteAllocator allocator_type>
	struct hash<::dice::hypertrie::Hypertrie<htt_t, allocator_type>> {
		size_t operator()(::dice::hypertrie::Hypertrie<htt_t, allocator_type> const &hyp) const noexcept {
			return hyp.hash();
		}
	};
} // namespace std

#endif//HYPERTRIE_HYPERTRIE_HPP
