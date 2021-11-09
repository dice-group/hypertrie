#ifndef HYPERTRIE_HYPERTRIE_HPP
#define HYPERTRIE_HYPERTRIE_HPP

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

#include "Dice/hypertrie/internal/HashDiagonal.hpp"
#include "Dice/hypertrie/internal/HypertrieContext.hpp"
#include "Dice/hypertrie/internal/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/Iterator.hpp"

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <optional>
#include <variant>
#include <vector>
#include <itertools.hpp>

namespace hypertrie {

	template<HypertrieTrait tr>
	class const_Hypertrie {
	public:
		using tri = internal::raw::Hypertrie_internal_t<tr>;

	private:
		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

		typedef typename internal::raw::RawNodeContainer NodeContainer;

	protected:
		NodeContainer node_container_{};

		HypertrieContext<tr> *context_ = nullptr;

		size_t depth_ = 0;

		const_Hypertrie(size_t depth, HypertrieContext<tr> *context, NodeContainer node_container = {})
			: node_container_(std::move(node_container)),
			  context_((depth != 0L) ? context : nullptr),
			  depth_(depth) {}

		constexpr bool contextless() const noexcept {
			return context_ == nullptr;
		}

		friend class HashDiagonal<tr>;


		void destruct_contextless_node() noexcept {
			if constexpr (tr::compressed_nodes) {
				if (contextless() and node_container_.hash_sized != 0 and depth() != 0) {
					if (tr::is_bool_valued and tr::lsb_unused and depth_ == 1 and size() == 1)
						return;
					using namespace internal;
					assert(node_container_.pointer_sized != nullptr);
					compiled_switch<hypertrie_depth_limit, 1>::switch_void(
							this->depth_,
							[&](auto depth_arg) {
								using CND = typename internal::raw::template CompressedNodeContainer<depth_arg, tri>;
								if constexpr (not(depth_arg == 1 and tri::is_bool_valued and tri::is_lsb_unused))
									delete reinterpret_cast<CND *>(&this->node_container_)->compressed_node();
							},
							[]() { assert(false); });
				}
			}
		}

		void copy_contextless_node() noexcept {
			if constexpr (tr::compressed_nodes) {
				if (contextless() and not empty() and depth() != 0) {
					if (not(tr::is_bool_valued and tr::lsb_unused and depth_ == 1 and size() == 1))internal::compiled_switch<hypertrie_depth_limit, 1>::switch_void(
							this->depth_,
							[&](auto depth_arg) {
								using CNodec = typename internal::raw::template NodeContainer<depth_arg, tri>;
								if constexpr (not(depth_arg == 1 and tri::is_bool_valued and tri::is_lsb_unused)) {
									// create a copy of the contextless compressed node
									CNodec &cnodec = *reinterpret_cast<CNodec *>(&this->node_container_);
									cnodec.compressed_node() = new internal::raw::CompressedNode<depth_arg, tri>{*cnodec.compressed_node()};;
								}
							},
							[]() { assert(false); });
				}
			}
		}
	public:
		~const_Hypertrie() {
			if constexpr (tr::compressed_nodes)
				destruct_contextless_node();
		}

		const_Hypertrie(const_Hypertrie &&const_hypertrie)
				: node_container_(const_hypertrie.node_container_), context_(const_hypertrie.context_), depth_(const_hypertrie.depth_) {
			const_hypertrie.node_container_ = {};
			const_hypertrie.context_ = nullptr;
		}

		const_Hypertrie &operator=(const const_Hypertrie &const_hypertrie) noexcept {
			if constexpr (tr::compressed_nodes)
				destruct_contextless_node();
			this->node_container_ = const_hypertrie.node_container_;
			this->context_ = const_hypertrie.context_;
			this->depth_ = const_hypertrie.depth_;
			if constexpr (tr::compressed_nodes)
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

		const_Hypertrie(const const_Hypertrie &const_hypertrie)
			: node_container_(const_hypertrie.node_container_), context_(const_hypertrie.context_), depth_(const_hypertrie.depth_) {
			if constexpr (tr::compressed_nodes)
				copy_contextless_node();
		}

		const_Hypertrie() = default;

		explicit const_Hypertrie(size_t depth) : depth_(depth) {}

		void *rawNode() const {
			return node_container_.pointer_sized;
		}

		const NodeContainer *rawNodeContainer() const {
			return &node_container_;
		}

		HypertrieContext<tr> *context() const {
			return context_;
		}
		size_t depth() const {
			return depth_;
		}

		size_t hash() const {
			return node_container_.hash_sized.hash();
		}


		using traits = tr;

		using Key = typename tr::Key;
		using SliceKey = typename tr::SliceKey;
		using value_type = typename tr::value_type;

		[[nodiscard]] size_t size() const{
			if (empty())
				return 0;
			else
				if (depth() == 0)
					return 1;
				else
					return internal::compiled_switch<hypertrie_depth_limit, 1>::switch_(
							this->depth_,
							[&](auto depth_arg) -> size_t {
								const auto &node_container = *reinterpret_cast<const internal::raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
								if constexpr (tr::compressed_nodes)
									if (node_container.isCompressed())
										return node_container.compressed_node()->size();
									else
										return node_container.uncompressed_node()->size();
								else
									return node_container.uncompressed_node()->size();
							},
							[]() -> size_t { assert(false); return 0; });
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return this->node_container_.hash_sized.empty();
		}

		[[nodiscard]] value_type operator[](const Key &key) const {
			if (this->depth() == 0) {
				assert(key.empty());
				if constexpr (sizeof(value_type) <= sizeof(void *)){
					union { value_type val; void * ptr; } reinterpret;
					reinterpret.ptr = this->node_container_.pointer_sized;
					return reinterpret.val;
				} else {
					throw std::logic_error{"Types with sizeof larger than void* are not supported"};
				}
			} else
				return internal::compiled_switch<hypertrie_depth_limit, 1>::switch_(
						this->depth_,
						[&](auto depth_arg) mutable -> value_type {
							RawKey<depth_arg> raw_key;
							std::copy_n(key.begin(), depth_arg, raw_key.begin());
							const auto &node_container = *reinterpret_cast<const internal::raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
							return this->context()->rawContext().template get<depth_arg>(
									node_container,
									raw_key);
						},
						[]() -> value_type { assert(false); return {}; });
		}


		[[nodiscard]] std::variant<const_Hypertrie, value_type> operator[](const SliceKey &slice_key) const {
			assert(slice_key.size() == depth());
			const size_t fixed_depth = tri::sliceKeyFixedDepth(slice_key);

			if (fixed_depth == 0) {
				if (this->depth() == 0)
					return this->operator[](Key{});
				else
					return const_Hypertrie(*this);
			} else if (fixed_depth == depth()) {
				Key key(slice_key.size());
				for (auto [key_part, slice_key_part] : iter::zip(key, slice_key))
					key_part = slice_key_part.value();
				return this->operator[](key);
			} else {
				const_Hypertrie<tr> result{depth_ - fixed_depth};
				internal::compiled_switch<hypertrie_depth_limit, 1>::switch_void(
						this->depth_,
						[&](auto depth_arg) {
						  internal::compiled_switch<depth_arg, 1>::switch_void(
									fixed_depth,
									[&](auto slice_key_depth_arg) {
										RawSliceKey<slice_key_depth_arg> raw_slice_key(slice_key);

										const auto &node_container = *reinterpret_cast<const internal::raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);

										auto [node_cont, is_managed] = this->context()->rawContext().template slice<depth_arg, slice_key_depth_arg>(node_container, raw_slice_key);
										if (not node_cont.empty())
											result = const_Hypertrie<tr>(
													depth_arg - slice_key_depth_arg,
													(is_managed) ? this->context() : nullptr,
													{node_cont.hash().hash(), node_cont.node()});
									});
						});
				return result;
			}
		}

		[[nodiscard]]
		std::vector<size_t> getCards(const std::vector<pos_type> &positions) const {
			assert(positions.size() <= depth());
			if (positions.size() == 0) // no positions provided
				return {};
			else if (empty()) {
				return std::vector<size_t>(positions.size(), 0);
			} if (depth() == 1){
				return {size()};
			} else if (size() == 1) {
				return std::vector<size_t>(positions.size(),1);
			} else {
				return internal::compiled_switch<hypertrie_depth_limit, 2>::switch_(
						this->depth_,
						[&](auto depth_arg) -> std::vector<size_t> {
							const auto &node_container = *reinterpret_cast<const internal::raw::UncompressedNodeContainer<depth_arg, tri> *>(&this->node_container_);
							return node_container.uncompressed_node()->getCards(positions);
						},
						[&]() ->std::vector<size_t> {
							assert(false); return {};
						});
			}
		}

		using iterator = Iterator<tr>;
		using const_iterator = iterator;

		[[nodiscard]] iterator begin() const {
			if (depth() != 0)
				return iterator{*this};
			else
				throw std::logic_error("Iterator is not yet implemented for depth 0.");
		}

		[[nodiscard]] const_iterator cbegin() const { return this->begin(); }

		[[nodiscard]]
		bool end() const { return false; }

		[[nodiscard]]
		bool cend() const { return false; }

		[[nodiscard]]
		bool operator==(const const_Hypertrie<tr> &other) const noexcept {
			return this->hash() == other.hash() and this->depth() == other.depth();
		}

		[[nodiscard]]
		bool operator==(const Hypertrie<tr> &other) const noexcept {
			return this->hash() == other.hash() and this->depth() == other.depth();
		}

		operator std::string() const {
			std::vector<std::string> mappings;
			if (depth() == 0) {
				if (not empty()){
					if constexpr (tr::is_bool_valued)
						mappings.push_back("⟨⟩ → true");
					else
						mappings.push_back(fmt::format("⟨⟩ → {}", this->operator[](Key{})));
				}
			} else {
				for (const auto &entry : *this) {
					if constexpr (tr::is_bool_valued)
						mappings.push_back(fmt::format("⟨{}⟩ → true", fmt::join(entry, ", ")));
					else
						mappings.push_back(fmt::format("⟨{}⟩ → {}", fmt::join(entry.first, ", "), entry.second));
				}
			}
			return fmt::format("[ {} ]", fmt::join(mappings, ", "));
		}

	};

	template<HypertrieTrait tr>
	class Hypertrie : public const_Hypertrie<tr>{
	public:
		using traits = tr;
		using tri = internal::raw::Hypertrie_internal_t<tr>;
		using Key = typename tr::Key;
		using value_type = typename tr::value_type;

	private:
		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

	public:
		value_type set(const Key &key, value_type value) {
			if (this->depth_ == 0) {
				assert(key.empty());
				if constexpr (sizeof(value_type) <= sizeof(void *)){
					union { value_type val; void * ptr; } reinterpret;
					reinterpret.ptr = this->node_container_.pointer_sized;
					value_type old_value = reinterpret.val;
					reinterpret.ptr = nullptr;
					reinterpret.val = value;
					this->node_container_.pointer_sized = reinterpret.ptr;
					if (value_type(0) == value)
						this->node_container_.hash_sized = 0L;
					else
						this->node_container_.hash_sized = internal::raw::TensorHash().addFirstEntry(RawKey<0L>(), value).hash();
					return old_value;
				} else {
					throw std::logic_error{"Types with sizeof larger than void* are not supported"};
				}
			} else
				return internal::compiled_switch<hypertrie_depth_limit, 1>::switch_(
						this->depth_,
						[&](auto depth_arg) -> value_type {
							RawKey<depth_arg> raw_key;
							std::copy_n(key.begin(), depth_arg, raw_key.begin());
							auto &node_container = *reinterpret_cast<internal::raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
							return this->context_->rawContext().template set<depth_arg>(node_container, raw_key, value);
						},
						[]() -> value_type { assert(false); return {}; });
		}

		Hypertrie(const Hypertrie<tr> &hypertrie) : const_Hypertrie<tr>(hypertrie) {
			if (not this->empty())
				internal::compiled_switch<hypertrie_depth_limit, 1>::switch_void(
						this->depth_,
						[&](auto depth_arg){
						  auto &typed_nodec = *reinterpret_cast<internal::raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
						  this->context_->rawContext().template incRefCount<depth_arg>(typed_nodec);
						}
				);
		}

		Hypertrie(const const_Hypertrie<tr> &hypertrie) : const_Hypertrie<tr>(hypertrie) {
			if (hypertrie.depth() != 0) {
				if (hypertrie.contextless())// TODO: add copying contextless hypertries
					throw std::logic_error{"Copying contextless const_Hypertries is not yet supported."};
				else
					internal::compiled_switch<hypertrie_depth_limit, 1>::switch_void(
							this->depth_,
							[&](auto depth_arg) {
								auto &typed_nodec = *reinterpret_cast<internal::raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
								this->context_->rawContext().template incRefCount<depth_arg>(typed_nodec); });
			}
		}

		Hypertrie(Hypertrie<tr> &&other) : const_Hypertrie<tr>(other) {
			other.node_container_ = {};
		}



		~Hypertrie() {
			if (not this->empty())
				internal::compiled_switch<hypertrie_depth_limit, 1>::switch_void(
						this->depth_,
						[&](auto depth_arg){
							auto &typed_nodec = *reinterpret_cast<internal::raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
							this->context_->rawContext().template decrRefCount<depth_arg>(typed_nodec);
						}
				);
		}

		Hypertrie(size_t depth = 1, HypertrieContext<tr> &context = DefaultHypertrieContext<tr>::instance())
			: const_Hypertrie<tr>(depth, &context) {}
	};

}// namespace hypertrie

#endif//HYPERTRIE_HYPERTRIE_HPP
