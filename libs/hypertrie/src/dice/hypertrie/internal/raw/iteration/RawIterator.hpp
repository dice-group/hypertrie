#ifndef HYPERTRIE_RAWITERATOR_HPP
#define HYPERTRIE_RAWITERATOR_HPP

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/RawDiagonalPositions.hpp"
#include "dice/hypertrie/internal/raw/node/FullNode.hpp"
#include "dice/hypertrie/internal/raw/node/NodeContainer.hpp"
#include "dice/hypertrie/internal/raw/node/SingleEntryNode.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"
#include "dice/hypertrie/internal/raw/node_context/SliceResult.hpp"
#include "dice/template-library/integral_template_tuple.hpp"
#include "dice/template-library/switch_cases.hpp"

namespace dice::hypertrie::internal::raw {

	template<size_t depth, bool use_raw_key, HypertrieTrait htt_t, ByteAllocator allocator_type, size_t context_max_depth>
	class RawIterator {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = std::conditional_t<use_raw_key, SingleEntry<depth, htt_t>, NonZeroEntry<htt_t>>;

	private:
		template<size_t depth2>
		using child_iterator = typename FullNode<depth2, htt_t, allocator_type>::ChildrenType::const_iterator;

		value_type value_ = []() { if constexpr (use_raw_key) return value_type{}; else return value_type(depth); }();
		RawHypertrieContext<context_max_depth, htt_t, allocator_type> const *context_;
		template_library::integral_template_tuple<1ul, depth, child_iterator> iters_;
		template_library::integral_template_tuple<1ul, depth, child_iterator> ends_;
		uint32_t active_iter_ = 0;
		bool ended_ = true;

		template<size_t child_depth>
		[[nodiscard]] child_iterator<child_depth> &get_iter() { return iters_.template get<child_depth>(); }
		template<size_t child_depth>
		[[nodiscard]] child_iterator<child_depth> &get_end() { return ends_.template get<child_depth>(); }

		[[nodiscard]] bool is_sen() const noexcept {
			return active_iter_ == 0;
		}

	public:
		RawIterator() = default;
		RawIterator(FNContainer<depth, htt_t, allocator_type> const &nodec, RawHypertrieContext<context_max_depth, htt_t, allocator_type> const &context) noexcept
			: context_(&context), ended_(nodec.empty()) {
			if (not this->ended_) {
				init_fn_rek<depth>(nodec);
				assert(not is_sen());
			}
		}

		template<ByteAllocator allocator_type2>
		explicit RawIterator(SENContainer<depth, htt_t, allocator_type2> const &nodec) noexcept : ended_(nodec.empty()) {
			if (not ended_) {
				if constexpr (depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
					// set key_part stored in the identifier
					value_.key()[depth - 1] = nodec.raw_identifier().get_entry().key()[0];
				} else {
					// set rest of the key (and value) with the data in the compressed node
					auto sen_ptr = nodec.node_ptr();
					const auto &sen_key = sen_ptr->key();
					// copy the key of the compressed child node to the iterator key
					std::copy(sen_key.cbegin(), sen_key.cend(), value_.key().begin());
					if constexpr (not HypertrieTrait_bool_valued<htt_t>)
						value_.value() = sen_ptr->value();
				}
			}
		}

		RawIterator(NodeContainer<depth, htt_t, allocator_type> const &nodec, RawHypertrieContext<context_max_depth, htt_t, allocator_type> const &context) noexcept
			: RawIterator((nodec.is_sen()) ? RawIterator(util::unsafe_cast<SENContainer<depth, htt_t, allocator_type> const>(nodec))
										   : RawIterator(util::unsafe_cast<FNContainer<depth, htt_t, allocator_type> const>(nodec), context)) {}

		[[nodiscard]] bool ended() const noexcept {
			return ended_;
		}

		operator bool() const noexcept {
			return not ended_;
		}

		[[nodiscard]] value_type const &value() const noexcept {
			return value_;
		}

		void inc() noexcept {
			if (is_sen()) {
				ended_ = true;
			} else
				template_library::switch_cases<1, depth + 1>(active_iter_,
												 [&](auto active_depth) {
													 this->template inc_rek<active_depth>();
												 });
		}

	protected:
		template<size_t current_depth>
		void init_fn_rek(FNContainer<current_depth, htt_t, allocator_type> const &nodec) noexcept {
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
					auto child_fn_ptr = context_->node_storage_.template lookup<current_depth - 1, FullNode>(iter->second);
					this->template init_fn_rek<current_depth - 1>(FNContainer<current_depth - 1, htt_t, allocator_type>{iter->second, child_fn_ptr});
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
		void write_compressed() noexcept {
			static_assert(node_depth < depth);

			// child is compressed
			if constexpr (node_depth == 1 and HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
				auto &iter = get_iter<node_depth + 1>();
				auto identifier = iter->second;
				// set key_part stored in TaggedTensorHash
				value_.key()[0] = identifier.get_entry().key()[0];
			} else {
				// set rest of the key (and value) with the data in the compressed node
				auto &iter = get_iter<node_depth + 1>();
				auto sen_ptr = context_->node_storage_.template lookup<node_depth, SingleEntryNode>(iter->second);
				const auto &sen_key = sen_ptr->key();
				// copy the key of the compressed child node to the iterator key
				std::copy(sen_key.cbegin(), sen_key.cend(), value_.key().begin());
				if constexpr (not HypertrieTrait_bool_valued<htt_t>)
					value_.value() = sen_ptr->value();
			}
		}

		/**
		 * Write the UncompressedNode to the key (and evtl. value) currently pointed at by std::get<child_depth>(iters)
		 * @tparam child_depth the node depth of the UncompressedNode to be written
		 */
		template<size_t node_depth>
		void write_uncompressed_mapped_key_part() noexcept {
			auto &iter = get_iter<node_depth>();

			// set the key at the corresponding position
			auto key_part = [&]() {
				if constexpr (node_depth == 1 and HypertrieTrait_bool_valued<htt_t>)
					return *iter;
				else
					return iter->first;
			}();

			value_.key()[node_depth - 1] = key_part;

			// write value
			if constexpr (node_depth == 1 and not HypertrieTrait_bool_valued<htt_t>)
				value_.value() = iter->second;
		}


		template<pos_type current_depth>
		void inc_rek() noexcept {
			active_iter_ = current_depth;
			auto &iter = get_iter<current_depth>();
			auto &end = get_end<current_depth>();

			++iter;
			if (iter != end) {
				write_uncompressed_mapped_key_part<current_depth>();
				if constexpr (current_depth > 1) {
					if (iter->second.is_fn()) {
						// child is fn, go on recursively
						auto child_fn_ptr = context_->node_storage_.template lookup<current_depth - 1, FullNode>(iter->second);
						this->template init_fn_rek<current_depth - 1>(FNContainer<current_depth - 1, htt_t, allocator_type>{iter->second, child_fn_ptr});
					} else {
						// child is compressed
						write_compressed<current_depth - 1>();
					}
				}
			} else {
				if constexpr (current_depth == depth)
					ended_ = true;
				else
					this->template inc_rek<current_depth + 1>();
			}
		}
	};

};// namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_RAWITERATOR_HPP
