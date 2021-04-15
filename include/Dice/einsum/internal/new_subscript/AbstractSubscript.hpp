#ifndef HYPERTRIE_ABSTRACTSUBSCRIPT_HPP
#define HYPERTRIE_ABSTRACTSUBSCRIPT_HPP

#include <string>

namespace einsum::internal::new_subscript {
	class AbstractSubscript {
	public:
		virtual ~AbstractSubscript() = default;
		virtual std::string str() const = 0;
	};
}// namespace einsum::internal::new_subscript

#endif//HYPERTRIE_ABSTRACTSUBSCRIPT_HPP
