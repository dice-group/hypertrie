#ifndef HYPERTRIE_SIZED_HPP
#define HYPERTRIE_SIZED_HPP

#include <cstddef>

namespace dice::hypertrie::internal::raw {

	struct Sized {
	protected:
		size_t size_ = 0;

	public:
		constexpr Sized() noexcept = default;
		explicit constexpr Sized(size_t size) noexcept : size_{size} {}

		[[nodiscard]] constexpr size_t &size() noexcept {
			return size_;
		}

		[[nodiscard]] constexpr size_t size() const noexcept {
			return size_;
		}
	};

}  //namespace dice::hypertrie::internal::raw

#endif//HYPERTRIE_SIZED_HPP
