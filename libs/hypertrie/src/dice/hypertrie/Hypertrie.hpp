#ifndef HYPERTRIE_HYPERTRIE_HPP
#define HYPERTRIE_HYPERTRIE_HPP

#include "dice/hypertrie/BulkUpdater_predeclare.hpp"
#include "dice/hypertrie/HashDiagonal.hpp"
#include "dice/hypertrie/HypertrieContext.hpp"
#include "dice/hypertrie/Hypertrie_predeclare.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/Iterator.hpp"
#include "dice/hypertrie/internal/container/AllContainer.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"
#include "dice/template-library/switch_cases.hpp"

#include <optional>
#include <variant>
#include <vector>


namespace dice::hypertrie {

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class const_Hypertrie {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;

		static_assert(sizeof(value_type) <= sizeof(void *), "Types with sizeof larger than void* are not supported as value_type.");

		friend HashDiagonal<htt_t, allocator_type>;
		friend Iterator<htt_t, allocator_type>;

		// BulkUpdater types are set to void if htt_t is not boolean valued. Otherwise, const_Hypertrie template would not be instantiatable in such cases.
		using AsyncBulkInserter = std::conditional_t<HypertrieTrait_bool_valued<htt_t>,
													 ::dice::hypertrie::AsyncBulkInserter<htt_t, allocator_type>,
													 void>;
		using SyncBulkInserter = std::conditional_t<HypertrieTrait_bool_valued<htt_t>,
													::dice::hypertrie::SyncBulkInserter<htt_t, allocator_type>,
													void>;
		using AsyncBulkRemover = std::conditional_t<HypertrieTrait_bool_valued<htt_t>,
													::dice::hypertrie::AsyncBulkRemover<htt_t, allocator_type>,
													void>;
		using SyncBulkRemover = std::conditional_t<HypertrieTrait_bool_valued<htt_t>,
												   ::dice::hypertrie::SyncBulkRemover<htt_t, allocator_type>,
												   void>;

		friend AsyncBulkInserter;
		friend SyncBulkInserter;
		friend AsyncBulkRemover;
		friend SyncBulkRemover;
	protected:
		template<size_t depth>
		using RawKey_t = internal::raw::RawKey<depth, htt_t>;

		template<size_t depth>
		using RawSliceKey_t = internal::raw::RawSliceKey<depth, htt_t>;

		using RawNodeContainer_t = typename internal::raw::RawNodeContainer<htt_t, allocator_type>;

		template<size_t depth>
		using NodeContainer_t = internal::raw::NodeContainer<depth, htt_t, allocator_type>;

		template<size_t depth>
		using stl_NodeContainer_t = internal::raw::SENContainer<depth, htt_t, std::allocator<std::byte>>;

		using RawHypertrieContext_t = typename HypertrieContext<htt_t, allocator_type>::RawHypertrieContext_t;

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
		[[nodiscard]] NodeContainer_t<depth> node_container() const noexcept {
			using namespace internal::util;
			return NodeContainer_t<depth>{node_container_};
		}

		template<size_t depth>
		[[nodiscard]] stl_NodeContainer_t<depth> stl_node_container() const noexcept {
			using namespace internal::util;
			return stl_NodeContainer_t<depth>{node_container_};
		}

		RawNodeContainer_t node_container_{};

		HypertrieContext_ptr_t context_ = nullptr;

		bool managed_ = true;
		uint32_t depth_ = 0;

		const_Hypertrie(size_t depth, HypertrieContext_ptr_t context, bool managed = true, RawNodeContainer_t node_container = {})
			: node_container_((depth == 0 and node_container.empty()) ? RawNodeContainer_t{{}, nullptr} : node_container),
			  context_(context),
			  managed_(managed),
			  depth_(depth) {

		}

		[[nodiscard]] constexpr bool contextless() const noexcept {
			return context_ == nullptr;
		}

		[[nodiscard]] constexpr bool unmanaged() const noexcept {
			return not managed_;
		}

		void destruct_contextless_node() noexcept {
			using namespace internal::util;
			using namespace internal::raw;


			if constexpr (sen_min_depth <= hypertrie_max_depth) {
				if (contextless() and unmanaged() and not node_container_.is_raw_null_ptr() and depth() != 0) {
					template_library::switch_cases<sen_min_depth, hypertrie_max_depth + 1>(
							this->depth_,
							[&](auto depth_arg) {
								// contextless nodes are always SingleEntryNodes
								assert(this->node_container_.is_sen() and not this->node_container_.empty());
								using SENContainer_t = SENContainer<depth_arg, htt_t, std::allocator<std::byte>>;
								SENContainer_t sen_node_container{this->node_container_};
								// TODO: when we switch over to a pool for SingleEntryNodes, we need to use here AllocateNode::delete_ instead of delete
								delete sen_node_container.node_ptr();
							},
							[]() { assert(false); __builtin_unreachable(); });
				}
			}
		}

		void copy_contextless_node() noexcept {
			using namespace internal::util;
			using namespace internal::raw;
			if constexpr (sen_min_depth <= hypertrie_max_depth) {
				if (contextless() and unmanaged() and not node_container_.is_raw_null_ptr() and depth() != 0) {
					template_library::switch_cases<sen_min_depth, hypertrie_max_depth + 1>(
							this->depth_,
							[&](auto depth_arg) {
								// contextless nodes are always SingleEntryNodes
								assert(this->node_container_.is_sen());
								using SENContainer_t = SENContainer<depth_arg, htt_t, std::allocator<std::byte>>;
								using SingleEntryNode_t = typename SENContainer_t::Node;
								SENContainer_t sen_node_container{this->node_container_};
								// TODO: when we switch over to a pool for SingleEntryNodes, we need to use here AllocateNode::new_ instead of new_
								sen_node_container.node_ptr(new SingleEntryNode_t(*sen_node_container.node_ptr()));
								this->node_container_ = sen_node_container;
							},
							[]() { assert(false);  __builtin_unreachable(); });
				}
			}
		}

	public:
		~const_Hypertrie() noexcept {
			destruct_contextless_node();
		}

		const_Hypertrie(const_Hypertrie &&other) noexcept
			: node_container_(other.node_container_), context_(other.context_), managed_(other.managed_), depth_(other.depth_) {
			assert(this != &other);
			other.node_container_ = {};
			other.context_ = {};
			other.managed_ = true;
			other.depth_ = 0;
		}

		const_Hypertrie &operator=(const_Hypertrie const &const_hypertrie) noexcept {
			if (this == &const_hypertrie)
				return *this;
			destruct_contextless_node();
			this->node_container_ = const_hypertrie.node_container_;
			this->context_ = const_hypertrie.context_;
			this->managed_ = const_hypertrie.managed_;
			this->depth_ = const_hypertrie.depth_;
			copy_contextless_node();

			return *this;
		}

		const_Hypertrie &operator=(const_Hypertrie &&other) noexcept {
			assert(this != &other);
			this->node_container_ = other.node_container_;
			this->context_ = other.context_;
			this->managed_ = other.managed_;
			this->depth_ = other.depth_;
			other.context_ = nullptr;
			other.node_container_ = {};
			other.managed_ = true;
			other.depth_ = 0;
			return *this;
		}

		const_Hypertrie(const_Hypertrie const &other) noexcept
			: node_container_(other.node_container_), context_(other.context_), managed_(other.managed_), depth_(other.depth_) {
			if (this != &other)
				copy_contextless_node();
		}

		const_Hypertrie() = default;

		explicit const_Hypertrie(size_t depth) noexcept : depth_(depth) {}

		[[nodiscard]] RawNodeContainer_t const *raw_node_container() const noexcept {
			return &node_container_;
		}

		[[nodiscard]] HypertrieContext_ptr_t context() const noexcept {
			return context_;
		}
		[[nodiscard]] size_t depth() const noexcept {
			return depth_;
		}

		[[nodiscard]] size_t hash() const noexcept {
			return node_container_.identifier().hash();
		}

		[[nodiscard]] size_t size() const noexcept {
			if (empty()) {
				return 0;
			}
			if (depth() == 0 or node_container_.is_sen()) {
				return 1;
			}
			return template_library::switch_cases<1, hypertrie_max_depth + 1>(
					this->depth_,
					[&](auto depth_arg) -> size_t {
						using namespace internal::util;
						using namespace internal::raw;

						auto get_size = [&](const auto &nodec) {
							return nodec.template specific<FullNode>().node_ptr()->size();
						};

						if (contextless()) {
							auto nodec = this->template stl_node_container<depth_arg>();
							return get_size(nodec);
						}
						auto nodec = this->template node_container<depth_arg>();
						return get_size(nodec);
					},
					[]() -> size_t { assert(false);  __builtin_unreachable(); });
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return this->node_container_.empty();
		}

	protected:
		[[nodiscard]] static value_type decode_depth_0_value(void *ptr) noexcept {
			union {
				value_type val;
				void *ptr;
			} reinterpret{.ptr = ptr};
			return reinterpret.val;
		}

		[[nodiscard]] static void *encode_depth_0_value(value_type value) noexcept {
			union {
				value_type val;
				void *ptr;
			} reinterpret{.ptr = nullptr};
			reinterpret.val = value;
			return reinterpret.ptr;
		}

	private:
		template<size_t depth>
		[[nodiscard]] value_type get_depth_n_value(RawKey_t<depth> const &raw_key) const noexcept {
			static_assert(depth != 0, "Use the function above (get_depth_0_value)");

			auto get = [&](auto const &nodec) {
				return this->context()->raw_context().template get<depth>(nodec, raw_key);
			};

			if (contextless()) {
				using namespace internal::raw;
				auto nodec = this->template stl_node_container<depth>();
				return RawHypertrieContext_t::template get<depth, std::allocator<std::byte>>(nodec, raw_key);
			}
			auto nodec = this->template node_container<depth>();
			return get(nodec);
		}

		// TODO: move to definition to the file of compile time switch case
		/**
		 * Wrapper for nested switch_cases where the outer counter dictates the inner one (Outer >= Inner).
		 * So for example with Max=2, the values (1,1), (2,1), (2,2) will be generated (Outer, Inner).
		 * @tparam Max The maximum value for the outer counter.
		 * @tparam Lambda Type of the function to execute.
		 * @param a The runtime outer variable.
		 * @param b The runtime inner variable.
		 * @param f The function to execute.
		 * @return f(a,b), but a and b are compile time values.
		 */
		template <std::size_t Max, typename Lambda> static auto upperTriangleMatrix(std::size_t a, std::size_t b, Lambda f) noexcept {
			using namespace internal::util;
			auto error = []() -> decltype(f(Max,Max)) {assert(false); __builtin_unreachable();};
			return template_library::switch_cases<1, Max+1>(a, [&](auto X) {
			  return template_library::switch_cases<1, X+1>(b, [&](auto Y) {
				return f(X,Y);
			  }, error);
			}, error);
		}

	public:
		[[nodiscard]] value_type operator[](Key<htt_t> const &key) const noexcept {
			assert(key.size() == this->depth());
			if (this->depth() == 0) {
				return decode_depth_0_value(this->node_container_.raw_void_node_ptr());
			}
			using namespace internal::util;
			using namespace internal::raw;

			return template_library::switch_cases<1, hypertrie_max_depth + 1>(
					this->depth_,
					[&](auto depth_arg) -> size_t {
						RawKey<depth_arg, htt_t> raw_key;
						std::copy_n(key.begin(), depth_arg, raw_key.begin());

						return get_depth_n_value(raw_key);
					},
					[]() -> size_t { assert(false);  __builtin_unreachable(); });
		}

		template<size_t depth>
		[[nodiscard]] value_type operator[](RawKey_t<depth> const &raw_key) const noexcept {
			assert(depth == this->depth_);
			if (this->depth() == 0) {
				return decode_depth_0_value(this->node_container_.raw_void_node_ptr());
			}
			return get_depth_n_value(raw_key);
		}

		[[nodiscard]] std::variant<const_Hypertrie, value_type> operator[](SliceKey<htt_t> const &slice_key) const noexcept {
			assert(slice_key.size() == depth());
			const size_t fixed_depth = slice_key.get_fixed_depth();
			if (fixed_depth == 0) {
				if (this->depth() == 0) {
					return decode_depth_0_value(this->node_container_.raw_void_node_ptr());
				}
				return const_Hypertrie(*this);
			}
			if (fixed_depth == depth()) {
				Key<htt_t> key(depth());
				for (size_t i = 0; i < depth(); ++i) {
					key[i] = slice_key[i].value();
				}
				return (*this)[key];
			}

			using namespace internal::raw;
			if (empty()) {
				return const_Hypertrie(depth() - fixed_depth);
			}
			return upperTriangleMatrix<hypertrie_max_depth>(this->depth_, fixed_depth, [&](auto depth_arg, auto slice_key_depth_arg) -> const_Hypertrie<htt_t, allocator_type> {
				if constexpr (depth_arg != slice_key_depth_arg) {
					RawSliceKey<slice_key_depth_arg, htt_t> raw_slice_key(slice_key);
					if (contextless()) {
						auto nodec = stl_node_container<depth_arg>();
						auto slice_result = RawHypertrieContext_t::slice(nodec, RawSliceKey<slice_key_depth_arg, htt_t>(raw_slice_key));
						if (slice_result.empty()) {
							return const_Hypertrie<htt_t, allocator_type>(depth_arg - slice_key_depth_arg);
						}
						return {depth_arg - slice_key_depth_arg, nullptr, false, static_cast<RawNodeContainer_t>(slice_result.get_stl_alloc_sen())};
					}

					auto nodec = this->template node_container<depth_arg>();
					auto slice_result = this->context()->raw_context().template slice<depth_arg>(nodec, raw_slice_key);
					if (slice_result.empty()) {
						return const_Hypertrie<htt_t, allocator_type>(depth_arg - slice_key_depth_arg);
					}
					if (slice_result.uses_provided_alloc()) {
						return {depth_arg - slice_key_depth_arg, this->context(), true, slice_result.get_raw_nodec()};
					}
					return {depth_arg - slice_key_depth_arg, nullptr, false, slice_result.get_stl_alloc_sen()};
				}
				assert(false);
				__builtin_unreachable();
			});
		}

		[[nodiscard]] std::vector<size_t> get_cards(const std::vector<internal::pos_type> &positions) const {
			assert(positions.size() <= depth());
			if (positions.empty()) {// no positions provided
				return {};
			}
			if (empty()) {
				return std::vector<size_t>(positions.size(), 0);
			}
			if (depth() == 1) {
				return {size()};
			}
			if (size() == 1) {
				return std::vector<size_t>(positions.size(), 1);
			}

			using namespace internal::util;
			using namespace internal::raw;

			return template_library::switch_cases<2, hypertrie_max_depth + 1>(
					depth_,
					[&](auto depth_arg) -> std::vector<size_t> {
						assert(this->node_container_.is_fn());
						FNContainer<depth_arg, htt_t, allocator_type> fn_node_container = this->node_container_;
						return fn_node_container.node_ptr()->getCards(positions);
					},
					[]() -> std::vector<size_t> { assert(false); __builtin_unreachable(); });
		}

		using iterator = Iterator<htt_t, allocator_type>;
		using const_iterator = iterator;

		[[nodiscard]] iterator begin() const {
			if (depth() != 0) [[likely]]
				return iterator{*this};
			else [[unlikely]]
				throw std::logic_error("Iterator is not yet implemented for depth 0.");
		}

		[[nodiscard]] const_iterator cbegin() const { return this->begin(); }

		[[nodiscard]] bool end() const noexcept { return false; }

		[[nodiscard]] bool cend() const noexcept { return false; }

		[[nodiscard]] bool operator==(const const_Hypertrie<htt_t, allocator_type> &other) const noexcept {
			return this->hash() == other.hash() and this->depth() == other.depth();
		}

		[[nodiscard]] bool operator==(const Hypertrie<htt_t, allocator_type> &other) const noexcept {
			return this->hash() == other.hash() and this->depth() == other.depth();
		}

		explicit operator std::string() const noexcept {
			std::string out = "[ ";
			out += "depth: " + std::to_string(depth()) +
				   ", hash: " + std::to_string(hash()) +
				   ", context: " + std::to_string(uintptr_t(std::to_address(context()))) +
				   ", unmanaged: " + std::to_string(unmanaged());
			bool first = true;
			for (const auto &entry : *this) {
				if (first)
					first = false;
				else {
					out += ",";
				}
				out += "\n" + std::string(entry);
			}
			return out + " ]";
		}
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class Hypertrie : public const_Hypertrie<htt_t, allocator_type> {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = typename htt_t::value_type;

		using HypertrieContext_ptr_t = HypertrieContext_ptr<htt_t, allocator_type>;


	private:
		using super_t = const_Hypertrie<htt_t, allocator_type>;
		static constexpr bool bool_valued_and_taggable_key_part = super_t::bool_valued_and_taggable_key_part;

		static constexpr size_t sen_min_depth = super_t::sen_min_depth;


		template<size_t depth>
		using RawKey_t = internal::raw::RawKey<depth, htt_t>;

		template<size_t depth>
		using RawSliceKey_t = internal::raw::RawSliceKey<depth, htt_t>;

	public:
		value_type set(const Key<htt_t> &key, value_type value) noexcept {
			using namespace internal::util;
			using namespace internal::raw;
			if (this->depth_ == 0) {
				assert(key.empty());
				value_type old_value = this->decode_depth_0_value(this->node_container_.raw_void_node_ptr());
				this->node_container_.raw_void_node_ptr() = this->encode_depth_0_value(value);

				if (value_type(0) == value) {
					this->node_container_.identifier() = {};
				} else {
					this->node_container_.identifier() = RawIdentifier<0, htt_t>(SingleEntry<0, htt_t>{{}, value});
				}
				return old_value;
			}
			// TODO: it seems like The raw_context() is problematic.
			auto result = template_library::switch_cases<1, hypertrie_max_depth + 1>(
					this->depth_,
					[&](auto depth_arg) -> value_type {
						RawKey<depth_arg, htt_t> raw_key{};
						std::copy_n(key.begin(), depth_arg, raw_key.begin());
						NodeContainer<depth_arg, htt_t, allocator_type> nodec{this->node_container_};
						auto result = this->context()->raw_context().template set<depth_arg>(
								nodec,
								raw_key,
								value);
						this->node_container_ = nodec;
						return result;
					},
					[]() -> value_type { assert(false); __builtin_unreachable(); });
			return result;
		}

		Hypertrie(const Hypertrie<htt_t, allocator_type> &hypertrie) noexcept : const_Hypertrie<htt_t, allocator_type>(hypertrie) {
			using namespace internal::util;
			using namespace internal::raw;
			if (not this->empty() and this->depth() != 0)
				template_library::switch_cases<1, hypertrie_max_depth + 1>(
						this->depth_,
						[&](auto depth_arg) {
							if constexpr (bool_valued_and_taggable_key_part and depth_arg  < sen_min_depth) {
								if (this->node_container_.is_sen()) {
									return;
								}
							}
							this->context()->raw_context().template inc_ref_count<depth_arg>(this->template node_container<depth_arg>());
						});
		}

		Hypertrie &operator=(Hypertrie<htt_t, allocator_type> const &other) noexcept {
			this->node_container_ = other.node_container_;
			this->context_ = other.context_;
			this->depth_ = other.depth_;
			if (not this->empty() and this->depth() != 0) {
				if (this->contextless()) {// TODO: add copying contextless hypertries
					throw std::logic_error{"Copying contextless const_Hypertries is not yet supported."};
				}
				template_library::switch_cases<1UL, hypertrie_max_depth + 1>(
						this->depth_,
						[&](auto depth_arg) {
							if constexpr (bool_valued_and_taggable_key_part and depth_arg  < sen_min_depth) {
								if (this->node_container_.is_sen()) {
									return;
								}
							}
							this->context()->raw_context().template inc_ref_count<depth_arg>(this->template node_container<depth_arg>());
						});
			}
			return *this;
		}

		Hypertrie(const const_Hypertrie<htt_t, allocator_type> &hypertrie) noexcept : const_Hypertrie<htt_t, allocator_type>(hypertrie) {
			using namespace internal::util;
			using namespace internal::raw;
			if (not this->empty() and this->depth() != 0) {
				if (hypertrie.contextless()) {// TODO: add copying contextless hypertries
					throw std::logic_error{"Copying contextless const_Hypertries is not yet supported."};
				}
				template_library::switch_cases<1UL, hypertrie_max_depth + 1>(
						this->depth_,
						[&](auto depth_arg) {
							if constexpr (bool_valued_and_taggable_key_part and depth_arg  < sen_min_depth) {
								if (this->node_container_.is_sen()) {
									return;
								}
							}
							this->context()->raw_context().template inc_ref_count<depth_arg>(this->template node_container<depth_arg>());
						});
			}
		}

		Hypertrie(Hypertrie<htt_t, allocator_type> &&other) noexcept : const_Hypertrie<htt_t, allocator_type>(other) {
			other.node_container_ = {};
			other.depth_ = {};
			other.managed_ = true;
			other.context_ = {};
		}

		Hypertrie &operator=(Hypertrie<htt_t, allocator_type> &&other) noexcept {
			this->node_container_ = other.node_container_;
			this->context_ = other.context_;
			this->managed_ = other.managed_;
			this->depth_ = other.depth_;
			other.context_ = {};
			other.node_container_ = {};
			other.managed_ = true;
			other.depth_ = {};

			return *this;
		}


		~Hypertrie() {
			using namespace internal::util;
			using namespace internal::raw;
			if (not this->empty() and this->depth() != 0) {
				assert(not this->contextless());
				template_library::switch_cases<1, hypertrie_max_depth + 1>(
						this->depth_,
						[&](auto depth_arg) {
							NodeContainer<depth_arg, htt_t, allocator_type> nodec{this->node_container_};
							if constexpr (bool_valued_and_taggable_key_part and depth_arg  < sen_min_depth)
								if (nodec.is_sen())
									return;
							this->context()->raw_context().template decr_ref_count<depth_arg>(nodec);
						});
			}
		}

		explicit Hypertrie(size_t depth = 1) noexcept
			: const_Hypertrie<htt_t, allocator_type>(depth, &DefaultHypertrieContext<htt_t, allocator_type>::instance(), true) {}

		explicit Hypertrie(size_t depth, HypertrieContext_ptr_t context) noexcept
			: const_Hypertrie<htt_t, allocator_type>(depth, context, true) {}
	};

}// namespace dice::hypertrie

#endif//HYPERTRIE_HYPERTRIE_HPP
