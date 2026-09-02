#ifndef HYPERTRIE_NODEPTR_HPP
#define HYPERTRIE_NODEPTR_HPP

#include <bit>
#include <cassert>
#include <cstddef>
#include <functional>
#include <utility>

#include <boost/interprocess/offset_ptr.hpp>

#include "dice/hypertrie/Hypertrie_trait.hpp"
#include "dice/hypertrie/internal/raw/node/NodeTypes_predeclare.hpp"
#include "dice/hypertrie/internal/raw/node/NodeTypes_reflection.hpp"
#include "dice/hypertrie/internal/raw/node/RawIdentifier.hpp"
#include "dice/hypertrie/internal/util/Overloaded.hpp"
#include "dice/hypertrie/internal/util/Unreachable.hpp"

namespace dice::hypertrie::internal::raw {

	template<ByteAllocator allocator_type>
	using VoidPtr = typename std::allocator_traits<allocator_type>::void_pointer;

	template<typename T, ByteAllocator allocator_type>
	using Ptr = typename std::allocator_traits<allocator_type>::template rebind_traits<T>::pointer;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	using FNPtr = Ptr<FullNode<depth, htt_t, allocator_type>, allocator_type>;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	using SENPtr = Ptr<SingleEntryNode<depth, htt_t>, allocator_type>;

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	using XNPtr = Ptr<CartesianNode<depth, htt_t, allocator_type>, allocator_type>;

	template<template<size_t, typename, typename...> typename Node, size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	using SpecificNodePtr = Ptr<instantiate_Node_t<Node, depth, htt_t, allocator_type>, allocator_type>;

	template<typename P>
	struct IsRawPtr : std::false_type {
	};

	template<typename T>
	struct IsRawPtr<T *> : std::true_type {
	};

	template<typename T>
	struct IsRawPtr<T const *> : std::true_type {
	};

	template<typename PTarget, typename PSource>
	PTarget reinterpret_pointer_cast(PSource ptr) noexcept {
		static_assert(IsRawPtr<PSource>::value == IsRawPtr<PTarget>::value);
		if constexpr (IsRawPtr<PSource>::value) {
			return reinterpret_cast<PTarget>(ptr);
		} else {
			return PTarget{ptr};
		}
	}

	namespace pointer_tagging_detail {
		template<typename void_ptr, HypertrieTrait htt_t>
		struct TaggedPtrRepr;

		template<HypertrieTrait htt_t>
		struct TaggedPtrRepr<void *, htt_t> {
			using void_pointer = void *;
			using repr_type = uintptr_t;

			static_assert(!HypertrieTrait_taggable_key_part<htt_t> || sizeof(typename htt_t::key_part_type) <= sizeof(repr_type),
						  "key_part_type must fit into pointer repr type in taggable configurations");

			template<bool inlined_key_part_possible>
			[[nodiscard]] static repr_type reencode_from_other([[maybe_unused]] TaggedPtrRepr const *src, repr_type tagged) noexcept {
				return tagged;
			}

			[[nodiscard]] static repr_type encode_pointer(void_pointer ptr, IdentifierTag tag) noexcept {
				auto repr = reinterpret_cast<repr_type>(ptr);
				assert((repr & 0b11) == 0);
				return repr | static_cast<repr_type>(tag);
			}

			[[nodiscard]] static std::pair<void_pointer, IdentifierTag> decode_pointer(repr_type tagged) noexcept {
				auto const tag = static_cast<IdentifierTag>(tagged & repr_type{0b11});
				auto *const ptr = reinterpret_cast<void *>(tagged & ~repr_type{0b11});
				return std::make_pair(ptr, tag);
			}

			[[nodiscard]] static repr_type encode_key_part(typename htt_t::key_part_type key_part) noexcept requires (HypertrieTrait_taggable_key_part<htt_t>) {
				assert(std::countl_zero(static_cast<repr_type>(key_part)) >= 2);
				return (static_cast<repr_type>(key_part) << 2) | static_cast<repr_type>(IdentifierTag::SEN);
			}

			[[nodiscard]] static typename htt_t::key_part_type decode_key_part(repr_type tagged) noexcept requires (HypertrieTrait_taggable_key_part<htt_t>) {
				assert(static_cast<IdentifierTag>(tagged & 0b11) == IdentifierTag::SEN);
				static constexpr repr_type mask = ~(static_cast<repr_type>(0b11) << (sizeof(repr_type) * 8 - 2));

				return static_cast<typename htt_t::key_part_type>((tagged >> 2) & mask);
			}

			[[nodiscard]] static IdentifierTag decode_tag(repr_type tagged) noexcept {
				return static_cast<IdentifierTag>(tagged & 0b11);
			}

			template<bool inlined_key_part_possible>
			[[nodiscard]] static bool eq(repr_type this_tagged, [[maybe_unused]] TaggedPtrRepr const *other, repr_type other_tagged) noexcept {
				return this_tagged == other_tagged;
			}

			template<bool inlined_key_part_possible>
			[[nodiscard]] static bool ne(repr_type this_tagged, [[maybe_unused]] TaggedPtrRepr const *other, repr_type other_tagged) noexcept {
				return this_tagged != other_tagged;
			}

			[[nodiscard]] static bool eq_null(repr_type this_tagged) noexcept {
				// nullptr representation: address == 0 | tag == IdentifierTag::Indeterminate == 0 => 0
				return this_tagged == 0;
			}

			[[nodiscard]] static bool ne_null(repr_type this_tagged) noexcept {
				// nullptr representation: address == 0 | tag == IdentifierTag::Indeterminate == 0 => 0
				return this_tagged != 0;
			}

			template<typename Policy, bool inlined_key_part_possible>
			[[nodiscard]] static auto hash(repr_type tagged) noexcept {
				return ::dice::hash::dice_hash_templates<Policy>::dice_hash(tagged);
			}
		};

		template<HypertrieTrait htt_t>
		struct TaggedPtrRepr<boost::interprocess::offset_ptr<void>, htt_t> {
			// Note: NodePtr using offset_ptr cannot point to itself, as that is the nullptr representation

			using void_pointer = boost::interprocess::offset_ptr<void>;
			using repr_type = void_pointer::offset_type;

			[[nodiscard]] void_pointer offset_to_pointer(repr_type offset) const noexcept {
				repr_type const mask = static_cast<repr_type>(offset == 0) - 1; // 0b000... if offset == 0 else 0b111...
				repr_type const target = (reinterpret_cast<repr_type>(this) + offset) & mask;
				return void_pointer{reinterpret_cast<void *>(target)};
			}

			[[nodiscard]] repr_type pointer_to_offset(void_pointer ptr) const noexcept {
				assert(std::to_address(ptr) != reinterpret_cast<void const *>(this));
				repr_type const mask = static_cast<repr_type>(ptr == nullptr) - 1; // 0b000... if ptr == nullptr else 0b111...
				repr_type const offset = (reinterpret_cast<repr_type>(std::to_address(ptr)) - reinterpret_cast<repr_type>(this)) & mask;
				return offset;
			}

			[[nodiscard]] static repr_type encode_offset(repr_type offset, IdentifierTag tag) noexcept {
				assert((offset & 0b11) == 0);
				return offset | static_cast<repr_type>(tag);
			}

			template<bool inlined_key_part_possible>
			[[nodiscard]] repr_type reencode_from_other(TaggedPtrRepr const *src, repr_type tagged) const noexcept {
				static_assert(!inlined_key_part_possible, "logic error");
				auto const [ptr, type_tag] = src->decode_pointer(tagged);
				return encode_pointer(ptr, type_tag);
			}

			[[nodiscard]] repr_type encode_pointer(void_pointer ptr, IdentifierTag tag) const noexcept {
				return encode_offset(pointer_to_offset(ptr), tag);
			}

			[[nodiscard]] std::pair<void_pointer, IdentifierTag> decode_pointer(repr_type tagged) const noexcept {
				auto const type_tag = static_cast<IdentifierTag>(tagged & 0b11);
				auto const offset = tagged & ~0b11;

				return std::make_pair(offset_to_pointer(offset), type_tag);
			}

			[[nodiscard]] static IdentifierTag decode_tag(repr_type tagged) noexcept {
				return static_cast<IdentifierTag>(tagged & 0b11);
			}

			template<bool inlined_key_part_possible>
			[[nodiscard]] bool eq(repr_type this_tagged, TaggedPtrRepr const *other, repr_type other_tagged) const noexcept {
				static_assert(!inlined_key_part_possible, "logic error");
				return this->decode_pointer(this_tagged) == other->decode_pointer(other_tagged);
			}

			template<bool inlined_key_part_possible>
			[[nodiscard]] bool ne(repr_type this_tagged, TaggedPtrRepr const *other, repr_type other_tagged) const noexcept {
				static_assert(!inlined_key_part_possible, "logic error");
				return this->decode_pointer(this_tagged) != other->decode_pointer(other_tagged);
			}

			[[nodiscard]] static bool eq_null(repr_type this_tagged) noexcept {
				// nullptr representation: offset == 0 && tag == IdentifierTag::Indeterminate == 0 => 0
				return this_tagged == 0;
			}

			[[nodiscard]] static bool ne_null(repr_type this_tagged) noexcept {
				// nullptr representation: offset == 0 && tag == IdentifierTag::Indeterminate == 0 => 0
				return this_tagged != 0;
			}

			template<typename Policy, bool inlined_key_part_possible>
			[[nodiscard]] size_t hash(repr_type tagged) const noexcept {
				static_assert(!inlined_key_part_possible, "logic error");
				using H = hash::dice_hash_templates<Policy>;

				auto [ptr, tag] = decode_pointer(tagged);
				return Policy::hash_combine({H::dice_hash(std::to_address(ptr)), H::dice_hash(tag)});
			}
		};

		template<HypertrieTrait_taggable_key_part htt_t>
		struct TaggedPtrRepr<boost::interprocess::offset_ptr<void>, htt_t> {
			// Note: NodePtr using offset_ptr cannot point to itself, as that is the nullptr representation

			using void_pointer = boost::interprocess::offset_ptr<void>;
			using repr_type = void_pointer::offset_type;

			static_assert(!HypertrieTrait_taggable_key_part<htt_t> || sizeof(typename htt_t::key_part_type) <= sizeof(repr_type),
						  "key_part_type must fit into pointer repr type in taggable configurations");

			[[nodiscard]] void_pointer offset_to_pointer(repr_type offset) const noexcept {
				repr_type const mask = static_cast<repr_type>(offset == 0) - 1; // 0b000... if offset == 0 else 0b111...
				repr_type const target = (reinterpret_cast<repr_type>(this) + offset) & mask;
				return void_pointer{reinterpret_cast<void *>(target)};
			}

			[[nodiscard]] repr_type pointer_to_offset(void_pointer ptr) const noexcept {
				assert(std::to_address(ptr) != reinterpret_cast<void const *>(this));
				repr_type const mask = static_cast<repr_type>(ptr == nullptr) - 1; // 0b000... if ptr == nullptr else 0b111...
				repr_type const offset = (reinterpret_cast<repr_type>(std::to_address(ptr)) - reinterpret_cast<repr_type>(this)) & mask;
				return offset;
			}

			[[nodiscard]] static repr_type encode_offset(repr_type offset, IdentifierTag tag) noexcept {
				assert((offset & 0b111) == 0);
				return offset | static_cast<repr_type>(tag);
			}

			[[nodiscard]] std::tuple<repr_type, bool, IdentifierTag> decode_all(repr_type tagged) const noexcept {
				auto const type_tag = static_cast<IdentifierTag>(tagged & 0b11);
				auto const inline_tag = static_cast<bool>(tagged & 0b100);
				auto const offset = tagged & ~0b111;

				return std::make_tuple(offset, inline_tag, type_tag);
			}

			template<bool inlined_key_part_possible>
			[[nodiscard]] repr_type reencode_from_other(TaggedPtrRepr const *src, repr_type tagged) const noexcept {
				auto const [offset, inline_tag, type_tag] = src->decode_all(tagged);
				if constexpr (inlined_key_part_possible) {
					// try to avoid branch wherever possible
					if (inline_tag) {
						return tagged;
					}
				}

				return encode_pointer(src->offset_to_pointer(offset), type_tag);
			}

			[[nodiscard]] repr_type encode_pointer(void_pointer ptr, IdentifierTag tag) const noexcept {
				return encode_offset(pointer_to_offset(ptr), tag);
			}

			[[nodiscard]] std::pair<void_pointer, IdentifierTag> decode_pointer(repr_type tagged) const noexcept {
				assert((tagged & 0b100) == 0);

				auto const type_tag = static_cast<IdentifierTag>(tagged & 0b011);
				auto const offset = tagged & ~0b111;

				return std::make_pair(offset_to_pointer(offset), type_tag);
			}

			[[nodiscard]] static repr_type encode_key_part(typename htt_t::key_part_type key_part) noexcept {
				assert(std::countl_zero(static_cast<repr_type>(key_part)) >= 3);
				return (static_cast<repr_type>(key_part) << 3) | (static_cast<repr_type>(1) << 2) | static_cast<repr_type>(IdentifierTag::SEN);
			}

			[[nodiscard]] static typename htt_t::key_part_type decode_key_part(repr_type tagged) noexcept {
				assert((tagged & 0b100) != 0);
				assert(static_cast<IdentifierTag>(tagged & 0b011) == IdentifierTag::SEN);
				static constexpr repr_type mask = ~(repr_type{0b111} << (sizeof(repr_type) * 8 - 3));

				return static_cast<typename htt_t::key_part_type>((tagged >> 3) & mask);
			}

			[[nodiscard]] static IdentifierTag decode_tag(repr_type tagged) noexcept {
				return static_cast<IdentifierTag>(tagged & 0b11);
			}

			template<bool inlined_key_part_possible>
			[[nodiscard]] bool eq(repr_type this_tagged, TaggedPtrRepr const *other, repr_type other_tagged) const noexcept {
				auto const [this_off, this_inlined, this_tag] = this->decode_all(this_tagged);
				auto const [other_off, other_inlined, other_tag] = other->decode_all(other_tagged);

				if constexpr (inlined_key_part_possible) {
					if (this_inlined) {
						return other_inlined && this_tag == other_tag && this_off == other_off;
					}

					return !other_inlined && this_tag == other_tag && this->offset_to_pointer(this_off) == other->offset_to_pointer(other_off);
				} else {
					assert(!this_inlined && !other_inlined);
					return this_tag == other_tag && this->offset_to_pointer(this_off) == other->offset_to_pointer(other_off);
				}
			}

			template<bool inlined_key_part_possible>
			[[nodiscard]] bool ne(repr_type this_tagged, TaggedPtrRepr const *other, repr_type other_tagged) const noexcept {
				auto const [this_off, this_inlined, this_tag] = this->decode_all(this_tagged);
				auto const [other_off, other_inlined, other_tag] = other->decode_all(other_tagged);

				if constexpr (inlined_key_part_possible) {
					if (this_inlined) {
						return !other_inlined || this_tag != other_tag || this_off != other_off;
					}

					return other_inlined || this_tag != other_tag || this->offset_to_pointer(this_off) != other->offset_to_pointer(other_off);
				} else {
					assert(!this_inlined && !other_inlined);
					return this_tag != other_tag || this->offset_to_pointer(this_off) != other->offset_to_pointer(other_off);
				}
			}

			[[nodiscard]] static bool eq_null(repr_type this_tagged) noexcept {
				// nullptr representation: offset == 0 && tag == IdentifierTag::Indeterminate == 0 => 0
				return this_tagged == 0;
			}

			[[nodiscard]] static bool ne_null(repr_type this_tagged) noexcept {
				// nullptr representation: offset == 0 && tag == IdentifierTag::Indeterminate == 0 => 0
				return this_tagged != 0;
			}

			template<typename Policy, bool inlined_key_part_possible>
			[[nodiscard]] auto hash(repr_type tagged) const noexcept {
				auto const [offset, inlined, tag] = decode_all(tagged);

				if constexpr (inlined_key_part_possible) {
					if (inlined) {
						return ::dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(static_cast<typename htt_t::key_part_type>(offset), true, tag));
					}
				}

				return ::dice::hash::dice_hash_templates<Policy>::dice_hash(std::make_tuple(std::to_address(offset_to_pointer(offset)), false, tag));
			}
		};
	} // namespace pointer_tagging_detail

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct RawNodePtrBase : protected pointer_tagging_detail::TaggedPtrRepr<typename std::allocator_traits<allocator_type>::void_pointer, htt_t> {
	private:
		using tagging_impl = pointer_tagging_detail::TaggedPtrRepr<typename std::allocator_traits<allocator_type>::void_pointer, htt_t>;
		using allocator_traits = std::allocator_traits<allocator_type>;

	protected:
		typename tagging_impl::repr_type tagged_ptr_;

		explicit RawNodePtrBase(typename tagging_impl::repr_type tagged_ptr) noexcept : tagged_ptr_{tagged_ptr} {
		}

	public:
		using VoidPtr_t = typename allocator_traits::void_pointer;

		RawNodePtrBase() noexcept : tagged_ptr_{tagging_impl::encode_pointer(nullptr, IdentifierTag::Indeterminate)} {
		}

		RawNodePtrBase(std::nullptr_t) noexcept : RawNodePtrBase{} {
		}

		RawNodePtrBase(VoidPtr_t ptr, IdentifierTag tag) noexcept : tagged_ptr_{tagging_impl::encode_pointer(ptr, tag)} {
			assert((ptr == nullptr && tag == IdentifierTag::Indeterminate)
				   || (ptr != nullptr && tag != IdentifierTag::Indeterminate));
		}

		// deleted because they can be more efficiently implemented downstream
		RawNodePtrBase(RawNodePtrBase const &other) noexcept = delete;
		RawNodePtrBase(RawNodePtrBase &&other) noexcept = delete;
		RawNodePtrBase &operator=(RawNodePtrBase const &other) noexcept = delete;
		RawNodePtrBase &operator=(RawNodePtrBase &&other) noexcept = delete;

		~RawNodePtrBase() noexcept = default;

		[[nodiscard]] std::pair<VoidPtr_t, IdentifierTag> decode_ptr_parts() const noexcept {
			return tagging_impl::decode_pointer(tagged_ptr_);
		}

		[[nodiscard]] VoidPtr_t ptr() const noexcept {
			return decode_ptr_parts().first;
		}

		[[nodiscard]] IdentifierTag tag() const noexcept {
			return tagging_impl::decode_tag(tagged_ptr_);
		}

		[[nodiscard]] bool is_sen() const noexcept {
			return tag() == IdentifierTag::SEN;
		}

		[[nodiscard]] bool is_fn() const noexcept {
			return tag() == IdentifierTag::FN;
		}

		[[nodiscard]] bool is_xn() const noexcept {
			return tag() == IdentifierTag::XN;
		}

		[[nodiscard]] bool is_indeterminate() const noexcept {
			return tag() == IdentifierTag::Indeterminate;
		}
	};

	template<HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct RawNodePtr : RawNodePtrBase<htt_t, allocator_type> {
	private:
		using Base = RawNodePtrBase<htt_t, allocator_type>;
		static constexpr bool key_part_inlining_possible = HypertrieTrait_taggable_key_part<htt_t>;

		template<size_t, HypertrieTrait, ByteAllocator>
		friend struct NodePtr;

	public:
		using Base::Base;
		using typename Base::VoidPtr_t;

		RawNodePtr(RawNodePtr const &other) noexcept : Base{Base::template reencode_from_other<key_part_inlining_possible>(&other, other.tagged_ptr_)} {
		}

		RawNodePtr(RawNodePtr &&other) noexcept : Base{Base::template reencode_from_other<key_part_inlining_possible>(&other, other.tagged_ptr_)} {
		}

		RawNodePtr &operator=(RawNodePtr const &other) noexcept {
			this->tagged_ptr_ = Base::template reencode_from_other<key_part_inlining_possible>(&other, other.tagged_ptr_);
			return *this;
		}

		RawNodePtr &operator=(RawNodePtr &&other) noexcept {
			this->tagged_ptr_ = Base::template reencode_from_other<key_part_inlining_possible>(&other, other.tagged_ptr_);
			return *this;
		}

		~RawNodePtr() noexcept = default;

		static RawNodePtr encode_key_part(typename htt_t::key_part_type key_part) noexcept {
			static_assert(HypertrieTrait_taggable_key_part<htt_t>, "Cannot encode key part; conditions not met");
			return RawNodePtr{Base::encode_key_part(key_part)};
		}

		typename htt_t::key_part_type decode_key_part() const noexcept {
			static_assert(HypertrieTrait_taggable_key_part<htt_t>, "Cannot decode key part; conditions not met");
			return Base::decode_key_part(this->tagged_ptr_);
		}

		bool operator==(RawNodePtr const &other) const noexcept {
			return Base::template eq<key_part_inlining_possible>(this->tagged_ptr_, &other, other.tagged_ptr_);
		}

		bool operator!=(RawNodePtr const &other) const noexcept {
			return Base::template ne<key_part_inlining_possible>(this->tagged_ptr_, &other, other.tagged_ptr_);
		}

		bool operator==(std::nullptr_t) const noexcept {
			return Base::eq_null(this->tagged_ptr_);
		}

		bool operator!=(std::nullptr_t) const noexcept {
			return Base::ne_null(this->tagged_ptr_);
		}

		template<::dice::hash::Policies::HashPolicy Policy>
		[[nodiscard]] size_t hash() const noexcept {
			return Base::template hash<Policy, key_part_inlining_possible>(this->tagged_ptr_);
		}
	};

	template<size_t depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct NodePtr : RawNodePtr<htt_t, allocator_type> {
	private:
		using Base = RawNodePtr<htt_t, allocator_type>;
		static constexpr bool key_part_inlining_possible = depth == 1 && HypertrieTrait_taggable_key_part<htt_t>;

	public:
		using typename Base::VoidPtr_t;
		using FNPtr_t = FNPtr<depth, htt_t, allocator_type>;
		using SENPtr_t = SENPtr<depth, htt_t, allocator_type>;
		using XNPtr_t = XNPtr<depth, htt_t, allocator_type>;

		using Base::Base;

		NodePtr(NodePtr const &other) noexcept : Base{Base::template reencode_from_other<key_part_inlining_possible>(&other, other.tagged_ptr_)} {
		}

		NodePtr(NodePtr &&other) noexcept : Base{Base::template reencode_from_other<key_part_inlining_possible>(&other, other.tagged_ptr_)} {
		}

		NodePtr &operator=(NodePtr const &other) noexcept {
			this->tagged_ptr_ = Base::template reencode_from_other<key_part_inlining_possible>(&other, other.tagged_ptr_);
			return *this;
		}

		NodePtr &operator=(NodePtr &&other) noexcept {
			assert(this != &other);
			this->tagged_ptr_ = Base::template reencode_from_other<key_part_inlining_possible>(&other, other.tagged_ptr_);
			return *this;
		}

		NodePtr(RawNodePtr<htt_t, allocator_type> const &raw) noexcept : Base{Base::template reencode_from_other<key_part_inlining_possible>(&raw, raw.tagged_ptr_)} {
		}

		NodePtr(FNPtr_t ptr) noexcept : Base{ptr, IdentifierTag::FN} {
			assert(ptr != nullptr);
		}

		NodePtr(SENPtr_t ptr) noexcept : Base{ptr, IdentifierTag::SEN} {
			assert(ptr != nullptr);
		}

		NodePtr(XNPtr_t ptr) noexcept : Base{ptr, IdentifierTag::XN} {
			assert(ptr != nullptr);
		}

		~NodePtr() noexcept = default;

		[[nodiscard]] static NodePtr encode_key_part(typename htt_t::key_part_type key_part) noexcept {
			static_assert(depth == 1 && HypertrieTrait_taggable_key_part<htt_t>, "Cannot encode key part; conditions not met");
			return Base::encode_key_part(key_part);
		}

		[[nodiscard]] typename htt_t::key_part_type decode_key_part() const noexcept {
			static_assert(depth == 1 && HypertrieTrait_taggable_key_part<htt_t>, "Cannot decode key part; conditions not met");
			return Base::decode_key_part();
		}

		template<template<size_t, typename, typename...> typename Node>
		[[nodiscard]] SpecificNodePtr<Node, depth, htt_t, allocator_type> specific_ptr() const noexcept {
			assert(node_tag_matches<Node>(this->tag()));
			return reinterpret_pointer_cast<SpecificNodePtr<Node, depth, htt_t, allocator_type>>(this->ptr());
		}

		template<typename V>
			requires std::invocable<V, FNPtr<depth, htt_t, allocator_type>>
					 && std::invocable<V, SENPtr<depth, htt_t, allocator_type>>
					 && std::invocable<V, XNPtr<depth, htt_t, allocator_type>>
		decltype(auto) visit_ptr(V &&visitor) const noexcept {
			switch (this->tag()) {
				case IdentifierTag::FN: {
					return std::invoke(std::forward<V>(visitor), specific_ptr<FullNode>());
				}
				case IdentifierTag::SEN: {
					return std::invoke(std::forward<V>(visitor), specific_ptr<SingleEntryNode>());
				}
				case IdentifierTag::XN: {
					return std::invoke(std::forward<V>(visitor), specific_ptr<CartesianNode>());
				}
				case IdentifierTag::Indeterminate: {
					HYPERTRIE_UNREACHABLE;
				}
			}
		}

		[[nodiscard]] RawIdentifier<depth, htt_t> identifier() const noexcept {
			if (*this == nullptr) {
				return RawIdentifier<depth, htt_t>{};
			}

			switch (this->tag()) {
				case IdentifierTag::FN: {
					return specific_ptr<FullNode>()->identifier();
				}
				case IdentifierTag::SEN: {
					if constexpr (depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
						return RawIdentifier<1, htt_t>{SingleEntry<1, htt_t>{{this->decode_key_part()}, true}};
					} else {
						return specific_ptr<SingleEntryNode>()->identifier();
					}
				}
				case IdentifierTag::XN: {
					if constexpr (depth > 1) {
						return specific_ptr<CartesianNode>()->identifier();
					} else {
						HYPERTRIE_UNREACHABLE;
					}
				}
				case IdentifierTag::Indeterminate: [[unlikely]] {
					HYPERTRIE_UNREACHABLE;
				}
			}
		}

		[[nodiscard]] size_t size() const noexcept {
			if (*this == nullptr) {
				return 0;
			}

			switch (this->tag()) {
				case IdentifierTag::FN: {
					return specific_ptr<FullNode>()->size();
				}
				case IdentifierTag::SEN: {
					return 1;
				}
				case IdentifierTag::XN: {
					if constexpr (depth > 1) {
						return specific_ptr<CartesianNode>()->size();
					} else {
						HYPERTRIE_UNREACHABLE;
					}
				}
				case IdentifierTag::Indeterminate: {
					HYPERTRIE_UNREACHABLE;
				}
			}
		}

		[[nodiscard]] size_t ref_count() const noexcept {
			return visit_ptr([](auto node_ptr) noexcept {
				return node_ptr->ref_count();
			});
		}

		bool operator==(NodePtr const &other) const noexcept {
			return Base::template eq<key_part_inlining_possible>(this->tagged_ptr_, &other, other.tagged_ptr_);
		}

		bool operator!=(NodePtr const &other) const noexcept {
			return Base::template ne<key_part_inlining_possible>(this->tagged_ptr_, &other, other.tagged_ptr_);
		}

		bool operator==(std::nullptr_t) const noexcept {
			return Base::eq_null(this->tagged_ptr_);
		}

		bool operator!=(std::nullptr_t) const noexcept {
			return Base::ne_null(this->tagged_ptr_);
		}
	};

} // namespace dice::hypertrie::internal::raw

namespace dice::hash {
	template<typename Policy, ::dice::hypertrie::HypertrieTrait htt_t, ::dice::hypertrie::ByteAllocator allocator_type>
	struct dice_hash_overload<Policy, ::dice::hypertrie::internal::raw::RawNodePtr<htt_t, allocator_type>> {
		static std::size_t dice_hash(::dice::hypertrie::internal::raw::RawNodePtr<htt_t, allocator_type> const &node_ptr) noexcept {
			return node_ptr.template hash<Policy>();
		}
	};

	template<typename Policy, size_t depth, ::dice::hypertrie::HypertrieTrait htt_t, ::dice::hypertrie::ByteAllocator allocator_type>
	struct dice_hash_overload<Policy, ::dice::hypertrie::internal::raw::NodePtr<depth, htt_t, allocator_type>> {
		static std::size_t dice_hash(::dice::hypertrie::internal::raw::NodePtr<depth, htt_t, allocator_type> const &node_ptr) noexcept {
			return node_ptr.template hash<Policy>();
		}
	};
}// namespace dice::hash

#endif//HYPERTRIE_NODEPTR_HPP
