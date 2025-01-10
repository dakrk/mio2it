#pragma once
#include <bit>
#include <concepts>

#define UNKNOWN_ENDIAN() \
	static_assert(endian::native == endian::little || \
	              endian::native == endian::big, \
	              "Unsupported endianness")

template <std::integral T>
constexpr T LE(T in) {
	using std::endian;

	if constexpr (endian::native == endian::little) {
		return in;
	} else if constexpr (endian::native == endian::big) {
		return std::byteswap(in);
	} else {
		UNKNOWN_ENDIAN();
	}
}

#undef UNKNOWN_ENDIAN
