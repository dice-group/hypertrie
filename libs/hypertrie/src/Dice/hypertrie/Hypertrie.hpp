#ifndef HYPERTRIE_HYPERTRIE_HPP
#define HYPERTRIE_HYPERTRIE_HPP

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

//#include "Dice/hypertrie/internal/HashDiagonal.hpp"
#include "Dice/hypertrie/HypertrieContext.hpp"
#include "Dice/hypertrie/Hypertrie_predeclare.hpp"
#include "Dice/hypertrie/internal/Hypertrie_trait.hpp"
#include "Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"
#include "Dice/hypertrie/internal/util/SwitchTemplateFunctions.hpp"
//#include "Dice/hypertrie/internal/Iterator.hpp"

//#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <itertools.hpp>
#include <optional>
#include <variant>
#include <vector>

namespace hypertrie {

	template<hypertrie::internal::HypertrieTrait tr_t>
	class const_Hypertrie {
	public:
		using tr = tr_t;
		using tri = internal::raw::template Hypertrie_core_t<tr>;
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;

		static_assert(sizeof(value_type) <= sizeof(void *), "Types with sizeof larger than void* are not supported as value_type.");


	private:
		template<size_t depth>
		using RawKey_t = internal::raw::RawKey<depth, tri>;

		template<size_t depth>
		using RawSliceKey_t = internal::raw::RawSliceKey<depth, tri>;

		typedef typename internal::raw::RawNodeContainer<tri> RawNodeContainer_t;

		template<size_t depth>
		using NodeContainer_t = internal::raw::NodeContainer<depth, tri>;

		template<size_t depth>
		using stl_NodeContainer_t = internal::raw::NodeContainer<depth, internal::raw::tri_with_stl_alloc<tri>>;

		template<size_t depth>
		const NodeContainer_t<depth> &node_container() const noexcept {
			using namespace internal::util;
			return unsafe_cast<NodeContainer_t<depth> const>(node_container_);
		}

		template<size_t depth>
		NodeContainer_t<depth> &node_container() noexcept {
			using namespace internal::util;
			return unsafe_cast<NodeContainer_t<depth>>(node_container_);
		}

		template<size_t depth>
		const stl_NodeContainer_t<depth> &stl_node_container() const noexcept {
			using namespace internal::util;
			return unsafe_cast<stl_NodeContainer_t<depth> const>(node_container_);
		}

		template<size_t depth>
		stl_NodeContainer_t<depth> &stl_node_container() noexcept {
			using namespace internal::util;
			return unsafe_cast<stl_NodeContainer_t<depth>>(node_container_);
		}

	protected:
		RawNodeContainer_t node_container_{};

		HypertrieContext<tr> *context_ = nullptr;

		size_t depth_ = 0;

		const_Hypertrie(size_t depth, HypertrieContext<tr> *context, RawNodeContainer_t node_container = {})
			: node_container_(std::move(node_container)),
			  context_((depth != 0L) ? context : nullptr),
			  depth_(depth) {}

		[[nodiscard]] constexpr bool contextless() const noexcept {
			return context_ == nullptr;
		}

		//		friend class HashDiagonal<tr>;


		void destruct_contextless_node() noexcept {
			using namespace internal::util;
			using namespace internal::raw;

			static constexpr size_t min_depth = (HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) ? 2 : 1;
			if constexpr (min_depth <= hypertrie_max_depth)
				if (contextless() and not node_container_.is_null_ptr() and depth() != 0) {

					assert(not node_container_.is_null_ptr());
					switch_cases<min_depth, hypertrie_max_depth>(
							this->depth_,
							[&](auto depth_arg) {
								// contextless nodes are always SingleEntryNodes
								assert(this->node_container_.is_sen());
								using SENContainer_t = SENContainer<depth_arg, tri_with_stl_alloc<tri>>;
								auto &sen_node_container = unsafe_cast<SENContainer_t>(this->node_container_);
								// TODO: when we switch over to a pool for SingleEntryNodes, we need to use here AllocateNode::delete_ instead of delete
								delete sen_node_container.node_ptr();
							},
							[]() { assert(false);  __builtin_unreachable(); });
				}
		}

		void copy_contextless_node() noexcept {
			using namespace internal::util;
			using namespace internal::raw;
			static constexpr size_t min_depth = (HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) ? 2 : 1;
			if constexpr (min_depth <= hypertrie_max_depth)
				if (contextless() and not node_container_.is_null_ptr() and depth() != 0) {
					assert(not node_container_.null());
					switch_cases<min_depth, hypertrie_max_depth>(
							this->depth_,
							[&](auto depth_arg) {
								// contextless nodes are always SingleEntryNodes
								assert(this->node_container_.is_sen());
								using SENContainer_t = SENContainer<depth_arg, tri_with_stl_alloc<tri>>;
								using SingleEntryNode_t = typename SENContainer_t::Node;
								auto &sen_node_container = unsafe_cast<SENContainer_t &>(this->node_container_);
								// TODO: when we switch over to a pool for SingleEntryNodes, we need to use here AllocateNode::new_ instead of new_
								sen_node_container.node_ptr() = new SingleEntryNode_t(*sen_node_container.node_ptr());
							},
							[]() { assert(false);  __builtin_unreachable(); });
				}
		}

	public:
		~const_Hypertrie() noexcept {
			destruct_contextless_node();
		}

		const_Hypertrie(const_Hypertrie &&const_hypertrie) noexcept
			: node_container_(const_hypertrie.node_container_), context_(const_hypertrie.context_), depth_(const_hypertrie.depth_) {
			const_hypertrie.node_container_ = {};
			const_hypertrie.context_ = nullptr;
		}

		const_Hypertrie &operator=(const const_Hypertrie &const_hypertrie) noexcept {
			if (this == &const_hypertrie)
				return *this;
			destruct_contextless_node();
			this->node_container_ = const_hypertrie.node_container_;
			this->context_ = const_hypertrie.context_;
			this->depth_ = const_hypertrie.depth_;
			copy_contextless_node();

			return *this;
		}


		const_Hypertrie &operator=(const_Hypertrie &&const_hypertrie) noexcept {
			this->node_container_ = const_hypertrie.node_container_;
			this->context_ = const_hypertrie.context_;
			this->depth_ = const_hypertrie.depth_;
			const_hypertrie.context_ = nullptr;
			const_hypertrie.node_container_.hash_sized = 0;
			const_hypertrie.node_container_.pointer_sized = nullptr;
			return *this;
		}

		const_Hypertrie(const const_Hypertrie &const_hypertrie) noexcept
			: node_container_(const_hypertrie.node_container_), context_(const_hypertrie.context_), depth_(const_hypertrie.depth_) {
			copy_contextless_node();
		}

		const_Hypertrie() = default;

		explicit const_Hypertrie(size_t depth) noexcept : depth_(depth) {}

		[[nodiscard]] void *rawNode() const noexcept {
			return node_container_.pointer_sized;
		}

		const RawNodeContainer_t *rawNodeContainer() const noexcept {
			return &node_container_;
		}

		HypertrieContext<tr> *context() const noexcept {
			return context_;
		}
		[[nodiscard]] size_t depth() const noexcept {
			return depth_;
		}

		[[nodiscard]] size_t hash() const noexcept {
			return node_container_.identifier().hash();
		}

		[[nodiscard]] size_t size() const noexcept {
			if (empty())
				return 0;
			else if (depth() == 0 or node_container_.is_sen())
				return 1;
			else
				// TODO: a differentiation between Identifier<tr> and RawIdentifier<tr, depth> would also be helpful
				// TODO: (non-raw) Identifier needs only a subset of functionality (e.g., read only).
				return internal::util::switch_cases<1, hypertrie_max_depth>(
						this->depth_,
						[&](auto depth_arg) -> size_t {
							using namespace internal::util;
							using namespace internal::raw;

							auto get_size = [&](const auto &nodec) {
								return nodec.template secific<FullNode>().node_ptr()->size();
							};

							if (contextless()) {
								auto &nodec = this->template stl_node_container<depth_arg>();
								return get_size(nodec);
							} else {
								auto &nodec = this->template node_container<depth_arg>();
								return get_size(nodec);
							}
						},
						[]() -> size_t { assert(false);  __builtin_unreachable(); });
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return this->node_container_.empty();
		}

	private:
		[[nodiscard]] value_type get_depth_0_value() const noexcept {
			// TODO: That could be integrated into NodeContainer.
			union {
				value_type val;
				typename RawNodeContainer_t::VoidNodePtr ptr;
			} reinterpret;
			reinterpret.ptr = this->node_container_.void_node_ptr();
			return reinterpret.val;
		}

		template<size_t depth>
		[[nodiscard]] value_type get_depth_n_value(RawKey_t<depth> const &raw_key) const noexcept {
			static_assert(depth != 0, "Use the function above (get_depth_0_value)");

			auto get = [&](const auto &nodec) {
				return this->context()->raw_context().template get<depth>(nodec, raw_key);
			};

			if (contextless()) {
				auto &nodec = this->template stl_node_container<depth>();
				return get(nodec);
			} else {
				auto &nodec = this->template node_container<depth>();
				return get(nodec);
			}
		}

	public:
		[[nodiscard]] value_type operator[](const Key<tr> &key) const {
			assert(key.size() == this->depth());
			if (this->depth() == 0) {
				return get_depth_0_value();
			} else {
				using namespace internal::util;
				using namespace internal::raw;

				return switch_cases<1, hypertrie_max_depth>(
						this->depth_,
						[&](auto depth_arg) -> size_t {
							RawKey<depth_arg, tri> raw_key;
							std::copy_n(key.begin(), depth_arg, raw_key.begin());

							return get_depth_n_value(raw_key);
						},
						[]() -> size_t { assert(false);  __builtin_unreachable(); });
			}
		}

		template<size_t depth>
		[[nodiscard]] value_type operator[](const RawKey_t<depth> &raw_key) const {
			assert(depth == this->depth_);
			if (this->depth() == 0) {
				return get_depth_0_value();
			} else {
				return get_depth_n_value(raw_key);
			}
		}


		[[nodiscard]] std::variant<const_Hypertrie, value_type> operator[](const SliceKey<tr> &slice_key) const noexcept {
			assert(slice_key.size() == depth());
			const size_t fixed_depth = slice_key.get_fixed_depth();

			if (fixed_depth == 0) {
				if (this->depth() == 0)
					return get_depth_0_value();
				else
					return const_Hypertrie(*this);
			} else if (fixed_depth == depth()) {
				Key key(depth());
				std::copy_n(slice_key.begin(), depth(), key.begin());

				return (*this)[key];
			} else {
				using namespace internal::util;
				using namespace internal::raw;

				return switch_cases<1, hypertrie_max_depth>(
						this->depth_,
						[&](auto depth_arg) -> const_Hypertrie<tr> {
							switch_cases<1, depth_arg>(
									fixed_depth,
									[&](auto slice_key_depth_arg) -> const_Hypertrie<tr> {
										RawSliceKey<slice_key_depth_arg, tr> raw_slice_key(slice_key);

										auto slice = [&](const auto &nodec) -> const_Hypertrie<tr> {
											auto slice_result = this->context()->raw_context().template slice<depth>(nodec, raw_slice_key);
											if (slice_result.empty())
												return {depth_arg - slice_key_depth_arg};
											else {
												if (slice_result.is_managed())
													return {depth_arg - slice_key_depth_arg,
															this->context(),
															slice_result.get_managed()};
												else
													return {depth_arg - slice_key_depth_arg,
															nullptr,
															slice_result.get_unmanaged()};
											}
										};

										if (contextless()) {
											auto &nodec = this->template stl_node_container<depth>();
											return slice(nodec);
										} else {
											auto &nodec = this->template node_container<depth>();
											return slice(nodec);
										}
									},
									[]() { assert(false); __builtin_unreachable(); });
						},
						[]() { assert(false); __builtin_unreachable(); });
			}
		}

		[[nodiscard]] std::vector<size_t> getCards(const std::vector<internal::pos_type> &positions) const {
			assert(positions.size() <= depth());
			if (positions.empty())// no positions provided
				return {};
			else if (empty()) {
				return std::vector<size_t>(positions.size(), 0);
			}
			if (depth() == 1) {
				return {size()};
			} else if (size() == 1) {
				return std::vector<size_t>(positions.size(), 1);
			} else {
				using namespace internal::util;
				using namespace internal::raw;

				return switch_cases<2, hypertrie_max_depth>(
						depth_,
						[&](auto depth_arg) -> std::vector<size_t> {
							assert(this->node_container_.is_sen());
							using FNContainer_t = FNContainer<depth_arg, tri>;
							auto &fn_node_container = unsafe_cast<FNContainer_t &>(this->node_container_);
							return fn_node_container.node_ptr()->getCards(positions);
						},
						[]() { assert(false); __builtin_unreachable(); });
			}
		}

		//		using iterator = Iterator<tr>;
		//		using const_iterator = iterator;
		//
		//		[[nodiscard]] iterator begin() const {
		//			if (depth() != 0)
		//				return iterator{*this};
		//			else
		//				throw std::logic_error("Iterator is not yet implemented for depth 0.");
		//		}
		//
		//		[[nodiscard]] const_iterator cbegin() const { return this->begin(); }
		//
		//		[[nodiscard]] bool end() const { return false; }
		//
		//		[[nodiscard]] bool cend() const { return false; }

		[[nodiscard]] bool operator==(const const_Hypertrie<tr> &other) const noexcept {
			return this->hash() == other.hash() and this->depth() == other.depth();
		}

		[[nodiscard]] bool operator==(const Hypertrie<tr> &other) const noexcept {
			return this->hash() == other.hash() and this->depth() == other.depth();
		}

		//		operator std::string() const {
		//			std::vector<std::string> mappings;
		//			if (depth() == 0) {
		//				if (not empty()) {
		//					if constexpr (tr::is_bool_valued)
		//						mappings.push_back("⟨⟩ → true");
		//					else
		//						mappings.push_back(fmt::format("⟨⟩ → {}", this->operator[](Key{})));
		//				}
		//			} else {
		//				for (const auto &entry : *this) {
		//					if constexpr (tr::is_bool_valued)
		//						mappings.push_back(fmt::format("⟨{}⟩ → true", fmt::join(entry, ", ")));
		//					else
		//						mappings.push_back(fmt::format("⟨{}⟩ → {}", fmt::join(entry.first, ", "), entry.second));
		//				}
		//			}
		//			return fmt::format("[ {} ]", fmt::join(mappings, ", "));
		//		}
	};

	template<internal::HypertrieTrait tr_t>
	class Hypertrie : public const_Hypertrie<tr_t> {
	public:
		using tr = tr_t;
		using tri = internal::raw::template Hypertrie_core_t<tr>;
		using key_part_type = typename tr::key_part_type;
		using value_type = typename tr::value_type;

	private:
		template<size_t depth>
		using RawKey_t = internal::raw::RawKey<depth, tri>;

		template<size_t depth>
		using RawSliceKey = internal::raw::RawSliceKey<depth, tri>;

	public:
		value_type set(const Key<tr> &key, value_type value) noexcept {
			using namespace internal::util;
			using namespace internal::raw;
			if (this->depth_ == 0) {
				assert(key.empty());
				union {
					value_type val;
					typename RawNodeContainer<tri>::VoidNodePtr ptr;
				} reinterpret;
				reinterpret.ptr = this->node_container_.void_node_ptr();
				value_type old_value = reinterpret.val;
				reinterpret.ptr = {};
				reinterpret.val = value;
				this->node_container_.void_node_ptr() = reinterpret.ptr;
				if (value_type(0) == value)
					this->node_container_.identifier() = {};
				else {
					this->node_container_.identifier() = RawIdentifier<0, tri>(SingleEntry<0, tri_with_stl_alloc<tri>>{{}, value});
				}
				return old_value;
			} else
				return switch_cases<1, hypertrie_max_depth>(
						this->depth_,
						[&](auto depth_arg) -> value_type {
							SingleEntry<depth_arg, tri_with_stl_alloc<tri>> raw_entry{{}, value};
							std::copy_n(key.begin(), depth_arg, raw_entry.key().begin());
							return this->context()->raw_context().template set<depth_arg>(
									unsafe_cast<NodeContainer<depth_arg, tri>>(this->node_container_),
									{raw_entry});
						},
						[]() -> value_type { assert(false); __builtin_unreachable(); });
		}

		Hypertrie(const Hypertrie<tr> &hypertrie) noexcept : const_Hypertrie<tr>(hypertrie) {
			using namespace internal::util;
			using namespace internal::raw;
			if (not this->empty() and this->depth() != 0)
				switch_cases<1, hypertrie_max_depth>(
						this->depth_,
						[&](auto depth_arg) {
							this->context()->raw_context().template inc_ref_count<depth_arg>(this->node_container_);
						});
		}

		Hypertrie(const const_Hypertrie<tr> &hypertrie) noexcept : const_Hypertrie<tr>(hypertrie) {
			using namespace internal::util;
			using namespace internal::raw;
			if (not this->empty() and this->depth() != 0) {
				if (hypertrie.contextless())// TODO: add copying contextless hypertries
					throw std::logic_error{"Copying contextless const_Hypertries is not yet supported."};
				else
					switch_cases<1UL, hypertrie_max_depth>(
							this->depth_,
							[&](auto depth_arg) {
								this->context()->raw_context().template inc_ref_count<depth_arg>(this->node_container_);
							});
			}
		}

		Hypertrie(Hypertrie<tr> &&other) noexcept : const_Hypertrie<tr>(other) {
			other.node_container_ = {};
		}


		~Hypertrie() {
			using namespace internal::util;
			using namespace internal::raw;
			if (not this->empty() and this->depth() != 0)
				switch_cases<1, hypertrie_max_depth>(
						this->depth_,
						[&](auto depth_arg) {
							this->context()->raw_context().template decr_ref_count<depth_arg>(unsafe_cast<NodeContainer<depth_arg, tri>>(this->node_container_));
						});
		}

		explicit Hypertrie(size_t depth = 1,
						   HypertrieContext<tr> &context = DefaultHypertrieContext<tr>::instance()) noexcept
			: const_Hypertrie<tr>(depth, &context) {}
	};

}// namespace hypertrie

#endif//HYPERTRIE_HYPERTRIE_HPP
