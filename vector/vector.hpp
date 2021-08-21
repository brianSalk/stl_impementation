#pragma once
#include "vector.h"
#include <initializer_list>
#include <iterator>
#include <memory>
// constructors
// default constructor
namespace brian {
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector() noexcept(noexcept(Allocator())) : vector(Allocator(),0,1) {}
	// allocator constructor
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(Allocator const& alloc) noexcept : vector(alloc,0,1) {}
	// fill constructor
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::
	vector(size_t count, T const& val, Allocator const& alloc) : vector(alloc,count,count){
		for (size_t i{0}; i < count;++i) {
			Traits::construct(allocator,arr+i,val);
		}
	}
	// default fill constructor
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(size_t count, Allocator const& alloc) : vector(alloc,count,count) { for (size_t i{0};i<count;++i)Traits::construct(allocator,arr); }
	// range constructor
	template <typename T, typename Allocator>
	template <typename It, typename std::iterator_traits<It>::pointer>
	constexpr vector<T,Allocator>::vector(It first, It last) {
		n = std::distance(first,last);
		capacity = n;
		arr = Traits::allocate(allocator,n);
		// insert overload just like list, one overload is for contiguous/random
		// access the other for forward/bidirectional	
		std::cout << "lhe\n";
		__insert(first,last,arr,0);
	}
	// copy constructor
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(vector const& other) : vector(Traits::select_on_container_copy_construction(other.get_allocator()),other.capacity(), other.size())  {
		for (size_t i{0}; i < n; ++i) {
			Traits::construct(allocator,arr + i, other.arr);
		}
	}
	// allocator extended copy constructor
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(vector const& other, Allocator const& alloc) : vector(alloc,other.size(),other.capacity()) {
		for (size_t i{0}; i < n; ++i) {
			Traits::construct(allocator,arr + i, other.arr);
		}
	}
	// move constructor
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(vector && other) noexcept {
		this->arr = other.arr;
		this->n = other.n;
		this->capacity = other.capacity;
		other.n = 0;
		other.capacity = 1;
		other.arr = Traits::allocate(allocator,1);
	}
	// move constructor allocator extended
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(vector && other, Allocator const& alloc) {
		if (allocator == alloc) {
			this->arr = other.arr;
			this->n = other.n;
			this->capacity = other.capacity;
			other.n = 0;
			other.capacity = 1;
			other.arr = Traits::allocate(allocator,1);
		} else {
			this->allocator = alloc;
			this->n = other.n;
			this->capacity = other.capacity;
			this->arr = Traits::allocate(this->allocator,this->capacity);
			for (size_t i{0}; i < this->n; ++i) {
				Traits::construct(allocator,arr + i,std::move(other.arr[i]));
			}
		}
	}
	// initializer list constructor
	template <typename T, typename Allocator>
	vector<T, Allocator>::vector(std::initializer_list<T> il) {
		n = il.size();	
		capacity = n;
		arr = Traits::allocate(allocator,n);
		size_t i = 0;
		for (auto const& each : il) {
			Traits::construct(allocator,arr + (i++),each);
		}
	}
	template <typename T, typename Allocator>
	vector<T,Allocator>::~vector() {
		Traits::destroy(allocator,arr);
		Traits::deallocate(allocator,arr,capacity);
	}
}
