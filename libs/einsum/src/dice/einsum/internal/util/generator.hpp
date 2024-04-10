#ifndef HYPERTRIE_GENERATOR_HPP
#define HYPERTRIE_GENERATOR_HPP

////////////////////////////////////////////////////////////////
// Reference implementation of std::generator proposal P2168R3.
// see https://github.com/RishabhRD/generator
// works with: (clang-14 + libstdc++-11), (gcc-13 + libstdc++-11)
// not yet tested: (clang-14 + libc++-14)
// working except exception handling: (clang-13 + libstdc++-11)
// not working: (clang-13 + libc++-13)
//

#if !__has_include(<generator>)

#ifdef __clang__
#if __clang_major__ <= 13
// Workaround for clang at least <=13. see https://bugs.llvm.org/show_bug.cgi?id=48172
// exceptions do not work
#define __cpp_impl_coroutine 1
namespace std::experimental {
	using namespace std;
}
#endif
#endif

#if __has_include(<coroutine>)
#include <coroutine>
#else
#include <experimental/coroutine>
namespace std {
	using std::experimental::coroutine_handle;
	using std::experimental::noop_coroutine;
	using std::experimental::suspend_always;
	using std::experimental::suspend_never;
}// namespace std
#endif

#include <concepts>
#include <exception>
#include <iterator>
#include <new>
#include <type_traits>
#include <utility>
#if __has_include(<ranges>)
#include <ranges>
#else
// Placeholder implementation of the bits we need from <ranges> header
// when we don't have the <ranges> header.
namespace std {

	template<typename T>
	using iter_reference_t = decltype(*std::declval<T &>());

	template<typename T>
	using iter_value_t =
			typename std::iterator_traits<std::remove_cvref_t<T>>::value_type;

	struct default_sentinel_t {};

	namespace ranges {
		namespace __begin {
			void begin();

			struct _fn {
				template<typename Range>
				requires requires(Range &r) {
					r.begin();
				}
				auto operator()(Range &&r) const noexcept(noexcept(r.begin()))
						-> decltype(r.begin()) {
					return r.begin();
				}

				template<typename Range>
				requires(!requires(Range & r) { r.begin(); }) && requires(Range &r) {
					begin(r);
				}
				auto operator()(Range &&r) const noexcept(noexcept(begin(r)))
						-> decltype(begin(r)) {
					return begin(r);
				}
			};
		}// namespace __begin

		inline namespace __begin_cpo {
			inline constexpr __begin::_fn begin = {};
		}

		namespace __end {
			void end();

			struct _fn {
				template<typename Range>
				requires requires(Range &r) {
					r.end();
				}
				auto operator()(Range &&r) const noexcept(noexcept(r.end()))
						-> decltype(r.end()) {
					return r.end();
				}

				template<typename Range>
				requires(!requires(Range & r) { r.end(); }) && requires(Range &r) {
					end(r);
				}
				auto operator()(Range &&r) const noexcept(noexcept(end(r)))
						-> decltype(end(r)) {
					return end(r);
				}
			};
		}// namespace __end

		inline namespace _end_cpo {
			inline constexpr __end::_fn end = {};
		}

		template<typename Range>
		using iterator_t = decltype(begin(std::declval<Range>()));

		template<typename Range>
		using sentinel_t = decltype(end(std::declval<Range>()));

		template<typename Range>
		using range_reference_t = iter_reference_t<iterator_t<Range>>;

		template<typename Range>
		using range_value_t = iter_value_t<iterator_t<Range>>;

		template<typename T>
		concept range = requires(T &t) {
			ranges::begin(t);
			ranges::end(t);
		};
	}// namespace ranges
}// namespace std
#endif


namespace std {
	template<typename Allocator>
	using rebound = typename std::allocator_traits<Allocator>::template rebind_alloc<char>;

	template<typename Allocator>
	concept proto_allocator = std::same_as<Allocator, void> ||(std::is_nothrow_copy_constructible_v<Allocator> &&requires(Allocator) {
		typename std::allocator_traits<Allocator>;
		typename rebound<Allocator>;
	});

	// template<typename T>
	// struct elements_of;

	template<typename R>
	struct elements_of {
		R &&__range;// \expos

		explicit constexpr elements_of(R &&r) noexcept : __range((R &&) r) {
		}
		constexpr elements_of(elements_of &&) noexcept = default;

		constexpr elements_of(const elements_of &) = delete;
		constexpr elements_of &operator=(const elements_of &) = delete;
		constexpr elements_of &operator=(elements_of &&) = delete;

		constexpr R &&get() &&noexcept {
			return std::forward<R>(__range);
		}
	};

	template<typename R>
	elements_of(R &&r) -> elements_of<decltype(r)>;

	template<typename Alloc>
	static constexpr bool allocator_needs_to_be_stored =
			!std::allocator_traits<Alloc>::is_always_equal::value ||
			!std::is_default_constructible_v<Alloc>;

	// Round s up to next multiple of a.
	constexpr size_t aligned_allocation_size(size_t s, size_t a) {
		return (s + a - 1) & ~(a - 1);
	}

	template<proto_allocator Alloc>
	class promise_base_alloc;

	template<>
	class promise_base_alloc<void> {

		using deleter = void (*)(void *, std::size_t);

		static constexpr std::size_t offset_of_deleter(std::size_t frameSize) {
			return aligned_allocation_size(frameSize, sizeof(deleter));
		}

		template<typename Alloc>
		static constexpr std::size_t offset_of_allocator(std::size_t frameSize) {
			return aligned_allocation_size(offset_of_deleter(frameSize) + sizeof(deleter), alignof(Alloc));
		}

		template<typename Alloc>
		static constexpr std::size_t padded_frame_size(std::size_t frameSize) {
			if constexpr (allocator_needs_to_be_stored<Alloc>) {
				return offset_of_allocator<Alloc>(frameSize) + sizeof(Alloc);
			} else {
				return offset_of_deleter(frameSize) + sizeof(deleter);
			}
		}

		template<typename Alloc>
		static Alloc &get_allocator(void *frame, std::size_t frameSize) {
			return *reinterpret_cast<Alloc *>(
					static_cast<char *>(frame) + offset_of_allocator<Alloc>(frameSize));
		}

		static deleter &get_deleter(void *frame, std::size_t frameSize) {
			return *(reinterpret_cast<deleter *>(
					static_cast<std::byte *>(frame) + offset_of_deleter(frameSize)));
		}

	public:
		static void *operator new(std::size_t size) {
			return promise_base_alloc::operator new(size, std::allocator_arg, std::allocator<std::byte>());
		}


		template<typename Alloc, typename... Args>
		static void *operator new(std::size_t frameSize, std::allocator_arg_t, Alloc alloc, Args &...) {

			using bytes_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<std::byte>;

			void *frame = alloc.allocate(padded_frame_size<bytes_alloc>(frameSize));


			deleter f = [](void *ptr, std::size_t frameSize) {
				if constexpr (allocator_needs_to_be_stored<Alloc>) {
					bytes_alloc &alloc = get_allocator<bytes_alloc>(ptr, frameSize);
					bytes_alloc localAlloc(std::move(alloc));
					alloc.~Alloc();
					localAlloc.deallocate(static_cast<std::byte *>(ptr), padded_frame_size<bytes_alloc>(frameSize));
				} else {
					bytes_alloc alloc;
					alloc.deallocate(static_cast<std::byte *>(ptr), padded_frame_size<bytes_alloc>(frameSize));
				}
			};
			::new (static_cast<void *>(std::addressof(get_deleter(frame, frameSize)))) deleter(std::move(f));
			if constexpr (allocator_needs_to_be_stored<bytes_alloc>) {
				// Store allocator at end of the coroutine frame.
				// Assuming the allocator's move constructor is non-throwing (a requirement for allocators)
				::new (static_cast<void *>(std::addressof(get_allocator<bytes_alloc>(frame, frameSize)))) bytes_alloc(std::move(alloc));
			}
			return frame;
		}

		template<typename Alloc, typename This, typename... Args>
		static void *operator new(std::size_t frameSize, This &, std::allocator_arg_t, Alloc alloc, Args &...) {
			return promise_base_alloc::operator new(frameSize, std::allocator_arg, std::move(alloc));
		}

		static void operator delete(void *ptr, std::size_t frameSize) {
			deleter f = get_deleter(ptr, frameSize);
			f(ptr, frameSize);
		}
	};

	template<proto_allocator Alloc>
	class promise_base_alloc {
		using bytes_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<std::byte>;


		static constexpr std::size_t offset_of_allocator(std::size_t frameSize) {
			return aligned_allocation_size(frameSize, alignof(bytes_alloc));
		}

		static constexpr std::size_t padded_frame_size(std::size_t frameSize) {
			return offset_of_allocator(frameSize) + sizeof(bytes_alloc);
		}

		static bytes_alloc &get_allocator(void *frame, std::size_t frameSize) {
			return *reinterpret_cast<bytes_alloc *>(
					static_cast<char *>(frame) + offset_of_allocator(frameSize));
		}

	public:
		template<typename... Args>
		static void *operator new(std::size_t frameSize, std::allocator_arg_t, Alloc alloc, Args &...) {
			bytes_alloc balloc = std::move(alloc);


			void *frame = balloc.allocate(padded_frame_size(frameSize));

			// Store allocator at end of the coroutine frame.
			// Assuming the allocator's move constructor is non-throwing (a requirement for allocators)
			::new (static_cast<void *>(std::addressof(get_allocator(frame, frameSize)))) Alloc(std::move(balloc));

			return frame;
		}

		template<typename This, typename... Args>
		static void *operator new(std::size_t frameSize, This &, std::allocator_arg_t, Alloc alloc, Args &...) {
			return promise_base_alloc::operator new(frameSize, std::allocator_arg, std::move(alloc));
		}

		static void operator delete(void *ptr, std::size_t frameSize) {
			bytes_alloc &alloc = get_allocator(ptr, frameSize);
			bytes_alloc localAlloc(std::move(alloc));
			alloc.~Alloc();
			localAlloc.deallocate(static_cast<char *>(ptr), padded_frame_size(frameSize));
		}
	};


	template<proto_allocator Alloc>
	requires(!allocator_needs_to_be_stored<Alloc>) class promise_base_alloc<Alloc> {
		using bytes_alloc = typename std::allocator_traits<Alloc>::template rebind_alloc<std::byte>;

	public:
		static void *operator new(std::size_t size) {
			bytes_alloc alloc;
			return alloc.allocate(size);
		}

		static void operator delete(void *ptr, std::size_t size) {
			bytes_alloc alloc;
			alloc.deallocate(static_cast<std::byte *>(ptr), size);
		}
	};

	template<typename Ref, typename Value = std::remove_cvref_t<Ref>, proto_allocator Alloc = void>
	class generator;


	template<typename Ref>
	struct promise_base {
		template<typename _Ref, typename Value, proto_allocator Alloc>
		friend class generator;

		std::coroutine_handle<promise_base> rootOrLeaf_;
		std::coroutine_handle<promise_base> parent_;
		std::exception_ptr *exception_ = nullptr;
		std::add_pointer_t<std::add_const_t<Ref>> value_;


		promise_base() noexcept
			: rootOrLeaf_(std::coroutine_handle<promise_base>::from_promise(*this)) {}

		void unhandled_exception() {
			if (exception_ == nullptr) throw;
			*exception_ = std::current_exception();
		}


		// Transfers control back to the parent of a nested coroutine
		struct final_awaiter {
			bool await_ready() noexcept {
				return false;
			}
			template<typename T>
			std::coroutine_handle<>
			await_suspend(std::coroutine_handle<T> h) noexcept {
				auto &promise = h.promise();
				std::coroutine_handle<promise_base> parent = promise.parent_;
				if (parent) {
					auto &root = promise.rootOrLeaf_.promise();
					root.rootOrLeaf_ = parent;
					return parent;
				}
				return std::noop_coroutine();
			}
			void await_resume() noexcept {}
		};

		final_awaiter final_suspend() noexcept {
			return {};
		}

		std::suspend_always yield_value(const Ref &x) {
			auto &root = rootOrLeaf_.promise();
			root.value_ = std::addressof(x);
			return {};
		}

		struct yield_value_holder {
			Ref ref;
			template<typename T>
			explicit yield_value_holder(T &&x) : ref(static_cast<T &&>(x)) {}

			bool await_ready() noexcept { return false; }

			template<typename Promise>
			void await_suspend(std::coroutine_handle<Promise> h) noexcept {
				h.promise().value_ = std::addressof(ref);
			}

			void await_resume() noexcept {}
		};

		template<typename T = std::remove_cvref_t<Ref>>
		requires std::is_convertible_v<T, Ref> && std::is_constructible_v<Ref, T> &&(
				!std::same_as<std::remove_cvref_t<T>, Ref>) yield_value_holder
				yield_value(T &&x) {
			return yield_value_holder{static_cast<T &&>(x)};
		}

		template<typename Gen>
		struct yield_sequence_awaiter {
			using promise_type = typename Gen::promise_type;

			Gen gen_;
			std::exception_ptr exception_;

			yield_sequence_awaiter(Gen &&g) noexcept
				// Taking ownership of the generator ensures frame are destroyed
				// in the reverse order of their creation
				: gen_(std::move(g)) {}

			bool await_ready() noexcept { return !gen_.coro_; }

			// set the parent, root and exceptions pointer and
			// resume the nested coroutine
			std::coroutine_handle<> await_suspend(
					std::coroutine_handle<promise_type> h) noexcept {
				auto &current = h.promise();
				auto &nested = gen_.coro_.promise();
				auto &root = current.rootOrLeaf_.promise();

				nested.rootOrLeaf_ = current.rootOrLeaf_;
				root.rootOrLeaf_ =
						std::coroutine_handle<promise_base>::from_address(gen_.coro_.address());
				nested.parent_ =
						std::coroutine_handle<promise_base>::from_address(h.address());
				nested.exception_ = &exception_;
				// Immediately resume the nested coroutine (nested generator)
				return gen_.coro_;
			}

			void await_resume() {
				if (exception_) { std::rethrow_exception(std::move(exception_)); }
			}
		};

		void resume() { rootOrLeaf_.resume(); }

		// Disable use of co_await within this coroutine.
		void await_transform() = delete;
	};


	template<typename Ref, typename Value, proto_allocator Alloc>
	class generator {


	public:
		class promise_type
			: public promise_base<Ref>,
			  public promise_base_alloc<Alloc> {
		public:
			using promise_base<Ref>::promise_base;


			generator get_return_object() noexcept {
				return generator{std::coroutine_handle<promise_type>::from_promise(
						*this)};
			}

			std::suspend_always yield_value(
					Ref x) requires std::is_lvalue_reference_v<Ref> {
				return promise_base<Ref>::yield_value(x);
				return {};
			}

			template<typename T = std::remove_cvref_t<Ref>>
			requires std::is_convertible_v<T, Ref>
			auto yield_value(T &&x) noexcept(std::is_nothrow_constructible_v<Ref, T>) {
				return promise_base<Ref>::yield_value(std::forward<T>(x));
			}

			void return_void() noexcept {}

			std::suspend_always initial_suspend() noexcept { return {}; }


			template<typename OValue, typename OAlloc>
			typename promise_base<Ref>::template yield_sequence_awaiter<
					generator<Ref, OValue, OAlloc>>
			yield_value(elements_of<generator<Ref, OValue, OAlloc>> g) noexcept {
				return (generator<Ref, OValue, OAlloc> &&) std::move(g).get();
			}

			// Adapt any std::elements_of() range that
			template<std::ranges::range R>
			// requires std::convertible_to<std::ranges::range_reference_t<R>, Ref>
			typename promise_base<Ref>::template yield_sequence_awaiter<generator>
			yield_value(elements_of<R> r) {
				auto &&range = std::move(r).get();
				for (auto &&v : range) co_yield v;
			}

		private:
		};

		generator() noexcept = default;

		generator(generator &&other) noexcept
			: coro_(std::exchange(other.coro_, {})),
			  started_(std::exchange(other.started_, false)) {}

		~generator() noexcept {
			if (coro_) {
				if (started_ && !coro_.done()) {
					//coro_.promise().value_.destruct();
				}
				coro_.destroy();
			}
		}

		generator &operator=(generator g) noexcept {
			swap(g);
			return *this;
		}

		void swap(generator &other) noexcept {
			std::swap(coro_, other.coro_);
			std::swap(started_, other.started_);
		}

		class iterator {
			using coroutine_handle = std::coroutine_handle<promise_type>;

		public:
			using iterator_category = std::input_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Value;
			using reference = Ref;
			using pointer = std::add_pointer_t<Ref>;

			iterator() noexcept = default;
			iterator(const iterator &) = delete;

			iterator(iterator &&o) noexcept : coro_(std::exchange(o.coro_, {})) {}

			iterator &operator=(iterator &&o) {
				std::swap(coro_, o.coro_);
				return *this;
			}

			~iterator() {}

			friend bool operator==(const iterator &it,
								   std::default_sentinel_t) noexcept {
				return !it.coro_ || it.coro_.done();
			}

			iterator &operator++() {
				// coro_.promise().value_.destruct();
				coro_.promise().resume();
				return *this;
			}
			void operator++(int) { (void) operator++(); }

			reference operator*() const noexcept {
				return static_cast<reference>(*(coro_.promise().value_));
			}

		private:
			friend generator;
			explicit iterator(coroutine_handle coro) noexcept : coro_(coro) {}

			coroutine_handle coro_;
		};

		iterator begin() {
			if (coro_) {
				started_ = true;
				coro_.resume();
			}
			return iterator{coro_};
		}

		std::default_sentinel_t end() noexcept { return {}; }

		//  private:
		explicit generator(std::coroutine_handle<promise_type> coro) noexcept
			: coro_(coro) {}

		std::coroutine_handle<promise_type> coro_;
		bool started_ = false;
	};


}// namespace std

namespace std {
	template<typename T, typename U>
	constexpr bool std::ranges::enable_view<std::generator<T, U>> = true;
}
#endif

#endif//HYPERTRIE_GENERATOR_HPP
