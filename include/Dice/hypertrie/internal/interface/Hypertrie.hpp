#ifndef HYPERTRIE_HYPERTRIE_HPP
#define HYPERTRIE_HYPERTRIE_HPP

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

#include "Dice/hypertrie/internal/interface/HashDiagonal.hpp"
#include "Dice/hypertrie/internal/interface/HypertrieContext.hpp"
#include "Dice/hypertrie/internal/interface/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/interface/Iterator.hpp"

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <optional>
#include <variant>
#include <vector>
#include <itertools.hpp>

namespace hypertrie::internal::node_based {

	template<HypertrieTrait tr>
	class const_Hypertrie {
	public:
		using tri = raw::Hypertrie_internal_t<tr>;

	private:
		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

		using NodeContainer = typename raw::RawNodeContainer;

	protected:
		NodeContainer node_container_;

		HypertrieContext<tr> *context_ = nullptr;

		size_t depth_ = 0;

		const_Hypertrie(size_t depth, HypertrieContext<tr> *context, NodeContainer node_container = {}) : node_container_(std::move(node_container)), context_(context), depth_(depth) {}

		constexpr bool contextless() const noexcept {
			return context_ == nullptr;
		}

		friend class HashDiagonal<tr>;


	public:
		~const_Hypertrie() {
			if (contextless() and node_container_.hash_sized != 0) {
				assert(node_container_.pointer_sized != nullptr);
				compiled_switch<hypertrie_depth_limit, 1>::switch_void(
						this->depth_,
						[&](auto depth_arg) {
							using CND = typename raw::template CompressedNodeContainer<depth_arg, tri>;
							if constexpr (not(depth_arg == 1 and tri::is_bool_valued and tri::is_lsb_unused))
								delete reinterpret_cast<CND *>(&this->node_container_)->compressed_node();
						},
						[]() { assert(false); });
			}
		}

		const_Hypertrie(const const_Hypertrie &const_hypertrie)
			: node_container_(const_hypertrie.node_container_), context_(const_hypertrie.context_), depth_(const_hypertrie.depth_) {
			if (contextless() and not empty()) {
				compiled_switch<hypertrie_depth_limit, 1>::switch_void(
						this->depth_,
						[&](auto depth_arg) {
							using CND = typename raw::template CompressedNodeContainer<depth_arg, tri>;
							if constexpr (not(depth_arg == 1 and tri::is_bool_valued and tri::is_lsb_unused)) {
								// create a copy of the contextless compressed node
								auto nodec = reinterpret_cast<CND *>(&this->node_container_);
								nodec->compressed_node() = new raw::CompressedNode<depth_arg, tri>(*nodec->compressed_node());
							}
						},
						[]() { assert(false); });
			}
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
			return node_container_.hash_sized;
		}


		using traits = tr;

		using Key = typename tr::Key;
		using SliceKey = typename tr::SliceKey;
		using value_type = typename tr::value_type;

		[[nodiscard]] std::vector<size_t> getCards(const std::vector<pos_type> &positions) const;

		[[nodiscard]] size_t size() const{
			return compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg)  -> size_t {
					  const auto &node_container = *reinterpret_cast<const raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
					  if (node_container.isCompressed())
							return node_container.compressed_node()->size();
						else
							return node_container.uncompressed_node()->size();
					},
					[]() -> size_t { assert(false); return 0; });
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return this->node_container_.hash_sized == 0;
		}

		[[nodiscard]] value_type operator[](const Key &key) const {
			return compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg) mutable -> bool {
						RawKey<depth_arg> raw_key;
						std::copy_n(key.begin(), depth_arg, raw_key.begin());
						const auto &node_container = *reinterpret_cast<const raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
						return this->context()->rawContext().template get<depth_arg>(
								node_container,
								raw_key);
					},
					[]() -> bool { assert(false); return {}; });
		}



		[[nodiscard]]
		std::variant<std::optional<const_Hypertrie>, value_type> operator[](const SliceKey &slice_key) const {
			assert(slice_key.size() == depth());
			 const size_t slice_key_depth = tri::sliceKeyDepth(slice_key);

			if (slice_key_depth == depth()) {
				 Key key(slice_key.size());
				 for (auto [key_part, slice_key_part] : iter::zip(key, slice_key))
					 key_part = slice_key_part.value();
				 return this->operator[](key);
			 } else if (slice_key_depth == 0) {
				return *this;
			} else {
				std::optional<const_Hypertrie<tr>> result;
				compiled_switch<hypertrie_depth_limit, 1>::switch_void(
						this->depth_,
						[&](auto depth_arg) {
						  compiled_switch<depth_arg, 1>::switch_void(
								  slice_key_depth,
								  [&](auto slice_key_depth_arg) {
									RawSliceKey<slice_key_depth_arg> raw_slice_key(slice_key);

									const auto &node_container = *reinterpret_cast<const raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);

									auto [node_cont, is_managed] = this->context()->rawContext().template slice<depth_arg, slice_key_depth_arg>(node_container, raw_slice_key);
									if (is_managed)
										result = const_Hypertrie<tr>(depth_arg - slice_key_depth_arg, this->context(), {node_cont.hash().hash(), node_cont.node()});
									else
										result = const_Hypertrie<tr>(depth_arg - slice_key_depth_arg, nullptr, {node_cont.hash().hash(), node_cont.node()});
								  });

						});
				return result;
			}
		}

		using iterator = Iterator<tr>;
		using const_iterator = iterator;

		[[nodiscard]]
		iterator begin() const { return iterator{*this}; }

		[[nodiscard]]
		const_iterator cbegin() const { return iterator{*this}; }

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

		operator std::string() {
			std::vector<std::string> mappings;
			for (const auto &entry : *this){
				if constexpr (tr::is_bool_valued)
					mappings.push_back(fmt::format("⟨{}⟩ → true", fmt::join(entry,", ")));
				else
					mappings.push_back(fmt::format("⟨{}⟩ → {}", fmt::join(entry.first,", "), entry.second));
			}
			return fmt::format("[ {} ]", fmt::join(mappings, ", "));
		}

	};

	template<HypertrieTrait tr>
	class Hypertrie : public const_Hypertrie<tr>{
	public:
		using traits = tr;
		using tri = raw::Hypertrie_internal_t<tr>;
		using Key = typename tr::Key;
		using value_type = typename tr::value_type;

	private:
		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

	public:
		value_type set(const Key &key, value_type value) {
			return compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg) -> value_type {
						RawKey<depth_arg> raw_key;
						std::copy_n(key.begin(), depth_arg, raw_key.begin());
						auto &node_container = *reinterpret_cast<raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
						return this->context_->rawContext().template set<depth_arg>(node_container, raw_key, value);
					},
					[]() -> value_type { assert(false); return {}; });
		}

		Hypertrie(const const_Hypertrie<tr> &hypertrie) : const_Hypertrie<tr>(hypertrie) {
			compiled_switch<hypertrie_depth_limit, 1>::switch_void(
					this->depth_,
					[&](auto depth_arg){
					  	auto &typed_nodec = *reinterpret_cast<raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
						this->context_->rawContext().template incRefCount<depth_arg>(typed_nodec);}
					);
		}

		~Hypertrie() {
			compiled_switch<hypertrie_depth_limit, 1>::switch_void(
					this->depth_,
					[&](auto depth_arg){
						auto &typed_nodec = *reinterpret_cast<raw::NodeContainer<depth_arg, tri> *>(&this->node_container_);
						this->context_->rawContext().template decrRefCount<depth_arg>(typed_nodec);
					}
			);
		}

		Hypertrie(size_t depth = 1, HypertrieContext<tr> &context = DefaultHypertrieContext<tr>::instance())
			: const_Hypertrie<tr>(depth, &context) {}
	};

}

#endif //HYPERTRIE_HYPERTRIE_HPP
