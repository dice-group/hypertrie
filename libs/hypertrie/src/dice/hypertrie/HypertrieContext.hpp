#ifndef HYPERTRIE_HYPERTRIECONTEXT_HPP
#define HYPERTRIE_HYPERTRIECONTEXT_HPP


#include "dice/hypertrie/ByteAllocator.hpp"
#include "dice/hypertrie/HypertrieContextConfig.hpp"
#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"

#include <memory>

namespace dice::hypertrie {

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class HypertrieContext {
	public:
		using RawHypertrieContext_t = internal::raw::RawHypertrieContext<hypertrie_max_depth, htt_t, allocator_type>;

	private:
		using context_allocator_traits = typename std::allocator_traits<allocator_type>::template rebind_traits<RawHypertrieContext_t>;
		using context_allocator_type = typename context_allocator_traits::allocator_type;

		typename context_allocator_traits::pointer raw_context_;
		[[no_unique_address]] context_allocator_type alloc_;

	public:
		HypertrieContext() = delete;
		HypertrieContext(HypertrieContext const &) = delete;
		HypertrieContext(HypertrieContext &&) = delete;
		HypertrieContext &operator=(HypertrieContext const &) = delete;
		HypertrieContext &operator=(HypertrieContext &&) = delete;

		explicit HypertrieContext(allocator_type const &alloc) : alloc_{alloc} {
			raw_context_ = context_allocator_traits::allocate(alloc_, 1);
			new (std::to_address(raw_context_)) RawHypertrieContext_t{alloc_};
		}

		~HypertrieContext() {
			raw_context_->~RawHypertrieContext();
			context_allocator_traits::deallocate(alloc_, raw_context_, 1);
		}

		[[nodiscard]] constexpr size_t max_depth() const noexcept {
			return hypertrie_max_depth;
		}

		RawHypertrieContext_t &raw_context() noexcept {
			return *raw_context_;
		}

		allocator_type get_allocator() const noexcept {
			return alloc_;
		}
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
		requires std::is_default_constructible_v<allocator_type>
	class DefaultHypertrieContext {
	protected:
		DefaultHypertrieContext() = default;

	public:
		static HypertrieContext<htt_t, allocator_type> &instance() {
			static HypertrieContext<htt_t, allocator_type> instance{allocator_type{}};
			return instance;
		}

		DefaultHypertrieContext(DefaultHypertrieContext const &) = delete;
		DefaultHypertrieContext(DefaultHypertrieContext &&) = delete;
		DefaultHypertrieContext &operator=(DefaultHypertrieContext const &) = delete;
		DefaultHypertrieContext &operator=(DefaultHypertrieContext &&) = delete;
		~DefaultHypertrieContext() = default;
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	using HypertrieContext_ptr = typename std::allocator_traits<allocator_type>::template rebind_traits<HypertrieContext<htt_t, allocator_type>>::pointer;

}// namespace dice::hypertrie

#endif//HYPERTRIE_HYPERTRIECONTEXT_HPP
