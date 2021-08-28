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
	return __insert_or_emplace_one(pos, std::forward<Args>(args)...);
}
template<typename T, typename Allocator> 
constexpr typename vector<T,Allocator>::iterator vector<T,Allocator>::insert(const_iterator pos, T const& val) {
		return __insert_or_emplace_one(pos, std::forward<T>(val));
	}
template<typename T, typename Allocator> 
constexpr typename  vector<T, Allocator>::iterator vector<T,Allocator>::insert(const_iterator pos, T && val) {
	return __insert_or_emplace_one(std::forward<T>(pos,val));
}

template<typename T, typename Allocator> 
constexpr typename vector<T,Allocator>::iterator vector<T,Allocator>::insert(const_iterator pos, size_t count, T const& val) {
	return __insert_many(pos,count,val);			
}
template<typename T, typename Allocator>
template <typename It, typename std::iterator_traits<It>::pointer>
constexpr typename vector<T,Allocator>::iterator vector<T,Allocator>::insert(const_iterator pos, It first, It last) {
	return __insert_range(pos,first,last);			
}
template<typename T, typename Allocator>
constexpr typename vector<T,Allocator>::iterator vector<T,Allocator>::insert(const_iterator pos, std::initializer_list<T> il) {
	return __insert_range(pos, il.begin(), il.end());
	}
template<typename T, typename Allocator>
constexpr typename vector<T,Allocator>::iterator vector<T,Allocator>::erase(const_iterator pos) {
	return __erase_range(pos,pos+1);			
}
template<typename T, typename Allocator>
constexpr typename vector<T,Allocator>::iterator vector<T,Allocator>::erase(const_iterator first, const_iterator last) {
	return __erase_range(first, last);
}
template<typename T, typename Allocator>
constexpr void vector<T,Allocator>::shrink_to_fit() {
	if (n == cpt) { return; }
	T* new_arr;
	size_t i{0};
	try {
		new_arr = Traits::allocate(allocator,n);
		for (; i < n;++i) {
			Traits::construct(allocator, new_arr + i, arr[i]);
		}
	} catch(...) {
		for (size_t j{0}; j < i; ++j) {
			Traits::destroy(allocator, new_arr + j);
		}
		Traits::deallocate(allocator,arr,i);
		throw;
	}
	for (size_t i{0}; i < n;++i) {
		Traits::destroy(allocator, arr + i);
	}
	Traits::deallocate(allocator, arr,cpt);
	arr = new_arr;
	cpt = n;
}
template<typename T, typename Allocator>
constexpr void vector<T,Allocator>::reserve(size_t new_cpt) {
	size_t i{0};
	if (new_cpt <= cpt) { return; }
	T* new_arr;
	try {
		new_arr = Traits::allocate(allocator, new_cpt, arr);
		for (;i < n; ++i) {
			Traits::construct(allocator, new_arr + i, arr[i]);
		}
	} catch(...) {
		for (size_t j{0}; j < i; ++j) {
			Traits::destroy(allocator, new_arr + j);
		}
		Traits::deallocate(allocator, new_arr, new_cpt);
		throw;
	}
	for (size_t j{0}; j < n; ++j) {
		Traits::destroy(allocator, arr + j);
	}
	Traits::deallocate(allocator, arr, cpt);
	arr = new_arr;
	cpt = new_cpt;
}
}
