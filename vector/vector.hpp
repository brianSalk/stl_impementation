#pragma once
#include "vector.h"

namespace brian {
	template <typename T, typename Allocator>
	vector<T,Allocator>::vector() {
		capacity = 1;
		n = 0;
		arr = Traits::allocate(allocator,1);
	}
	template <typename T, typename Allocator>
	vector<T,Allocator>::~vector() {
		Traits::destroy(allocator,arr);
		Traits::deallocate(allocator,arr,capacity);
	}
}
