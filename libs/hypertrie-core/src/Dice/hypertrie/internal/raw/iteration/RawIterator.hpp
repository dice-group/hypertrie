#ifndef HYPERTRIE_RAWITERATOR_HPP
#define HYPERTRIE_RAWITERATOR_HPP

#include <Dice/hypertrie/internal/raw/Hypertrie_core_trait.hpp>
#include <Dice/hypertrie/internal/raw/RawDiagonalPositions.hpp>
#include <Dice/hypertrie/internal/raw/node/FullNode.hpp>
#include <Dice/hypertrie/internal/raw/node/NodeContainer.hpp>
#include <Dice/hypertrie/internal/raw/node/SingleEntryNode.hpp>
#include <Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp>
#include <Dice/hypertrie/internal/raw/node_context/SliceResult.hpp>
#include <Dice/hypertrie/internal/util/SwitchTemplateFunctions.hpp>
namespace hypertrie::internal::raw {

	// TODO: parameter for switching between raw-key and key
	// TODO: support for iterating contextless sen
	template<size_t depth, template<size_t, typename> typename node_type, HypertrieCoreTrait tri, size_t context_max_depth>
	class RawIterator;

	template<size_t depth, HypertrieCoreTrait tri_t, size_t context_max_depth>
	class RawIterator<depth, FullNode, tri_t, context_max_depth> {
	public:
		using tri = tri_t;
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;

	private:
		template<size_t depth2>
		using child_iterator = typename FullNode<depth2, tri>::ChildrenType::const_iterator;


		SingleEntryNode<depth, tri_with_stl_alloc<tri>> value_;
		RawHypertrieContext<context_max_depth, tri> const *context_;
		util::IntegralTemplatedTuple<child_iterator, 1, depth> iters_;
		util::IntegralTemplatedTuple<child_iterator, 1, depth> ends_;
		uint32_t active_iter_ = 0;
		bool ended_ = true;

		template<size_t child_depth>
		child_iterator<child_depth> &get_iter() { return iters_.template get<child_depth + 1>(); }
		template<size_t child_depth>
		child_iterator<child_depth> &get_end() { return ends_.template get<child_depth + 1>(); }

		[[nodiscard]] bool is_sen() const noexcept {
			return active_iter_ == 0;
		}

	public:
		RawIterator() = default;
		RawIterator(FNContainer<depth, tri> const &nodec, RawHypertrieContext<context_max_depth, tri> const &context)
			: context_(&context), ended_(nodec.empty()) {
			if (not this->ended_) {
				init_fn_rek<depth>(nodec);
				assert(not is_sen());
			}
		}

		explicit RawIterator(SENContainer<depth, tri> const &nodec) : ended_(nodec.empty()) {
			if (not ended_) {
				if constexpr (depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) {
					// set key_part stored in the identifier
					value_.key()[depth - 1] = nodec.raw_identifier().get_entry().key()[0];
				} else {
					// set rest of the key (and value) with the data in the compressed node
					auto sen_ptr = nodec.node_ptr();
					const auto &sen_key = sen_ptr->key();
					// copy the key of the compressed child node to the iterator key
					std::copy(sen_key.cbegin(), sen_key.cend(), value_.key().begin());
					if constexpr (not HypertrieCoreTrait_bool_valued<tri>)
						value_.value() = sen_ptr->value();
				}
			}
		}

		RawIterator(NodeContainer<depth, tri> const &nodec, RawHypertrieContext<context_max_depth, tri> const &context)
			: RawIterator((nodec.is_sen()) ? RawIterator(util::unsafe_cast<SENContainer<depth, tri>>(nodec))
										   : RawIterator(util::unsafe_cast<FNContainer<depth, tri>>(nodec), context)) {}

		[[nodiscard]] bool ended() const noexcept {
			return ended_;
		}

		operator bool() const noexcept {
			return not ended_;
		}

		auto &value() const noexcept {
			return value_;
		}

		inline void inc() {
			if (is_sen()) {
				ended_ = true;
			} else
				util::switch_cases<1, depth + 1>(active_iter_,
												 [&](auto active_depth) {
													 this->template inc_rek<active_depth>();
												 });
		}

	protected:
		template<size_t current_depth>
		void init_fn_rek(FNContainer<depth, tri> const &nodec) {
			active_iter_ = current_depth;

			const auto fn_ptr = nodec.node_ptr();

			auto &iter = get_iter<current_depth>();
			auto &end = get_end<current_depth>();
			iter = fn_ptr->edges(current_depth - 1).cbegin();
			end = fn_ptr->edges(current_depth - 1).cend();

			write_uncompressed_mapped_key_part<current_depth>();
			if constexpr (current_depth > 1) {
				if (iter->second.is_fn()) {
					// child is fn, go on recursively
					auto child_fn_ptr = context_->node_storage_.template lookup<depth, FullNode>(iter->second);
					this->init_fn_rek<current_depth - 1>(FNContainer<current_depth - 1, tri>{iter->second, child_fn_ptr});
				} else {
					// child is compressed
					write_compressed<current_depth - 1>();
				}
			}
		}

		/**
		 * Write the CompressedNode to the key and value currently pointed at by std::get<node_depth>(iters)
		 * @tparam node_depth the node depth of the CompressedNode to be written
		 */
		template<size_t node_depth>
		inline void write_compressed() noexcept {
			static_assert(node_depth < depth);

			// child is compressed
			if constexpr (node_depth == 1 and HypertrieCoreTrait_bool_valued_and_taggable_key_part<tri>) {
				auto &iter = get_iter<node_depth + 1>();
				auto identifier = iter->second;
				// set key_part stored in TaggedTensorHash
				value_.key()[0] = identifier.get_entry().key()[0];
			} else {
				// set rest of the key (and value) with the data in the compressed node
				auto &iter = get_iter<node_depth + 1>();
				auto sen_ptr = context_->node_storage_.template lookup<depth, SingleEntryNode>(iter->second);
				const auto &sen_key = sen_ptr->key();
				// copy the key of the compressed child node to the iterator key
				std::copy(sen_key.cbegin(), sen_key.cend(), value_.key().cbegin());
				if constexpr (not HypertrieCoreTrait_bool_valued<tri>)
					value_.value() = sen_ptr->value();
			}
		}

		/**
		 * Write the UncompressedNode to the key (and evtl. value) currently pointed at by std::get<child_depth>(iters)
		 * @tparam child_depth the node depth of the UncompressedNode to be written
		 */
		template<size_t node_depth>
		inline void write_uncompressed_mapped_key_part() noexcept {
			auto &iter = get_iter<node_depth>();

			// set the key at the corresponding position
			auto key_part = [&]() {
				if constexpr (node_depth == 0 and HypertrieCoreTrait_bool_valued<tri>)
					return *iter;
				else
					return iter->first;
			}();

			value_.key()[node_depth - 1] = key_part;

			// write value
			if constexpr (node_depth == 1 and not HypertrieCoreTrait_bool_valued<tri>)
				value_.value() = iter->second;
		}


		template<pos_type current_depth>
		inline bool inc_rek() {
			active_iter_ = current_depth;
			auto &iter = get_iter<current_depth>();
			auto &end = get_end<current_depth>();

			++iter;
			if (iter != end) {
				write_uncompressed_mapped_key_part<current_depth>();
				if constexpr (current_depth > 1) {
					if (iter->second.is_fn()) {
						// child is fn, go on recursively
						auto child_fn_ptr = context_->node_storage_.template lookup<depth, FullNode>(iter->second);
						this->init_fn_rek<current_depth - 1>(FNContainer<current_depth - 1, tri>{iter->second, child_fn_ptr});
					} else {
						// child is compressed
						write_compressed<current_depth - 1>();
					}
				}
			} else {
				if (current_depth == depth)
					ended_ = true;
				else
					this->template inc_rek<current_depth + 1>();
			}
		}
	};

};// namespace hypertrie::internal::raw

#endif//HYPERTRIE_RAWITERATOR_HPP
