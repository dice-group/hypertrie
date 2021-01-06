#ifndef HYPERTRIE_MARTINUS_ROBINHOOD_HASH_HPP
#define HYPERTRIE_MARTINUS_ROBINHOOD_HASH_HPP
//                 ______  _____                 ______                _________
//  ______________ ___  /_ ___(_)_______         ___  /_ ______ ______ ______  /
//  __  ___/_  __ \__  __ \__  / __  __ \        __  __ \_  __ \_  __ \_  __  /
//  _  /    / /_/ /_  /_/ /_  /  _  / / /        _  / / // /_/ // /_/ // /_/ /
//  /_/     \____/ /_.___/ /_/   /_/ /_/ ________/_/ /_/ \____/ \____/ \__,_/
//                                      _/_____/
//
// Fast & memory efficient hashtable based on robin hood hashing for C++11/14/17/20
// https://github.com/martinus/robin-hood-hashing
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2020 Martin Ankerl <http://martin.ankerl.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
namespace Dice::hash::martinus {

	inline constexpr size_t m = 0xc6a4a7935bd1e995;
	inline constexpr size_t seed = 0xe17a1465;
	inline constexpr unsigned int r = 47;


	template<typename T>
	inline T unaligned_load(void const *ptr) noexcept {
		// using memcpy so we don't get into unaligned load problems.
		// compiler should optimize this very well anyways.
		T t;
		std::memcpy(&t, ptr, sizeof(T));
		return t;
	}

	template<typename T>
	inline T rotr(T x, unsigned k) {
		return (x >> k) | (x << (8U * sizeof(T) - k));
	}

	inline size_t hash_bytes(void const *ptr, size_t len) noexcept {


		static constexpr unsigned int r = 47;

		auto const *const data64 = static_cast<uint64_t const *>(ptr);
		uint64_t h = seed ^ (len * m);

		size_t const n_blocks = len / 8;
		for (size_t i = 0; i < n_blocks; ++i) {
			auto k = unaligned_load<uint64_t>(data64 + i);

			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}

		auto const *const data8 = reinterpret_cast<uint8_t const *>(data64 + n_blocks);
		switch (len & 7U) {
			case 7:
				h ^= static_cast<uint64_t>(data8[6]) << 48U;
				[[fallthrough]];
			case 6:
				h ^= static_cast<uint64_t>(data8[5]) << 40U;
				[[fallthrough]];
			case 5:
				h ^= static_cast<uint64_t>(data8[4]) << 32U;
				[[fallthrough]];
			case 4:
				h ^= static_cast<uint64_t>(data8[3]) << 24U;
				[[fallthrough]];
			case 3:
				h ^= static_cast<uint64_t>(data8[2]) << 16U;
				[[fallthrough]];
			case 2:
				h ^= static_cast<uint64_t>(data8[1]) << 8U;
				[[fallthrough]];
			case 1:
				h ^= static_cast<uint64_t>(data8[0]);
				h *= m;
				[[fallthrough]];
			default:
				break;
		}

		h ^= h >> r;
		h *= m;
		h ^= h >> r;
		return static_cast<size_t>(h);
	}

	inline size_t hash_combine(std::initializer_list<size_t> hashes) {


		uint64_t h = seed ^ (hashes.size() * m);

		for (auto k : hashes) {
			k *= m;
			k ^= k >> r;
			k *= m;

			h ^= k;
			h *= m;
		}

		h ^= h >> r;
		h *= m;
		h ^= h >> r;
		return static_cast<size_t>(h);
	}

	class HashState {

		size_t h;

	public:
		HashState(uint64_t size) : h(seed ^ (size * m)) {}

		void add(size_t hash) noexcept {
			hash *= m;
			hash ^= hash >> r;
			hash *= m;

			h ^= hash;
			h *= m;
		}

		size_t digest() noexcept {
			size_t hash = h;
			hash ^= hash >> r;
			hash *= m;
			hash ^= hash >> r;
			return static_cast<size_t>(hash);
		}
	};

	inline size_t hash_int(uint64_t x) noexcept {
		// inspired by lemire's strongly universal hashing
		// https://lemire.me/blog/2018/08/15/fast-strongly-universal-64-bit-hashing-everywhere/
		//
		// Instead of shifts, we use rotations so we don't lose any bits.
		//
		// Added a final multiplication with a constant for more mixing. It is most important that
		// the lower bits are well mixed.
		auto h1 = x * UINT64_C(0xA24BAED4963EE407);
		auto h2 = rotr(x, 32U) * UINT64_C(0x9FB21C651E98DF25);
		auto h = rotr(h1 + h2, 32U);
		return static_cast<size_t>(h);
	}
}// namespace Dice::hash::martinus

#endif//HYPERTRIE_MARTINUS_ROBINHOOD_HASH_HPP
