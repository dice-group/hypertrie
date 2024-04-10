#ifndef HYPERTRIE_OFFSETALLOCATOR_HPP
#define HYPERTRIE_OFFSETALLOCATOR_HPP

#include <boost/interprocess/offset_ptr.hpp>

template<typename T>
struct OffsetAllocator {
	using value_type = T;
	template<typename P>
	using offset_ptr = boost::interprocess::offset_ptr<P>;
	using pointer = offset_ptr<value_type>;
	using const_pointer = offset_ptr<const value_type>;
	using void_pointer = offset_ptr<void>;
	using const_void_pointer = offset_ptr<const void>;
	using difference_type = typename offset_ptr<value_type>::difference_type;

	OffsetAllocator() noexcept = default;
	OffsetAllocator(OffsetAllocator const &) noexcept = default;
	OffsetAllocator(OffsetAllocator &&) noexcept = default;
	OffsetAllocator &operator=(OffsetAllocator const &) noexcept = default;
	OffsetAllocator &operator=(OffsetAllocator &&) noexcept = default;
	template<typename V>
	OffsetAllocator(OffsetAllocator<V>) noexcept {}

	pointer allocate(std::size_t n) {
		return pointer(static_cast<T *>(::operator new(n * sizeof(T))));
	}
	void deallocate(pointer p, std::size_t) noexcept {
		::operator delete(p.get());
	}
	friend bool operator==(OffsetAllocator const &, OffsetAllocator const &) noexcept {
		return true;
	}
	friend bool operator!=(OffsetAllocator const &l, OffsetAllocator const &r) noexcept {
		return !(l == r);
	}
};

#endif//HYPERTRIE_OFFSETALLOCATOR_HPP
