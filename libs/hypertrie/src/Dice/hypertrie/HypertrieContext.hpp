#ifndef HYPERTRIE_HYPERTRIECONTEXT_HPP
#define HYPERTRIE_HYPERTRIECONTEXT_HPP


#include "Dice/hypertrie/HypertrieContextConfig.hpp"
#include "Dice/hypertrie/Hypertrie_trait.hpp"
#include "Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"
#include <memory>


namespace Dice::hypertrie {

	template<HypertrieTrait tr_t>
	class HypertrieContext {
	private:
		static constexpr const size_t max_depth_ = hypertrie_max_depth;

	public:
		using tr = tr_t;
		using NodeContext = typename internal::raw::template RawHypertrieContext<max_depth_, typename internal::raw::template Hypertrie_core_t<tr>>;
		using allocator_type = typename tr::allocator_type;

	private:
		NodeContext raw_context_{};

	public:
		explicit HypertrieContext(allocator_type const &alloc) : raw_context_(alloc) {}

		[[nodiscard]] constexpr size_t max_depth() const noexcept {
			return max_depth_;
		}
		NodeContext &raw_context() noexcept {
			return raw_context_;
		}
	};

	template<HypertrieTrait tr>
	class DefaultHypertrieContext {
	public:
		static HypertrieContext<tr> &instance() {
			static const std::unique_ptr<HypertrieContext<tr>> instance{new HypertrieContext<tr>{typename tr::allocator_type()}};
			return *instance;
		}

		DefaultHypertrieContext(const DefaultHypertrieContext &) = delete;
		DefaultHypertrieContext &operator=(const DefaultHypertrieContext) = delete;

	protected:
		DefaultHypertrieContext() = default;
	};

	template<HypertrieTrait tr_t>
	class TaggedHypertrieContextPtr {
	public:
		using tr = tr_t;

	private:
		std::uintptr_t raw_{};

		static inline std::uintptr_t encode(HypertrieContext<tr> *ctx_ptr, bool is_managed) noexcept {
			return std::uintptr_t(ctx_ptr) | (std::uintptr_t(not is_managed) << 63);
		}

		HypertrieContext<tr> *ptr() const noexcept {
			return reinterpret_cast<HypertrieContext<tr> *>(raw_ & ~(std::uintptr_t(1) << 63));
		}

	public:
		TaggedHypertrieContextPtr() = default;

		TaggedHypertrieContextPtr(HypertrieContext<tr> *ctx, bool is_managed = true) : raw_(encode(ctx, is_managed)) {}
		HypertrieContext<tr> &operator*() const noexcept {
			return *ptr();
		}

		[[nodiscard]] bool is_managed() const noexcept {
			return not(raw_ & (std::uintptr_t(1) << 63));
		}

		operator HypertrieContext<tr> *() const noexcept {
			return ptr();
		}
	};
}// namespace Dice::hypertrie

#endif//HYPERTRIE_HYPERTRIECONTEXT_HPP
