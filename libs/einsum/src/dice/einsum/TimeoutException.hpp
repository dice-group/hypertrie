#ifndef HYPERTRIE_TIMEOUTEXCEPTION_HPP
#define HYPERTRIE_TIMEOUTEXCEPTION_HPP

#include "dice/einsum/Subscript.hpp"

#include <chrono>

namespace dice::einsum {

	class TimeoutException : public std::runtime_error {
		using DurationType = std::chrono::steady_clock::duration;

		static constexpr std::size_t countMS(DurationType const& duration) noexcept {
			return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		}

		static constexpr std::size_t countS(DurationType const& duration) noexcept {
			return std::chrono::duration_cast<std::chrono::seconds>(duration).count();
		}

	public:
		explicit TimeoutException(DurationType const &duration) noexcept
			: std::runtime_error(
					  std::string{"Evaluation of einsum timed out after "} +
					  duration_string(duration) +
					  ".") {}

		static std::string duration_string(DurationType const &duration) {
			if (countMS(duration) >= 10'000) {
				return std::to_string(countS(duration)) + " s";
			}
			return std::to_string(countMS(duration)) + "ms";
		}
	};
}// namespace dice::einsum

#endif//HYPERTRIE_TIMEOUTEXCEPTION_HPP
