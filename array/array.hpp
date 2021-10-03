#pragma once
#include "array.h"
namespace brian {
	template<typename T, size_t N>
	constexpr void array<T, N>::fill(T const& val) {
		for (size_t i{0}; i < N; ++i) {
			arr[i] = val;
		}
	}
} // END NAMESPACE BRIAN
