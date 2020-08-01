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

		void* node_container_;

		HypertrieContext<tr> *context_;

		size_t depth_;

		const_Hypertrie(size_t depth, HypertrieContext<tr> *context, void *raw_hypertrie) : node_container_(raw_hypertrie), context_(context), depth_(depth)  {}

	public:


		using traits = tr;

		using Key = typename tr::Key;
		using SliceKey = typename tr::SliceKey;
		using value_type = typename  tr::value_type;

		[[nodiscard]]
		std::vector<size_t> getCards(const std::vector<pos_type> &positions) const;

		[[nodiscard]]
		size_t size() const;

		[[nodiscard]]
		bool operator[](const Key &key) const{
			return compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg)  {
					  RawKey<depth_arg> raw_key;
					  std::copy_n(key.begin(), depth_arg, raw_key.begin());
					  const auto &node_container = *static_cast<const raw::NodeContainer<depth_arg, tri>*>(this->node_container_);
					  this->context_->raw_context.get(
							  node_container,
							  raw_key);
					},
					[]() {});
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
		using value_type = typename  tr::value_type;

	private:
		template<size_t depth>
		using RawKey = typename tri::template RawKey<depth>;

		template<size_t depth>
		using RawSliceKey = typename tri::template RawSliceKey<depth>;
	public:

		void set(const Key &key, value_type value) {
			compiled_switch<hypertrie_depth_limit, 1>::switch_(
					this->depth_,
					[&](auto depth_arg)  {
						RawKey<depth_arg> raw_key;
					  	std::copy_n(key.begin(), depth_arg, raw_key.begin());
					  	this->context_->raw_context.set(raw_key, value);
					 },
					[]() {});
		}

		Hypertrie(size_t depth, HypertrieContext<tr> &context = DefaultHypertrieContext<tr>::instance())
			: const_Hypertrie<tr>(depth, &context,
							  compiled_switch<hypertrie_depth_limit, 1>::switch_(
									  depth,
									  [&](auto depth_arg) -> void * {
										  return new raw::NodeContainer<depth_arg, tri>; },
									  [&]() -> void * { throw std::logic_error{fmt::format("Hypertrie depth {} invalid. allowed range: [1,{})", depth, hypertrie_depth_limit)}; })
							  ) {}
	};

}

#endif //HYPERTRIE_HYPERTRIE_HPP
