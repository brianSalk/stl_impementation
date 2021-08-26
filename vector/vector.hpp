#pragma once
#include "vector.h"
#include <initializer_list>
#include <iterator>
#include <memory>
#include <utility>
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
		cpt = n;
		arr = Traits::allocate(allocator,n);
		// insert overload just like list, one overload is for contiguous/random
		// access the other for forward/bidirectional	
		std::cout << "lhe\n";
		__insert(first,last,arr,0);
	}
	// copy constructor
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(vector const& other) : vector(Traits::select_on_container_copy_construction(other.get_allocator()),other.cpt(), other.size())  {
		for (size_t i{0}; i < n; ++i) {
			Traits::construct(allocator,arr + i, other.arr);
		}
	}
	// allocator extended copy constructor
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(vector const& other, Allocator const& alloc) : vector(alloc,other.size(),other.cpt()) {
		for (size_t i{0}; i < n; ++i) {
			Traits::construct(allocator,arr + i, other.arr);
		}
	}
	// move constructor
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(vector && other) noexcept {
		this->arr = other.arr;
		this->n = other.n;
		this->cpt = other.cpt;
		other.n = 0;
		other.cpt = 1;
		other.arr = Traits::allocate(allocator,1);
	}
	// move constructor allocator extended
	template <typename T, typename Allocator>
	constexpr vector<T,Allocator>::vector(vector && other, Allocator const& alloc) {
		if (allocator == alloc) {
			this->arr = other.arr;
			this->n = other.n;
			this->cpt = other.cpt;
			other.n = 0;
			other.cpt = 1;
			other.arr = Traits::allocate(allocator,1);
		} else {
			this->allocator = alloc;
			this->n = other.n;
			this->cpt = other.cpt;
			this->arr = Traits::allocate(this->allocator,this->cpt);
			for (size_t i{0}; i < this->n; ++i) {
				Traits::construct(allocator,arr + i,std::move(other.arr[i]));
			}
		}
	}
	// initializer list constructor
	template <typename T, typename Allocator>
	vector<T, Allocator>::vector(std::initializer_list<T> il) {
		n = il.size();	
		cpt = n;
		arr = Traits::allocate(allocator,n);
		size_t i = 0;
		for (auto const& each : il) {
			Traits::construct(allocator,arr + (i++),each);
		}
	}
	template <typename T, typename Allocator>
	vector<T,Allocator>::~vector() {
		Traits::destroy(allocator,arr);
		Traits::deallocate(allocator,arr,cpt);
	}
	/*mutators*/
	template <typename T, typename Allocator>
	constexpr void vector<T,Allocator>::push_back(T const& val) {
		if (n == cpt) {
			__grow();
		}
		arr[n++] = val;
	}
	template <typename T, typename Allocator>
	constexpr void vector<T,Allocator>::push_back(T && val) {
		if (n == cpt) {
			__grow();
		}
		// TO DO: should I use Traits::construct?
		arr[n++] = std::move(val);
	}
template<typename T,typename Allocator>   
template <typename ...Args>
constexpr T& vector<T,Allocator>::emplace_back(Args &&... args) {
	if (n == cpt) { __grow(); }	
	Traits::construct(allocator, arr + n, std::forward<Args>(args)...);
	return arr[n++];
}
template<typename T, typename Allocator> 
constexpr void vector<T,Allocator>::pop_back() {
		Traits::destroy(allocator, arr + --n);
}

template<typename T, typename Allocator> 
constexpr void vector<T,Allocator>::clear() noexcept {
	for (size_t i{0}; i < n; ++i) {
		Traits::destroy(allocator, arr+i);
	}
	n = 0;
}
template<typename T, typename Allocator> 
template <typename ...Args>
constexpr typename vector<T,Allocator>::iterator vector<T,Allocator>::emplace(const_iterator pos, Args &&...args) {
	size_t i{0};
	if (n == cpt) {
		size_t new_cpt = cpt*2;
		T* new_arr = Traits::allocate(allocator,new_cpt,arr);
		// stop i when it is at pos
		for (; arr + i != pos.itr_p;++i) {
			Traits::construct(allocator, new_arr + i, std::move_if_noexcept(arr[i]));
		}
		for (size_t j{n};arr + i != arr + j;--j) {
			new_arr[j] = std::move_if_noexcept(arr[j-1]);
		}
		for (size_t i{0}; i < n; ++i) {
			Traits::destroy(allocator, arr + i);
		}
		Traits::deallocate(allocator,arr,cpt);
		arr = new_arr;
		cpt = new_cpt;
	} else {
		for(i=n+1;arr+i != pos;--i) {
			arr[i] = std::move_if_noexcept(arr[i-1]);
		}	
	}
	Traits::construct(allocator,arr+i,std::forward<Args>(args)...);
	++n;
	return iterator(arr);
}
}
