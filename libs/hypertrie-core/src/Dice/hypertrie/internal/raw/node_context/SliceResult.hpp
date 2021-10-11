#ifndef HYPERTRIE_SLICERESULT_HPP
#define HYPERTRIE_SLICERESULT_HPP


#include <Dice/hypertrie/internal/raw/node/NodeContainer.hpp>
#include <Dice/hypertrie/internal/util/UnsafeCast.hpp>

#include <bit>
#include <utility>

namespace hypertrie::internal::raw {

	template<size_t result_depth, HypertrieCoreTrait tri_t>
	struct SliceResult {
		using tri = tri_t;

	private:
		NodeContainer<result_depth, tri> node_container_;

		bool uses_tri_alloc_ = true;
		bool managed_ = true;

		SliceResult(const NodeContainer<result_depth, tri> &nodeContainer, bool hasTriAlloc, bool managed) noexcept
			: node_container_(nodeContainer), uses_tri_alloc_(hasTriAlloc), managed_(managed) {}

	public:
		SliceResult() = default;

		template<class... Args>
		static auto make_with_tri_alloc(Args &&...args) noexcept {
			return SliceResult(NodeContainer<result_depth, tri>{std::forward<Args...>(args...)}, true, true);
		}

		template<class... Args>
		static auto make_with_stl_alloc(bool managed, Args &&...args) noexcept {
			return SliceResult{
					std::bit_cast<NodeContainer<result_depth, tri>>(NodeContainer<result_depth, tri_with_stl_alloc<tri>>(std::forward<Args...>(args...)),
					false,
					managed)};
		}

		const NodeContainer<result_depth, tri> &get_with_tri_alloc() const noexcept {
			return node_container_;
		}

		const NodeContainer<result_depth, tri_with_stl_alloc<tri>> &get_with_stl_alloc() const noexcept {
			return util::unsafe_cast<NodeContainer<result_depth, tri_with_stl_alloc<tri>> const>(node_container_);
		}

		[[nodiscard]] bool is_managed() const noexcept {
			return managed_;
		}

		[[nodiscard]] bool uses_tri_alloc() const noexcept {
			return uses_tri_alloc_;
		}

		[[nodiscard]] bool empty() const noexcept {
			return node_container_.empty();
		}
	};

}// namespace hypertrie::internal::raw
#endif//HYPERTRIE_SLICERESULT_HPP
