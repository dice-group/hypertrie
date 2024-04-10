#ifndef HYPERTRIE_HYPERTRIECONTEXT_HPP
#define HYPERTRIE_HYPERTRIECONTEXT_HPP


#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/HypertrieContextConfig.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/hypertrie_allocator_trait.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"

#include <memory>


namespace dice::hypertrie {

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class HypertrieContext {
	private:
		static constexpr size_t max_depth_ = hypertrie_max_depth;

		using ht_alloc_trait_t = hypertrie_allocator_trait<allocator_type>;

	public:
		using RawHypertrieContext_t = typename internal::raw::template RawHypertrieContext<max_depth_, htt_t, allocator_type>;

	private:
		typename ht_alloc_trait_t::template pointer<RawHypertrieContext_t> raw_context_;

		allocator_type alloc_;

		using ht_ctx_alloc_type = typename ht_alloc_trait_t::template rebind_alloc<RawHypertrieContext_t>;

	public:
		HypertrieContext() = delete;
		HypertrieContext(HypertrieContext const &) = delete;
		HypertrieContext(HypertrieContext &&) = delete;
		HypertrieContext &operator=(HypertrieContext const &) = delete;
		HypertrieContext &operator=(HypertrieContext &&) = delete;

		explicit HypertrieContext(allocator_type const &alloc)
			: raw_context_([&]() {
				  ht_ctx_alloc_type alloc_ht_ctx = alloc;
				  auto ht_ctx_ptr = std::allocator_traits<ht_ctx_alloc_type>::allocate(alloc_ht_ctx, 1);
				  std::allocator_traits<ht_ctx_alloc_type>::construct(alloc_ht_ctx, std::to_address(ht_ctx_ptr), alloc);
				  return ht_ctx_ptr;
			  }()),
			  alloc_(alloc) {}

		~HypertrieContext() {
			ht_ctx_alloc_type alloc_ht_ctx = alloc_;
			std::allocator_traits<ht_ctx_alloc_type>::destroy(alloc_ht_ctx, std::to_address(raw_context_));
			std::allocator_traits<ht_ctx_alloc_type>::deallocate(alloc_ht_ctx, std::to_address(raw_context_), 1);
		}

		[[nodiscard]] constexpr size_t max_depth() const noexcept {
			return max_depth_;
		}
		RawHypertrieContext_t &raw_context() noexcept {
			return *raw_context_;
		}
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires hypertrie_allocator_trait<allocator_type>::is_stl_alloc
	class DefaultHypertrieContext {
	public:
		static HypertrieContext<htt_t, allocator_type> &instance() {
			static std::unique_ptr<HypertrieContext<htt_t, allocator_type>> const instance{new HypertrieContext<htt_t, allocator_type>{std::allocator<std::byte>{}}};
			return *instance;
		}

		DefaultHypertrieContext(DefaultHypertrieContext const &) = delete;
		DefaultHypertrieContext(DefaultHypertrieContext &&) = delete;
		DefaultHypertrieContext &operator=(DefaultHypertrieContext const &) = delete;
		DefaultHypertrieContext &operator=(DefaultHypertrieContext &&) = delete;
		~DefaultHypertrieContext() = default;

	protected:
		DefaultHypertrieContext() = default;
	};

	template<HypertrieTrait tr, ByteAllocator allocator_type>
	using HypertrieContext_ptr = typename hypertrie_allocator_trait<allocator_type>::template pointer<HypertrieContext<tr, allocator_type>>;
}// namespace dice::hypertrie

#endif//HYPERTRIE_HYPERTRIECONTEXT_HPP
