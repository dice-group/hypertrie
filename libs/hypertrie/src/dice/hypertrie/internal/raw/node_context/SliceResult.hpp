#ifndef HYPERTRIE_SLICERESULT_HPP
#define HYPERTRIE_SLICERESULT_HPP


#include "dice/hypertrie/internal/raw/node/NodeContainer.hpp"

#include <utility>

namespace dice::hypertrie::internal::raw {

	template<size_t result_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct SliceResult {

	private:
		using ht_allocator_trait = hypertrie_allocator_trait<allocator_type>;
		using ContainerType = std::conditional_t<ht_allocator_trait::is_stl_alloc,
												 // tri allocator and stl allocator are the same (std::allocator<std::byte>)
												 std::variant<SENContainer<result_depth, htt_t, allocator_type>,
															  FNContainer<result_depth, htt_t, allocator_type>>,
												 // tri allocator is not std::allocator<std::byte>
												 // -> two types of single entry nodes
												 std::variant<SENContainer<result_depth, htt_t, allocator_type>,
															  FNContainer<result_depth, htt_t, allocator_type>,
															  SENContainer<result_depth, htt_t, std::allocator<std::byte>>>>;

		ContainerType container_;

		bool uses_provided_alloc_ = true;
		bool managed_ = true;

		explicit SliceResult(const FNContainer<result_depth, htt_t, allocator_type> &fn_container) noexcept
			: container_{fn_container}, uses_provided_alloc_(true), managed_(true) {}

		explicit SliceResult(const SENContainer<result_depth, htt_t, allocator_type> &sen_container) noexcept
			: container_{sen_container}, uses_provided_alloc_(true), managed_(true) {}


		/**
		 * Assumes a managed node_container that uses tri_alloc.
		 * @param fn_container
		 */
		explicit SliceResult(const NodeContainer<result_depth, htt_t, allocator_type> &node_container) noexcept
			: uses_provided_alloc_(true), managed_(true) {
			if (node_container.is_fn()) // TODO: do we really need the FN/SEN constructor here?
				container_.template emplace<FNContainer<result_depth, htt_t, allocator_type>>(FNContainer<result_depth, htt_t, allocator_type>(node_container));
			else
				container_.template emplace<SENContainer<result_depth, htt_t, allocator_type>>(SENContainer<result_depth, htt_t, allocator_type>(node_container));
		}

		template<typename = void>
		SliceResult(const SENContainer<result_depth, htt_t, std::allocator<std::byte>> &sen_container, bool has_tri_alloc, bool managed) noexcept
			: container_{sen_container}, uses_provided_alloc_(has_tri_alloc), managed_(managed) {}

	public:
		SliceResult() = default;

		template<class... Args>
		static auto make_with_provided_alloc(Args &&...args) noexcept {
			return SliceResult(NodeContainer<result_depth, htt_t, allocator_type>{std::forward<Args>(args)...});
		}

		template<class... Args>
		static auto make_sen_with_stl_alloc(bool managed, Args &&...args) noexcept {
			return SliceResult{
					SENContainer<result_depth, htt_t, std::allocator<std::byte>>(std::forward<Args>(args)...),
					false,
					managed};
		}

		[[nodiscard]] RawNodeContainer<htt_t, allocator_type> get_raw_nodec() const noexcept {
			RawNodeContainer<htt_t, allocator_type> result_nodec;
			std::visit([&result_nodec](auto &&nodec) { result_nodec = nodec; }, container_);
			return result_nodec;
		};

		[[nodiscard]] bool holds_fn() const noexcept {
			return std::holds_alternative<FNContainer<result_depth, htt_t, allocator_type>>(container_);
		}

		[[nodiscard]] bool holds_sen() const noexcept {
			return std::holds_alternative<SENContainer<result_depth, htt_t, allocator_type>>(container_);
		}

		[[nodiscard]] bool holds_stl_alloc_sen() const noexcept {
			return std::holds_alternative<SENContainer<result_depth, htt_t, std::allocator<std::byte>>>(container_);
		}

		[[nodiscard]] FNContainer<result_depth, htt_t, allocator_type> const &get_fn() const noexcept {
			return std::get<FNContainer<result_depth, htt_t, allocator_type>>(container_);
		}
		[[nodiscard]] FNContainer<result_depth, htt_t, allocator_type> &get_fn() noexcept {
			return std::get<FNContainer<result_depth, htt_t, allocator_type>>(container_);
		}

		[[nodiscard]] SENContainer<result_depth, htt_t, allocator_type> const &get_sen() const noexcept {
			return std::get<SENContainer<result_depth, htt_t, allocator_type>>(container_);
		}
		[[nodiscard]] SENContainer<result_depth, htt_t, allocator_type> &get_sen() noexcept {
			return std::get<SENContainer<result_depth, htt_t, allocator_type>>(container_);
		}

		[[nodiscard]] SENContainer<result_depth, htt_t, std::allocator<std::byte>> const &get_stl_alloc_sen() const noexcept {
			return std::get<SENContainer<result_depth, htt_t, std::allocator<std::byte>>>(container_);
		}
		[[nodiscard]] SENContainer<result_depth, htt_t, std::allocator<std::byte>> &get_stl_alloc_sen() noexcept {
			return std::get<SENContainer<result_depth, htt_t, std::allocator<std::byte>>>(container_);
		}

		[[nodiscard]] bool is_managed() const noexcept {
			return managed_;
		}

		[[nodiscard]] bool uses_provided_alloc() const noexcept {
			return uses_provided_alloc_;
		}

		[[nodiscard]] bool empty() const noexcept {
			if (container_.index() != std::variant_npos) {
				return std::visit([](auto &&nodec) { return nodec.empty(); }, container_);
			}
			return true;
		}
	};

}// namespace dice::hypertrie::internal::raw
#endif//HYPERTRIE_SLICERESULT_HPP
