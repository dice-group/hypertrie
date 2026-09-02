#ifndef HYPERTRIE_OVERLOADED_HPP
#define HYPERTRIE_OVERLOADED_HPP

namespace dice::hypertrie::internal::util {

	template<typename ...Fs>
	struct Overloaded : Fs... {
		using Fs::operator()...;
	};

	template<typename ...Fs>
	Overloaded(Fs...) -> Overloaded<Fs...>;

} // namespace dice::hypertrie::internal::util

#endif//HYPERTRIE_OVERLOADED_HPP
