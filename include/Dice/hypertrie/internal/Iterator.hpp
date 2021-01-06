#ifndef HYPERTRIE_ITERATOR_HPP
#define HYPERTRIE_ITERATOR_HPP

#include "Dice/hypertrie/internal/raw/iterator/Iterator.hpp"
#include "Dice/hypertrie/internal/Hypertrie_predeclare.hpp"
#include "Dice/hypertrie/internal/old_hypertrie/BoolHypertrie.hpp"

namespace hypertrie {

	template<HypertrieTrait tr_t = default_bool_Hypertrie_t>
	struct Iterator {
		using tr = tr_t;
		using tri = internal::raw::Hypertrie_internal_t<tr>;
	protected:
		template<pos_type depth>
		using RawBoolHypertrie = typename hypertrie::internal::interface::rawboolhypertrie<typename tri::key_part_type, tri::template map_type, tri::template set_type>::template RawBoolHypertrie<depth>;

		using Key = typename tr::Key;
		using RawBaseIterator = internal::raw::base_iterator<tri>;
		template <size_t depth>
		using RawIterator = internal::raw::iterator<depth, tri>;
		struct RawMethods {

			void (*destruct)(const void *);

			void *(*begin)(const const_Hypertrie<tr> &boolHypertrie);

			const Key &(*value)(void const *);

			void (*inc)(void *);

			bool (*ended)(void const *);

		};

		template<pos_type depth>
		inline static RawMethods generateRawMethods() {
			return RawMethods{
					[](const void *rawboolhypertrie_iterator) {
					  using T = const typename RawBoolHypertrie<depth>::iterator;
					  if (rawboolhypertrie_iterator != nullptr){
						  delete static_cast<T *>(rawboolhypertrie_iterator);
					  }
					},
					[](const const_Hypertrie<tr> &boolHypertrie) -> void * {
					  return new typename RawBoolHypertrie<depth>::iterator(
							  *static_cast<RawBoolHypertrie<depth> const *>(boolHypertrie.rawNode()));
					},
					&RawBoolHypertrie<depth>::iterator::value,
					&RawBoolHypertrie<depth>::iterator::inc,
					&RawBoolHypertrie<depth>::iterator::ended};
		}

		inline static const std::vector<RawMethods> raw_method_cache = [](){
		  using namespace internal;
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

		// TODO: go on here


	protected:
		RawMethods const * raw_methods = nullptr;
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

		value_type operator*() const { return value_type(1); }

		operator bool() const { return not raw_methods->ended(&raw_iterator); }

	};

}


#endif//HYPERTRIE_ITERATOR_HPP
