#ifndef HYPERTRIE_HYPERTRIE_HPP
#define HYPERTRIE_HYPERTRIE_HPP

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

#include "Dice/hypertrie/internal/HashDiagonal.hpp"
#include "Dice/hypertrie/internal/HypertrieContext.hpp"
#include "Dice/hypertrie/internal/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/Iterator.hpp"
#include "Dice/hypertrie/internal/old_hypertrie/BoolHypertrie.hpp"

#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <optional>
#include <variant>
#include <vector>
#include <itertools.hpp>

namespace hypertrie {

	template<HypertrieTrait tr>
	class const_Hypertrie {
	public:
		using tri = internal::raw::Hypertrie_internal_t<tr>;

	private:

		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

		typedef typename internal::raw::RawNodeContainer NodeContainer;

	protected:
		template<pos_type depth>
		using RawBoolHypertrie = typename hypertrie::internal::interface::rawboolhypertrie<typename tri::key_part_type, tri::template map_type, tri::template set_type>::template RawBoolHypertrie<depth>;
		template<pos_type depth, pos_type diag_depth>
		using RawHashDiagonal = typename hypertrie::internal::interface::rawboolhypertrie<typename tri::key_part_type, tri::template map_type, tri::template set_type>::template RawHashDiagonal<diag_depth, depth>;


		std::shared_ptr<void> hypertrie_;

		size_t depth_ = 0;

		explicit const_Hypertrie(size_t depth, const std::shared_ptr<void> & hypertrie = {}) : hypertrie_(hypertrie),depth_(depth) {}

		constexpr bool contextless() const noexcept {
			return false;
		}

		friend class HashDiagonal<tr>;


		void destruct_contextless_node() noexcept {
			// there are no contextless nodes
		}

		void copy_contextless_node() noexcept {
			// there are no contextless nodes
		}
	public:
		~const_Hypertrie() {
		}

		const_Hypertrie(const_Hypertrie &&const_hypertrie)
				: hypertrie_(std::move(const_hypertrie.hypertrie_)), depth_(const_hypertrie.depth_) {
			const_hypertrie.hypertrie_.reset();
		}

		const_Hypertrie &operator=(const const_Hypertrie &const_hypertrie) noexcept {
			this->hypertrie_ = const_hypertrie.hypertrie_;
			this->depth_ = const_hypertrie.depth_;
			return *this;
		}


		const_Hypertrie &operator=(const_Hypertrie &&const_hypertrie) noexcept {
			this->hypertrie_ = std::move(const_hypertrie.hypertrie_);
			const_hypertrie.hypertrie_.reset();
			this->depth_ = const_hypertrie.depth_;
			return *this;
		}

		const_Hypertrie(const const_Hypertrie &const_hypertrie)
			: hypertrie_(const_hypertrie.hypertrie_), depth_(const_hypertrie.depth_) {
			copy_contextless_node();
		}

		const_Hypertrie() = default;

		void *rawNode() const {
			return this->hypertrie_.get();
		}

		size_t depth() const {
			return depth_;
		}

		using traits = tr;

		using Key = typename tr::Key;
		using SliceKey = typename tr::SliceKey;
		using value_type = typename tr::value_type;

		[[nodiscard]] size_t size() const{
			if (not this->hypertrie_)
				return 0;
			else
				return internal::compiled_switch<hypertrie_depth_limit, 1>::switch_(
						this->depth_,
						[&](auto depth_arg) -> size_t {
						  	return std::static_pointer_cast<RawBoolHypertrie<depth_arg>>(this->hypertrie_)->size();
						},
						[]() -> size_t { assert(false); return 0; });
		}

		[[nodiscard]] constexpr bool empty() const noexcept {
			return this->size() == 0UL;
		}

		[[nodiscard]] value_type operator[](const Key &key) const {
			if (empty()){
				return value_type(0);
			} else {
			return internal::compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg) mutable -> bool {
						RawKey<depth_arg> raw_key;
						std::copy_n(key.begin(), depth_arg, raw_key.begin());
					  return std::static_pointer_cast<RawBoolHypertrie<depth_arg>>(this->hypertrie_)->operator[](raw_key);
					},
					[]() -> bool { assert(false); return {}; });
			}
		}

	protected:
		template<pos_type depth>
		inline static std::tuple<typename RawBoolHypertrie<depth>::SliceKey, pos_type>
		extractRawSliceKey(const SliceKey &slice_key) {
			typename RawBoolHypertrie<depth>::SliceKey raw_slice_key;
			std::copy_n(slice_key.begin(), depth, raw_slice_key.begin());
			return {raw_slice_key, std::count(slice_key.begin(), slice_key.end(), std::nullopt)};
		}

		template<pos_type depth, pos_type result_depth>
		inline static auto
		executeRawSlice(const std::shared_ptr<void> &hypertrie,
						typename RawBoolHypertrie<depth>::SliceKey raw_slice_key)
		-> std::conditional_t<(result_depth > 0), std::shared_ptr<const_Hypertrie>, bool> {
			auto raw_hypertrie = std::static_pointer_cast<RawBoolHypertrie<depth>>(hypertrie);
			auto result = raw_hypertrie->template operator[]<result_depth>(raw_slice_key);
			if (result)
				return instance(raw_hypertrie->template operator[]<result_depth>(raw_slice_key));
			else
				return std::nullopt;
		}

	public:


		[[nodiscard]] std::variant<const_Hypertrie, value_type> operator[](const SliceKey &slice_key) const {
			assert(slice_key.size() == depth());
			const size_t fixed_depth = tri::sliceKeyFixedDepth(slice_key);

			if (fixed_depth == depth()) {
				Key key(slice_key.size());
				for (auto [key_part, slice_key_part] : iter::zip(key, slice_key))
					key_part = slice_key_part.value();
				return this->operator[](key);
			} else if (fixed_depth == 0) {
				return const_Hypertrie(*this);
			} else if (this->size() == 0UL) {
				return const_Hypertrie(this->depth() - fixed_depth);
			} else {
				std::variant<const_Hypertrie, value_type> result;
				internal::compiled_switch<hypertrie_depth_limit, 1>::switch_void(
						this->depth_,
						[&](auto depth_arg) {
							return internal::compiled_switch<depth_arg, 1>::switch_void(
									fixed_depth,
									[&](auto slice_key_depth_arg) {
										typename RawBoolHypertrie<depth_arg>::SliceKey raw_slice_key;
										std::copy_n(slice_key.begin(), depth_arg, raw_slice_key.begin());
										auto *raw_hypertrie = static_cast<RawBoolHypertrie<depth_arg> *>(this->hypertrie_.get());

										if constexpr (slice_key_depth_arg != depth_arg and depth_arg != 1) {
											result = const_Hypertrie{depth_arg - slice_key_depth_arg,
																	 std::move(raw_hypertrie->template operator[]<depth_arg - slice_key_depth_arg>(raw_slice_key))};
										}
									});
						});
				return result;
			}
		}

		[[nodiscard]]
		std::vector<size_t> getCards(const std::vector<pos_type> &positions) const {
			assert(positions.size() <= depth());
			if (positions.size() == 0) // no positions provided
				return {};
			else if (empty()) {
				return std::vector<size_t>(positions.size(), 0);
			} if (depth() == 1){
				return {size()};
			} else if (size() == 1) {
				return std::vector<size_t>(positions.size(),1);
			} else {
				return internal::compiled_switch<hypertrie_depth_limit, 2>::switch_(
						this->depth_,
						[&](auto depth_arg) -> std::vector<size_t> {
							auto raw_hypertrie = std::static_pointer_cast<RawBoolHypertrie<depth_arg>>(this->hypertrie_);
							return raw_hypertrie->getCards(positions);
						},
						[&]() -> std::vector<size_t> {
							assert(false);
							return {};
						});
			}
		}

		using iterator = Iterator<tr>;
		using const_iterator = iterator;

		[[nodiscard]]
		iterator begin() const { return iterator{*this}; }

		[[nodiscard]]
		const_iterator cbegin() const { return iterator{*this}; }

		[[nodiscard]]
		bool end() const { return false; }

		[[nodiscard]]
		bool cend() const { return false; }

		[[nodiscard]]
		bool operator==(const const_Hypertrie<tr> &other) const noexcept {
			return this->hypertrie_ == other.hypertrie_ and this->depth() == other.depth();
		}

		[[nodiscard]]
		bool operator==(const Hypertrie<tr> &other) const noexcept {
			return this->hypertrie_ == other.hypertrie_ and this->depth() == other.depth();
		}

		operator std::string() const {
			std::vector<std::string> mappings;
			for (const auto &entry : *this){
				if constexpr (tr::is_bool_valued)
					mappings.push_back(fmt::format("⟨{}⟩ → true", fmt::join(entry,", ")));
				else
					mappings.push_back(fmt::format("⟨{}⟩ → {}", fmt::join(entry.first,", "), entry.second));
			}
			return fmt::format("[ {} ]", fmt::join(mappings, ", "));
		}

	};

	template<HypertrieTrait tr>
	class Hypertrie : public const_Hypertrie<tr>{
	public:
		using traits = tr;
		using tri = internal::raw::Hypertrie_internal_t<tr>;
		using Key = typename tr::Key;
		using value_type = typename tr::value_type;

	private:
		template<pos_type depth>
		using RawBoolHypertrie = typename hypertrie::internal::interface::rawboolhypertrie<typename tri::key_part_type, tri::template map_type, tri::template set_type>::template RawBoolHypertrie<depth>;
		template<pos_type depth, pos_type diag_depth>
		using RawHashDiagonal = typename hypertrie::internal::interface::rawboolhypertrie<typename tri::key_part_type, tri::template map_type, tri::template set_type>::template RawHashDiagonal<diag_depth, depth>;

		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

	public:
		value_type set(const Key &key, value_type value) {
			return internal::compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg) -> value_type {
					  	auto raw_hypertrie = std::static_pointer_cast<RawBoolHypertrie<depth_arg>>(this->hypertrie_);
						RawKey<depth_arg> raw_key;
						std::copy_n(key.begin(), depth_arg, raw_key.begin());
						if constexpr (depth_arg == 1)
					  		raw_hypertrie->set(raw_key[0], value);
						else
							raw_hypertrie->set(raw_key, value);
						return false; // return dummy value
					},
					[]() -> value_type { assert(false); return {}; });
		}

		Hypertrie(const Hypertrie<tr> &hypertrie) : const_Hypertrie<tr>(hypertrie) {}

		Hypertrie(const const_Hypertrie<tr> &hypertrie) : const_Hypertrie<tr>(hypertrie) {}

		Hypertrie(Hypertrie<tr> &&other) : const_Hypertrie<tr>(std::move(other)) {}

		Hypertrie(size_t depth = 1)
			: const_Hypertrie<tr>(depth) {
			this->hypertrie_ = internal::compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg) -> std::shared_ptr<void> {
						return std::make_shared<RawBoolHypertrie<depth_arg>>();
					},
					[]() -> std::shared_ptr<void> { assert(false); return {}; });
		}
	};

}

#endif //HYPERTRIE_HYPERTRIE_HPP
