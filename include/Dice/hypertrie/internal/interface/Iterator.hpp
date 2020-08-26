#ifndef HYPERTRIE_ITERATOR_HPP
#define HYPERTRIE_ITERATOR_HPP

#include "Dice/hypertrie/internal/raw/iterator/Iterator.hpp"
#include "Dice/hypertrie/internal/interface/Hypertrie_predeclare.hpp"

namespace hypertrie::internal::node_based {

	template<HypertrieTrait tr_t = default_bool_Hypertrie_t>
	struct Iterator {
		using tr = tr_t;
		using tri = raw::Hypertrie_internal_t<tr>;
	protected:
		using Key = typename tr::Key;
		using RawBaseIterator = raw::base_iterator<tri>;
		template <size_t depth>
		using RawIterator = raw::iterator<depth, tri>;
		struct RawMethods {

			RawBaseIterator (*begin)(const const_Hypertrie<tr> &hypertrie) = nullptr;

			const Key &(*value)(void const *) =  nullptr;

			void (*inc)(void *) =  nullptr;

			bool (*ended)(void const *) =  nullptr;

			RawMethods() {}

			RawMethods(RawBaseIterator (*begin)(const const_Hypertrie<tr> &), const Key &(*value)(const void *), void (*inc)(void *), bool (*ended)(const void *)) : begin(begin), value(value), inc(inc), ended(ended) {}
		};

		template<pos_type depth>
		inline static RawMethods generateRawMethods() {
			return RawMethods(
					[](const const_Hypertrie<tr> &hypertrie) -> RawBaseIterator {
					  return (RawBaseIterator) RawIterator<depth>{
								*const_cast<raw::NodeContainer<depth, tri> *>(reinterpret_cast<const raw::NodeContainer<depth, tri> *>(hypertrie.rawNodeContainer())),
							  hypertrie.context()->rawContext()};
					},
					&RawIterator<depth>::value,
					&RawIterator<depth>::inc,
					&RawIterator<depth>::ended);
		}

		inline static const std::vector<RawMethods> raw_method_cache = [](){
		  std::vector<RawMethods> raw_methods;
		  for(size_t depth : iter::range(1UL,hypertrie_depth_limit))
			  raw_methods.push_back(compiled_switch<hypertrie_depth_limit, 1>::switch_(
					  depth,
					  [](auto depth_arg) -> RawMethods {
						return generateRawMethods<depth_arg>();
					  },
					  []() -> RawMethods { assert(false); throw std::logic_error{"Something is really wrong."}; }));
		  return raw_methods;
		}();

		static RawMethods const &getRawMethods(pos_type depth) {
			return raw_method_cache[depth - 1];
		};


	protected:
		RawMethods const * const raw_methods = nullptr;
		RawBaseIterator raw_iterator;

	public:
		using self_type = Iterator;
		using value_type = Key;

		Iterator() = default;

		Iterator(const const_Hypertrie<tr> &hypertrie) :
				raw_methods(&getRawMethods(hypertrie.depth())),
				raw_iterator(raw_methods->begin(hypertrie)) {}

		Iterator(const_Hypertrie<tr> &hypertrie) : Iterator(&hypertrie) {}

		self_type &operator++() {
			raw_methods->inc(&raw_iterator);
			return *this;
		}

		value_type operator*() const { return raw_methods->value(&raw_iterator); }

		operator bool() const { return not raw_methods->ended(&raw_iterator); }

	};

}


#endif//HYPERTRIE_ITERATOR_HPP
