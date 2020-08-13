#ifndef HYPERTRIE_HYPERTRIE_HPP
#define HYPERTRIE_HYPERTRIE_HPP

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

#include "Dice/hypertrie/internal/node_based/interface/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/interface/HypertrieContext.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <optional>
#include <variant>
#include <vector>

namespace hypertrie::internal::node_based {

	template<typename tr>
	class const_Hypertrie {
	public:
		using tri = raw::Hypertrie_internal_t<tr>;

	private:
		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

	protected:
		void *node_container_ = nullptr;

		HypertrieContext<tr> *context_ = nullptr;

		size_t depth_ = 0;

		const_Hypertrie(size_t depth, HypertrieContext<tr> *context, void *raw_hypertrie) : node_container_(raw_hypertrie), context_(context), depth_(depth) {}

	public:
		explicit const_Hypertrie(size_t depth) : depth_(depth) {}
		void *rawNodeContainer() const {
			return node_container_;
		}
		HypertrieContext<tr> *context() const {
			return context_;
		}
		size_t depth() const {
			return depth_;
		}


		using traits = tr;

		using Key = typename tr::Key;
		using SliceKey = typename tr::SliceKey;
		using value_type = typename tr::value_type;

		[[nodiscard]] std::vector<size_t> getCards(const std::vector<pos_type> &positions) const;

		[[nodiscard]] size_t size() const;

		[[nodiscard]] constexpr bool empty() const noexcept {
			return this->node_container_ == nullptr;
		}

		[[nodiscard]] value_type operator[](const Key &key) const {
			return compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg) mutable -> bool {
						RawKey<depth_arg> raw_key;
						std::copy_n(key.begin(), depth_arg, raw_key.begin());
						const auto &node_container = *static_cast<const raw::NodeContainer<depth_arg, tri> *>(this->node_container_);
						return this->context()->rawContext().template get<depth_arg>(
								node_container,
								raw_key);
					},
					[]() -> bool { assert(false); return {}; });
		}



		[[nodiscard]]
		std::variant<std::optional<const_Hypertrie>, value_type> operator[](const SliceKey &slice_key) const;

		class iterator;

		using const_iterator = iterator;

		[[nodiscard]]
		iterator begin() const { return iterator{this}; }

		[[nodiscard]]
		const_iterator cbegin() const { return iterator{this}; }

		[[nodiscard]]
		bool end() const { return false; }

		[[nodiscard]]
		bool cend() const { return false; }

	};

	template<typename tr = default_bool_Hypertrie_t>
	class Hypertrie : public const_Hypertrie<tr> {
		;
	public:
		using traits = tr;
		using tri = raw::Hypertrie_internal_t<tr>;
		using Key = typename tr::Key;
		using value_type = typename tr::value_type;

	private:
		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;

	public:
		value_type set(const Key &key, value_type value) {
			return compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg) -> value_type {
						RawKey<depth_arg> raw_key;
						std::copy_n(key.begin(), depth_arg, raw_key.begin());
						auto &node_container = *static_cast<raw::NodeContainer<depth_arg, tri> *>(this->node_container_);
						return this->context_->rawContext().template set<depth_arg>(node_container, raw_key, value);
					},
					[]() -> value_type { assert(false); return {}; });
		}

		Hypertrie(const const_Hypertrie<tr> &hypertrie) : const_Hypertrie<tr>(hypertrie) {
			// TODO: increase reference counter
			compiled_switch<hypertrie_depth_limit, 1>::switch_void(
					this->depth_,
					[&](auto depth_arg){ this->context_->template incRefCount<depth_arg>(reinterpret_cast<raw::NodeContainer<depth_arg, tri> *>(this->node_container_));},
					[&](auto depth_arg){ throw std::logic_error{fmt::format("Hypertrie depth {} invalid. allowed range: [1,{})", depth_arg, hypertrie_depth_limit)}; }
					);
		}

		Hypertrie(size_t depth, HypertrieContext<tr> &context = DefaultHypertrieContext<tr>::instance())
			: const_Hypertrie<tr>(depth, &context,
								  compiled_switch<hypertrie_depth_limit, 1>::switch_(
										  depth,
										  [&](auto depth_arg) -> void * { return (void *) new raw::NodeContainer<depth_arg, tri>; },
										  [&]() -> void * { throw std::logic_error{fmt::format("Hypertrie depth {} invalid. allowed range: [1,{})", depth, hypertrie_depth_limit)}; })) {}
	};

}

#endif //HYPERTRIE_HYPERTRIE_HPP
