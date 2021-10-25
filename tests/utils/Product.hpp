#ifndef HYPERTRIE_PRODUCT_HPP
#define HYPERTRIE_PRODUCT_HPP

#include <array>
#include <random>
#include <itertools.hpp>

namespace Dice::hypertrie::tests::utils {
	namespace {
		using namespace iter;
	}

/**
 * Generates a std::array of length count that contains deterministic n-tuples. 
 * Calling the function twice will return exactly the same n-tuples. 
 * The range for the j-th entry of a n-triple is [0, J]
 * where J is ranges[j]. 
 * @tparam value_type  the type of the entries of the n-tuples
 * @tparam count the number of n-tuples
 * @tparam n the length of the tuples
 * @tparam ranges the maximum values for the positions of the n-tuples.
 * @return an array of n-tuples
 */
	template<typename V, typename T = typename V::value_type>
	struct Product {
		size_t size;
		V candidates;

		Product(const size_t &size, V candidates) : size(size), candidates(std::move(candidates)) {}

		class iterator {
		public:
			using value_type = std::vector<T>;
			Product *container;
			std::vector<typename V::const_iterator> iters;
			typename V::const_iterator end;
			value_type value;
			bool ended;

		public:

			explicit iterator(Product *container) :
					container(container),
					iters(container->size, container->candidates.cbegin()),
					end(container->candidates.cend()),
					value(container->size, *container->candidates.cbegin()),
					ended(not container->candidates.size()) {}

			const value_type &operator*() {
				return value;
			}

			void operator++() {
				for (const auto i: iter::range(iters.size())) {
					++iters[i];
					if (iters[i] != end) {
						value[iters.size() - 1 - i] = *iters[i];
						return;
					} else {
						iters[i] = container->candidates.cbegin();
						value[iters.size() - 1 - i] = *iters[i];
					}
				}
				ended = true;
			}

			operator bool() { return not ended; }

		};

		decltype(auto) begin() {
			return iterator(this);
		}

		decltype(auto) end() {
			return false;
		}

	};

	template<typename T>
	decltype(auto) product(const size_t size, const T excl_max, const T min = 0) {
		return Product{size, std::vector<T>{iter::range(min, excl_max).begin(), iter::range(min, excl_max).end()}};
	}


}
#endif //HYPERTRIE_PRODUCT_HPP
