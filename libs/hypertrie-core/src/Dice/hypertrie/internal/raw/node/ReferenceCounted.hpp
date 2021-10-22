#ifndef HYPERTRIE_REFERENCECOUNTED_HPP
#define HYPERTRIE_REFERENCECOUNTED_HPP

#include <cstddef>
namespace Dice::hypertrie::internal::raw {

	/**
	 * A super class to provide counting references in a Node.
	 */
	struct ReferenceCounted {
	protected:
		size_t ref_count_ = 0;

	public:
		/**
		 * Default constructor. ref_count is set to 0.
		 */
		ReferenceCounted() = default;

		/**
		 * ref_count is set to the given value.
		 * @param ref_count ref_count value
		 */
		explicit ReferenceCounted(size_t ref_count) noexcept : ref_count_(ref_count) {}
		/**
		 * Modifiable reference to ref_count.
		 * @return
		 */
		constexpr size_t &ref_count() noexcept { return this->ref_count_; }

		/**
		 * Constant reference to ref_count.
		 * @return
		 */
		[[nodiscard]] const size_t &ref_count() const noexcept { return this->ref_count_; }
	};

}// namespace Dice::hypertrie::internal::raw
#endif//HYPERTRIE_REFERENCECOUNTED_HPP
