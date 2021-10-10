#ifndef HYPERTRIE_HYPERTRIECONTEXT_HPP
#define HYPERTRIE_HYPERTRIECONTEXT_HPP


#include "Dice/hypertrie/HypertrieContextConfig.hpp"
#include "Dice/hypertrie/internal/Hypertrie_trait.hpp"
#include "Dice/hypertrie/internal/raw/node_context/RawHypertrieContext.hpp"
#include <memory>


namespace hypertrie {

	template<internal::HypertrieTrait tr_t>
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
		NodeContext &raw_context()  noexcept{
			return raw_context_;
		}
	};

	template<internal::HypertrieTrait tr>
	class DefaultHypertrieContext {
	public:
		static HypertrieContext<tr> &instance() {
			static const std::unique_ptr<HypertrieContext<tr>> instance{new HypertrieContext<tr>()};
			return *instance;
		}

		DefaultHypertrieContext(const DefaultHypertrieContext &) = delete;
		DefaultHypertrieContext &operator=(const DefaultHypertrieContext) = delete;

	protected:
		DefaultHypertrieContext() = default;
	};
}// namespace hypertrie

#endif//HYPERTRIE_HYPERTRIECONTEXT_HPP
