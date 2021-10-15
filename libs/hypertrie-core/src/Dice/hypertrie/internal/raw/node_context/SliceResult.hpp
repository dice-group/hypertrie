#ifndef HYPERTRIE_SLICERESULT_HPP
#define HYPERTRIE_SLICERESULT_HPP


#include <Dice/hypertrie/internal/raw/node/NodeContainer.hpp>

#include <utility>

namespace hypertrie::internal::raw {

	template<size_t result_depth, HypertrieCoreTrait tri_t>
	struct SliceResult {
		using tri = tri_t;

	private:
		union {
			NodeContainer<result_depth, tri> with_tri{};
			SENContainer<result_depth, tri_with_stl_alloc<tri>> with_stl;
		};

		bool uses_tri_alloc_ = true;
		bool managed_ = true;

		explicit SliceResult(const NodeContainer<result_depth, tri> &node_container) noexcept
			: uses_tri_alloc_(true), managed_(true) {
			this->with_tri = node_container;
		}

		SliceResult(const SENContainer<result_depth, tri_with_stl_alloc<tri>> &node_container, bool has_tri_alloc, bool managed) noexcept
			: uses_tri_alloc_(has_tri_alloc), managed_(managed) {
			this->with_stl = node_container;
		}

	public:
		SliceResult() noexcept {
			this->with_tri = {};
		};

		template<class... Args>
		static auto make_with_tri_alloc(Args &&...args) noexcept {
			return SliceResult(NodeContainer<result_depth, tri>{std::forward<Args>(args)...});
		}

		template<class... Args>
		static auto make_with_stl_alloc(bool managed, Args &&...args) noexcept {
			return SliceResult{
					SENContainer<result_depth, tri_with_stl_alloc<tri>>(std::forward<Args>(args)...),
					false,
					managed};
		}

		const NodeContainer<result_depth, tri> &get_with_tri_alloc() const noexcept {
			return this->with_tri;
		}

		const SENContainer<result_depth, tri_with_stl_alloc<tri>> &get_with_stl_alloc() const noexcept {
			return this->with_stl;
		}

		[[nodiscard]] bool is_managed() const noexcept {
			return managed_;
		}

		[[nodiscard]] bool uses_tri_alloc() const noexcept {
			return uses_tri_alloc_;
		}

		[[nodiscard]] bool empty() const noexcept {
			return this->with_tri.empty();
		}

		bool operator==(const SliceResult &rhs) const noexcept {
			return std::tie(with_tri, uses_tri_alloc_, managed_) == std::tie(rhs.with_tri, rhs.uses_tri_alloc_, rhs.managed_);
		}
		bool operator!=(const SliceResult &rhs) const noexcept {
			return !(rhs == *this);
		}
	};

}// namespace hypertrie::internal::raw
#endif//HYPERTRIE_SLICERESULT_HPP
