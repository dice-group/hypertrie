#ifndef HYPERTRIE_NODEMODIFICATIONPLAN_HPP
#define HYPERTRIE_NODEMODIFICATIONPLAN_HPP

#include "Dice/hypertrie/internal/raw/Hypertrie_internal_traits.hpp"
#include "Dice/hypertrie/internal/raw/node/TensorHash.hpp"
#include "Dice/hypertrie/internal/raw/storage/Entry.hpp"

//from
//#include "Dice/hypertrie/internal/util/RobinHoodHash.hpp"
//to
#include "Dice/hash/DiceHash.hpp"


namespace hypertrie::internal::raw {


	enum struct ModificationOperations : unsigned int {
		NONE = 0,
		CHANGE_VALUE,
		NEW_COMPRESSED_NODE,
		NEW_UNCOMPRESSED_NODE,
		INSERT_INTO_COMPRESSED_NODE,
		INSERT_INTO_UNCOMPRESSED_NODE,
		REMOVE_FROM_UC,
	};

	template<size_t depth, HypertrieInternalTrait tri_t>
	class NodeModificationPlan {
		static_assert(depth >= 1);
	public:
		using tri = tri_t;
		/// public definitions
		using key_part_type = typename tri::key_part_type;
		using value_type = typename tri::value_type;
		using RawKey = typename tri::template RawKey<depth>;

		using red = RawEntry_t<depth, tri>;

		using Entry = typename red::RawEntry;

	private:

		ModificationOperations mod_op_{};
		TensorHash hash_before_{};
		mutable TensorHash hash_after_{};
		mutable std::vector<Entry> entries_{};

	public:
		ModificationOperations &modOp()  noexcept { return this->mod_op_;}

		const ModificationOperations &modOp() const noexcept { return this->mod_op_;}

		TensorHash &hashBefore()  noexcept { return this->hash_before_;}

		const TensorHash &hashBefore() const noexcept {
			return this->hash_before_;
		}

		TensorHash &hashAfter() noexcept {
			if (hash_after_.empty()) calcHashAfter();
			return this->hash_after_;
		}

		const TensorHash &hashAfter() const noexcept {
			if (hash_after_.empty()) calcHashAfter();
			return this->hash_after_;
		}

		std::vector<Entry> &entries() noexcept { return this->entries_;}

		const std::vector<Entry> &entries() const noexcept { return this->entries_;}



		void addEntry(Entry entry) noexcept {
			entries_.push_back(entry);
		}

		void addEntry(RawKey key, const value_type  value) noexcept {
			entries_.push_back(red::make_Entry(key, value));
		}

		void addKey(RawKey key) noexcept {
			entries_.push_back({key});
		}

		value_type &oldValue() noexcept {
			if constexpr (not tri::is_bool_valued){
				assert(mod_op_ == ModificationOperations::CHANGE_VALUE);
				entries_.resize(2);
				return red::value(entries_[1]);
			} else
				assert(false);
		}

		const value_type &oldValue() const noexcept {
			if constexpr (not tri::is_bool_valued){
				assert(mod_op_ == ModificationOperations::CHANGE_VALUE);
				entries_.resize(2);
				return red::value(entries_[1]);
			} else
				assert(false);
		}

		RawKey &firstKey() noexcept { return red::key(entries_[0]); }

		const RawKey &firstKey() const noexcept { return red::key(entries_[0]); }

		value_type &firstValue() noexcept  { return red::value(entries_[0]); }

		value_type firstValue() const noexcept  { return red::value(entries_[0]); }

	private:
		void calcHashAfter() const noexcept {
			hash_after_ = hash_before_;
			switch (mod_op_) {
				case ModificationOperations::CHANGE_VALUE:
					assert(not hash_before_.empty());
					assert(entries_.size() == 2);
					this->hash_after_.changeValue(firstKey(), oldValue(), firstValue());
					break;
				case ModificationOperations::NEW_COMPRESSED_NODE:{
					assert(hash_before_.empty());
					hash_after_ = TensorHash::getCompressedNodeHash(firstKey(), firstValue());
					break;
				}
				case ModificationOperations::INSERT_INTO_COMPRESSED_NODE:
					[[fallthrough]];
				case ModificationOperations::INSERT_INTO_UNCOMPRESSED_NODE:
					assert(not hash_before_.empty());
					for (const auto &entry : entries_)
						hash_after_.addEntry(red::key(entry), red::value(entry));
					break;
				case ModificationOperations::NEW_UNCOMPRESSED_NODE:
					assert(hash_before_.empty());
					assert(entries_.size() > 1);

					hash_after_ = TensorHash::getCompressedNodeHash(firstKey(), firstValue());
					for (auto entry_it = std::next(entries_.begin()); entry_it != entries_.end(); ++entry_it)
						hash_after_.addEntry(red::key(*entry_it), red::value(*entry_it));
					break;
				default:
					assert(false);
			}
		}

	public:

		bool operator<(const NodeModificationPlan &other) const noexcept {
			return std::make_tuple(this->mod_op_, this->hash_before_, this->hashAfter()) <
				   std::make_tuple(other.mod_op_, other.hash_before_, other.hashAfter());
		};

		bool operator==(const NodeModificationPlan &other) const noexcept {
			return std::make_tuple(this->mod_op_, this->hash_before_, this->hashAfter()) ==
				   std::make_tuple(other.mod_op_, other.hash_before_, other.hashAfter());
		};

		friend struct std::hash<NodeModificationPlan>;
	};

}

template<size_t depth, hypertrie::internal::raw::HypertrieInternalTrait tri>
struct std::hash<hypertrie::internal::raw::NodeModificationPlan<depth, tri>> {
	size_t operator()(const hypertrie::internal::raw::NodeModificationPlan<depth, tri> &update) const noexcept {
		return dice::hash::dice_hash(std::make_tuple(update.hashBefore(), update.hashAfter()));
	}
};
#endif//HYPERTRIE_NODEMODIFICATIONPLAN_HPP
