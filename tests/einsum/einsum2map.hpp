#ifndef HYPERTRIE_EINSUM2MAP_HPP
#define HYPERTRIE_EINSUM2MAP_HPP

#include <chrono>
#include <memory>
#include <vector>

#include <ankerl/unordered_dense.h>

#include <dice/einsum.hpp>
namespace dice::einsum::tests {

	template<typename value_type = std::size_t, hypertrie::HypertrieTrait htt_t = hypertrie::default_bool_Hypertrie_trait, hypertrie::ByteAllocator allocator_type = std::allocator<std::byte>>
	static auto
	einsum2map(std::shared_ptr<Subscript> const &subscript,
			   std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
			   std::chrono::steady_clock::time_point end_time = internal::Context::time_point::max()) {
		using Key_t = Key<value_type, htt_t>;

		struct HashKey {
			using is_avalanching = void;

			inline size_t operator()(Key_t const &key) const noexcept {
				return hash::dice_hash_templates<hash::Policies::wyhash>::dice_hash(key);
			}
		};

		::ankerl::unordered_dense::map<Key_t, value_type, HashKey> results{};
		for (auto const &operand : operands) {
			if (operand.size() == 0) {
				return results;
			}
		}
		for (auto const &entry : einsum<value_type, htt_t, allocator_type>(subscript, operands, end_time)) {
			results[entry.key()] += entry.value();
		}
		return results;
	}
}// namespace dice::einsum::tests
#endif//HYPERTRIE_EINSUM2MAP_HPP
