#ifndef HYPERTRIE_ITERATOR_HPP
#define HYPERTRIE_ITERATOR_HPP

#include "dice/hypertrie/Hypertrie_default_traits.hpp"
#include "dice/hypertrie/Hypertrie_predeclare.hpp"
#include "dice/hypertrie/internal/raw/iteration/RawIterator.hpp"
#include "dice/template-library/switch_cases.hpp"

namespace dice::hypertrie {

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	class Iterator {
	public:
		using key_part_type = typename htt_t::key_part_type;
		using value_type = NonZeroEntry<htt_t> const;
		using reference = value_type &;
		using pointer = value_type *;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;

	protected:
		template<size_t depth>
		using RawIterator_t = internal::raw::RawIterator<depth, false, htt_t, allocator_type>;

		using max_sized_RawIterator_t = RawIterator_t<hypertrie_max_depth>;

		struct VTable {
			void (*construct)(void *location, const_Hypertrie<htt_t, allocator_type> const &hypertrie) noexcept;
			void (*copy)(void *location, void const *self) noexcept;
			void (*move)(void *location, void *self) noexcept;
			void (*destroy)(void *self) noexcept;
			reference (*value)(void const *self) noexcept;
			void (*advance)(void *self) noexcept;
			bool (*ended)(void const *self) noexcept;

			template<size_t depth>
			static consteval VTable make() {
				using RawIterator_tt = RawIterator_t<depth>;

				return VTable{
						.construct = [](void *location, const_Hypertrie<htt_t, allocator_type> const &hypertrie) noexcept -> void {
							if (hypertrie.empty()) {
								new (location) RawIterator_tt{};
							} else if (hypertrie.depth() == 0) {
								if constexpr (depth == 0) {
									new (location) RawIterator_tt{hypertrie.to_scalar()};
								} else {
									HYPERTRIE_UNREACHABLE;
								}
							} else {
								if constexpr (depth > 0) {
									using internal::raw::Ownership;
									new (location) RawIterator_tt{hypertrie.template node_ptr<depth>()};
								} else {
									HYPERTRIE_UNREACHABLE;
								}
							}
						},
						.copy = [](void *location, void const *self) noexcept -> void {
							new (location) RawIterator_tt{*reinterpret_cast<RawIterator_tt const *>(self)};
						},
						.move = [](void *location, void *self) noexcept -> void {
							new (location) RawIterator_tt{std::move(*reinterpret_cast<RawIterator_tt *>(self))};
						},
						.destroy = [](void *self) noexcept -> void {
							reinterpret_cast<RawIterator_tt *>(self)->~RawIterator_tt();
						},
						.value = [](void const *self) noexcept -> reference {
							return reinterpret_cast<RawIterator_tt const *>(self)->value();
						},
						.advance = [](void *self) noexcept -> void {
							reinterpret_cast<RawIterator_tt *>(self)->advance();
						},
						.ended = [](void const *self) noexcept -> bool {
							return reinterpret_cast<RawIterator_tt const *>(self)->ended();
						}};
			}
		};

		template<size_t ...depths>
		static consteval std::array<VTable, hypertrie_max_depth + 1> make_vtables(std::index_sequence<depths...>) {
			return {VTable::template make<depths>()...};
		}

		static constexpr std::array<VTable, hypertrie_max_depth + 1> vtables_ = make_vtables(std::make_index_sequence<hypertrie_max_depth + 1>{});

		VTable const *vtable_;
		alignas(max_sized_RawIterator_t) std::byte inner_[sizeof(max_sized_RawIterator_t)];

		void drop() noexcept {
			if (vtable_ != nullptr) {
				// need to call destructor because RawIterator with non-raw keys is not trivially destructible
				vtable_->destroy(inner_);
			}
		}

	public:
		explicit Iterator(const_Hypertrie<htt_t, allocator_type> const &hypertrie) noexcept : vtable_{&vtables_[hypertrie.depth()]} {
			vtable_->construct(inner_, hypertrie);
		}

		Iterator(Iterator const &other) : vtable_{other.vtable_} {
			vtable_->copy(inner_, other.inner_);
		}

		Iterator &operator=(Iterator const &other) {
			if (this == &other) {
				return *this;
			}

			drop();
			vtable_ = other.vtable_;
			vtable_->copy(inner_, other.inner_);
			return *this;
		}

		Iterator(Iterator &&other) noexcept : vtable_{std::exchange(other.vtable_, nullptr)} {
			vtable_->move(inner_, other.inner_);
		}

		Iterator &operator=(Iterator &&other) noexcept {
			if (this == &other) {
				return *this;
			}

			drop();
			vtable_ = std::exchange(other.vtable_, nullptr);
			vtable_->move(inner_, other.inner_);
			return *this;
		}

		~Iterator() noexcept {
			drop();
		}

		void advance() noexcept {
			vtable_->advance(inner_);
		}

		Iterator &operator++() noexcept {
			advance();
			return *this;
		}

		Iterator operator++(int) {
			auto cpy = *this;
			this->advance();
			return cpy;
		}

		[[nodiscard]] reference value() const noexcept {
			return vtable_->value(inner_);
		}

		reference operator*() const noexcept {
			return value();
		}

		pointer operator->() const noexcept {
			return &value();
		}

		[[nodiscard]] bool ended() const noexcept {
			return vtable_->ended(inner_);
		}

		explicit operator bool() const noexcept {
			return !ended();
		}

		friend bool operator==(Iterator const &self, std::default_sentinel_t) noexcept {
			return self.ended();
		}

		friend bool operator==(std::default_sentinel_t, Iterator const &self) noexcept {
			return self.ended();
		}

		friend bool operator!=(Iterator const &self, std::default_sentinel_t) noexcept {
			return !self.ended();
		}

		friend bool operator!=(std::default_sentinel_t, Iterator const &self) noexcept {
			return !self.ended();
		}
	};

}// namespace dice::hypertrie


#endif//HYPERTRIE_ITERATOR_HPP
