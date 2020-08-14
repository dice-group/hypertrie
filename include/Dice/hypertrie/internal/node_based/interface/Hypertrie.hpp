#ifndef HYPERTRIE_HYPERTRIE_HPP
#define HYPERTRIE_HYPERTRIE_HPP

#include "Dice/hypertrie/internal/container/AllContainer.hpp"

#include "Dice/hypertrie/internal/node_based/interface/Hypertrie_traits.hpp"
#include "Dice/hypertrie/internal/node_based/interface/HypertrieContext.hpp"
#include "Dice/hypertrie/internal/node_based/raw/iterator/Iterator.hpp"
#include "Dice/hypertrie/internal/util/CONSTANTS.hpp"
#include <optional>
#include <variant>
#include <vector>
#include <itertools.hpp>

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

		class iterator {
		protected:
			struct RawMethods {

				static void (*destruct)(const void *);

				static void *(*begin)(const const_Hypertrie &boolHypertrie);

				static const Key &(*value)(void const *);

				static void (*inc)(void *);

				static bool (*ended)(void const *);

			};

			template<pos_type depth>
			inline static RawMethods generateRawMethods() {
				using RawIterator = typename raw::iterator<depth, raw::NodeCompression::uncompressed, tri>;
				return RawMethods{
						[](const void *raw_hypertrie_iterator) {
						  if (raw_hypertrie_iterator != nullptr){
							  delete static_cast<const RawIterator *>(raw_hypertrie_iterator);
						  }
						},
						[](const const_Hypertrie &hypertrie) -> void * {
						  return new RawIterator(
								  static_cast<raw::UncompressedNodeContainer<depth, tri> *>(hypertrie.rawNodeContainer()),
								  static_cast<raw::UncompressedNodeContainer<depth, tri> *>(hypertrie.context()->rawContext())
									);
						},
						&RawIterator::value,
						&RawIterator::inc,
						&RawIterator::ended};
			}

			inline static const std::vector<RawMethods> raw_method_cache_uncompressed = [](){
				std::vector<RawMethods> raw_methods;
				for(size_t depth : iter::range(1UL,hypertrie_depth_limit))
					raw_methods.push_back(compiled_switch<hypertrie_depth_limit, 1>::switch_(
							depth,
							[&](auto depth_arg) -> RawMethods {
							  return generateRawMethods<depth_arg>();
							},
							[]() -> RawMethods { assert(false); return {}; }));
			}();

			static RawMethods const &getRawMethodsUncompressed(pos_type depth) {
				return raw_method_cache_uncompressed[depth - 1];
			};


		protected:
			RawMethods const *raw_methods = nullptr;
			void *raw_iterator = nullptr;

		public:
			using self_type =  iterator;
			using value_type = Key;

			iterator() = default;

			iterator(iterator &) = delete;

			iterator(const iterator &) = delete;

			iterator(iterator &&) = delete;

			iterator &operator=(iterator &&other) noexcept {
				if (this->raw_methods != nullptr)
					this->raw_methods->destruct(this->raw_iterator);
				this->raw_methods = other.raw_methods;
				this->raw_iterator = other.raw_iterator;
				other.raw_iterator = nullptr;
				other.raw_methods = nullptr;
				return *this;
			}

			iterator &operator=(iterator &) = delete;

			iterator &operator=(const iterator &) = delete;

			iterator(const_Hypertrie const *const hypertrie) :
					raw_methods(&getRawMethodsUncompressed(hypertrie->depth())),
					raw_iterator(raw_methods->begin(*hypertrie)) {}

			iterator(const_Hypertrie &hypertrie) : iterator(&hypertrie) {}

			~iterator() {
				if (raw_methods != nullptr)
					raw_methods->destruct(raw_iterator);
				raw_methods = nullptr;
				raw_iterator = nullptr;
			}

			self_type &operator++() {
				raw_methods->inc(raw_iterator);
				return *this;
			}

			value_type operator*() const { return raw_methods->value(raw_iterator); }

			operator bool() const { return not raw_methods->ended(raw_iterator); }

		};

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
