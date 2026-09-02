#ifndef HYPERTRIE_SLICERESULT_HPP
#define HYPERTRIE_SLICERESULT_HPP


#include "dice/hypertrie/internal/raw/node/NodePtr.hpp"
#include "dice/hypertrie/internal/util/Unreachable.hpp"

#include <utility>

namespace dice::hypertrie::internal::raw {

	enum struct Ownership : uint8_t {
		Owned,             // hypertrie has to manage the lifetime of this node itself
		EphemeralBorrowed, // node is borrowed from somewhere but will probably not live very long
		ContextBorrowed,   // node is borrowed from the context and therefore expected to have a long lifetime
	};

	inline std::string to_string(Ownership const &value) noexcept {
		switch (value) {
			case Ownership::Owned: {
				return "owned";
			}
			case Ownership::EphemeralBorrowed: {
				return "ephemeral-borrowed";
			}
			case Ownership::ContextBorrowed: {
				return "context-borrowed";
			}
		}
	}

	template<size_t result_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct SliceResultStorage {
		SingleEntryNode<result_depth, htt_t> sen;
		CartesianNode<result_depth, htt_t, allocator_type> xn;
	};

	template<size_t result_depth, HypertrieTrait htt_t, ByteAllocator allocator_type>
	struct SliceResult {
	private:
		NodePtr<result_depth, htt_t, allocator_type> node_ptr_{};
		Ownership ownership_ = Ownership::ContextBorrowed;

		void drop() noexcept {
			if (ownership_ == Ownership::Owned && node_ptr_ != nullptr) {
				switch (node_ptr_.tag()) {
					case IdentifierTag::SEN: {
						if constexpr (result_depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
							delete std::to_address(node_ptr_.template specific_ptr<SingleEntryNode>());
						}
						break;
					}
					case IdentifierTag::XN: {
						delete std::to_address(node_ptr_.template specific_ptr<CartesianNode>());
						break;
					}
					default: {
						HYPERTRIE_UNREACHABLE;
					}
				}
			}
		}

	public:
		SliceResult() = default;

		SliceResult(Ownership ownership, SingleEntryNode<result_depth, htt_t> *sen_ptr) noexcept
			requires (result_depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
			: node_ptr_{sen_ptr},
			  ownership_{ownership} {
		}

		explicit SliceResult(typename htt_t::key_part_type key_part) noexcept
			requires (result_depth == 1 && HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>)
			: node_ptr_{NodePtr<1, htt_t, allocator_type>::encode_key_part(key_part)},
			  ownership_{Ownership::ContextBorrowed} {
		}

		SliceResult(Ownership ownership, CartesianNode<result_depth, htt_t, allocator_type> *xn_ptr) noexcept
			requires (result_depth > 1)
			: node_ptr_{xn_ptr},
			  ownership_{ownership} {
		}

		SliceResult(Ownership ownership, VoidPtr<allocator_type> void_node_ptr, IdentifierTag tag) noexcept
			: node_ptr_{void_node_ptr, tag},
			  ownership_{ownership} {
		}

		SliceResult(Ownership ownership, NodePtr<result_depth, htt_t, allocator_type> node_ptr) noexcept
			: node_ptr_{node_ptr},
			  ownership_{ownership} {
		}

		SliceResult(SliceResult const &other) = delete;
		SliceResult &operator=(SliceResult const &) = delete;

		SliceResult(SliceResult &&other) noexcept
			: node_ptr_{std::exchange(other.node_ptr_, {})},
			  ownership_{std::exchange(other.ownership_, Ownership::ContextBorrowed)} {
		}

		SliceResult &operator=(SliceResult &&other) noexcept {
			if (this == &other) {
				return *this;
			}

			drop();
			node_ptr_ = std::exchange(other.node_ptr_, {});
			ownership_ = std::exchange(other.ownership_, Ownership::ContextBorrowed);

			return *this;
		}

		~SliceResult() noexcept {
			drop();
		}

		[[nodiscard]] RawIdentifier<result_depth, htt_t> identifier() const noexcept {
			return node_ptr_.identifier();
		}

		[[nodiscard]] NodePtr<result_depth, htt_t, allocator_type> &node_ptr() noexcept {
			return node_ptr_;
		}

		[[nodiscard]] NodePtr<result_depth, htt_t, allocator_type> as_node_ptr() const noexcept {
			return node_ptr_;
		}

		/**
		 * Converts this slice result into a NodePtr, this call transfers ownership to the caller.
		 * This means the caller is now responsible for freeing the memory
		 * pointed to if necessary.
		 */
		[[nodiscard]] NodePtr<result_depth, htt_t, allocator_type> release_node_ptr() noexcept {
			ownership_ = Ownership::ContextBorrowed;
			return std::exchange(node_ptr_, {});
		}

		[[nodiscard]] bool holds_fn() const noexcept {
			return node_ptr_.is_fn();
		}

		[[nodiscard]] bool holds_sen() const noexcept {
			return node_ptr_.is_sen();
		}

		[[nodiscard]] bool holds_xn() const noexcept {
			return node_ptr_.is_xn();
		}

		[[nodiscard]] Ownership ownership() const noexcept {
			return ownership_;
		}

		[[nodiscard]] bool empty() const noexcept {
			return node_ptr_ == nullptr;
		}

		bool operator==(SliceResult const &other) const noexcept {
			return this->node_ptr_ == other.node_ptr_;
		}

		bool operator!=(SliceResult const &other) const noexcept {
			return this->node_ptr_ != other.node_ptr_;
		}

		SliceResult clone() const noexcept {
			if (ownership_ == Ownership::Owned || ownership_ == Ownership::EphemeralBorrowed) {
				switch (node_ptr_.tag()) {
					case IdentifierTag::SEN: {
						if constexpr (result_depth > 1 || !HypertrieTrait_bool_valued_and_taggable_key_part<htt_t>) {
							auto const sen_ptr = node_ptr_.template specific_ptr<SingleEntryNode>();
							return SliceResult{Ownership::Owned,
											   new SingleEntryNode<result_depth, htt_t>{*sen_ptr}};
						} else {
							HYPERTRIE_UNREACHABLE;
						}
					}
					case IdentifierTag::XN: {
						if constexpr (result_depth > 1) {
							auto const xn_ptr = node_ptr_.template specific_ptr<CartesianNode>();
							return SliceResult{Ownership::Owned,
											   new CartesianNode<result_depth, htt_t, allocator_type>{*xn_ptr}};
						} else {
							HYPERTRIE_UNREACHABLE;
						}
					}
					default: {
						HYPERTRIE_UNREACHABLE;
					}
				}
			} else {
				return SliceResult{ownership_, node_ptr_};
			}
		}
	};

}// namespace dice::hypertrie::internal::raw
#endif//HYPERTRIE_SLICERESULT_HPP
