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
template<typename T, typename Allocator>
constexpr void vector<T,Allocator>::assign(size_t count, T const& val) {
	// the old needs to be destructed after the new is created, 
	// that way if the allocation of the new fails, we can fall back on the old
	// this does, however, make the allocation of new more likely to throw bad_alloc
	size_t i{0};
	if (count <= cpt) {
		// destroy the old
		for (size_t i{0}; i < n;++i) { Traits::destroy(allocator,arr + i); }
		// construct the new
		try {
			for (; i < count;++i) { Traits::construct(allocator, arr + i, val); }
		} catch (...) {
			n = i;
			throw;
		}
	} else {
		T* new_arr;
		try {
			// allocate new array of size count*2
			new_arr = Traits::allocate(allocator, count*2, arr);
		} catch(...) {
			Traits::deallocate(allocator, new_arr, count*2);
			throw;
		}
		try {
			// construct count new val
			for (; i < count; ++i) { Traits::construct(allocator, new_arr + i, val); }
		} catch(...) {
			n = i;
			cpt = count*2;
			throw;
		}
		// destroy the old
		for (size_t j{0}; j < n; ++j) { Traits::destroy(allocator, arr + j); }
		// deallocate the old
		Traits::deallocate(allocator, arr, cpt);
		arr = new_arr;
		cpt = count*2;
	}
	n = count;
}
template<typename T, typename Allocator>
template <typename It, typename std::iterator_traits<It>::pointer>
constexpr void vector<T,Allocator>::assign(It first, It last) {
	size_t range_size = __get_size(first, last);			
	size_t i{0};
	if (range_size <= cpt) {
		// destroy the old
		for (size_t i{0}; i < n;++i) { Traits::destroy(allocator,arr + i); }
		// construct the new
		try {
			for (auto it = first; it != last; ++it,++i) { 
				Traits::construct(allocator, arr + i, std::move_if_noexcept(*it)); 
			}
		} catch (...) {
			n = i;
			throw;
		}
	} else {
		T* new_arr;
		try {
			// allocate new array of size range_size*2
			new_arr = Traits::allocate(allocator, range_size*2, arr);
		} catch(...) {
			Traits::deallocate(allocator, new_arr, range_size*2);
			throw;
		}
		try {
			// construct range_size new val
			for (auto it = first; it != last; ++it, ++i) { 
				Traits::construct(allocator, new_arr + i, std::move_if_noexcept(*it)); 
			}
		} catch(...) {
			n = i;
			cpt = range_size*2;
			throw;
		}
		// destroy the old
		for (size_t j{0}; j < n; ++j) { Traits::destroy(allocator, arr + j); }
		// deallocate the old
		Traits::deallocate(allocator, arr, cpt);
		arr = new_arr;
		cpt = range_size*2;
	}
	n = range_size;
}
template<typename T, typename Allocator>
constexpr void vector<T,Allocator>::assign(std::initializer_list<T> il) {
	assign(il.begin(), il.end());
}
template<typename T, typename Allocator>
constexpr typename vector<T, Allocator>:: template vector<T,Allocator> & 
vector<T,Allocator>::operator=(vector const& other) {
	if (Traits::propagate_on_container_copy_assignment::value) {
		this->allocator = other.get_allocator();
	} else if (this->allocator != other.get_allocator() || this->cpt < other.n) {
		// destroy and reallocate
		size_t i{0};
		T* new_arr;
		try {
			new_arr = Traits::allocate(allocator, other.cpt);
			for (size_t j{0}; j < other.n; ++j) {
				Traits::construct(allocator, new_arr + j,other[j]);
			}
		} catch(...) {
			for (size_t j{0}; j < i; ++j) {
				Traits::destroy(allocator, new_arr + j);
			}
			Traits::deallocate(allocator, new_arr, this->cpt);
			throw;
		}
		for (size_t j{0}; j < this->n; ++j) {
			Traits::destroy(allocator, arr + j);
		}	
		Traits::deallocate(allocator, this->arr, this->cpt);
		arr = new_arr;
		this->n = other.n;
		this->cpt = other.cpt;
		return *this;
	}
	// allocators are equal and we have the capacity so we can reassign old data
	// TO DO: find out if capacity is required to be equal to other after assignment.
	// Im just going to assume it does not here
	// we can just reassign elements of this to elements of other		
	for (size_t j{0}; j < this->n; ++j) {
		Traits::destroy(allocator, this->arr + j);
	}
	size_t i{0};
	try {
		for (; i < other.size(); ++i) {
			Traits::construct(allocator, this->arr + i, other.arr[i]);
		}
	} catch(...) {
		this->n = i;
		throw;
	}
	this->n = other.n;
	return *this;
}
template<typename T, typename Allocator>
constexpr typename vector<T,Allocator>::vector<T, Allocator>& 
vector<T,Allocator>::operator=(vector && other) noexcept(Traits::propagate_on_container_move_assignment::value || Traits::is_always_equal::value) {
	if (Traits::propagate_on_container_move_assignment::value) {
		this->allocator = other.get_allocator();
	} else if (this->allocator != other.allocator || this->cpt < other.n) {
		std::cout << "reallocate\n";
		// we need to delete this->arr and allocate new_arr
		T* new_arr;
		size_t new_cpt = other.n *2;
		size_t new_n = other.n;
		new_arr = Traits::allocate(allocator, new_cpt, arr);
		size_t i{0};
		try {
			for (;i < other.n; ++i) {
				Traits::construct(allocator, new_arr + i, std::move(other.arr[i]));
			}
		} catch(...) {
			for (size_t j{0}; j < i; ++j) {
				Traits::destroy(allocator, arr + j);
			}
			Traits::deallocate(allocator, new_arr, new_cpt);
			throw;
		}
		for (size_t j{0}; j < i; ++j) {
			Traits::destroy(allocator, arr + j);
		}
		Traits::deallocate(allocator, arr, this->cpt);
		n = new_n;
		cpt = new_cpt;
		return *this;
	}
	// we can try to reuse memory here, so destroy/deallocate the old and repoint
	// destry old
	for (size_t j{0}; j < this->n; ++j) {
		Traits::destroy(allocator, this->arr + j);
	}
	Traits::deallocate(allocator, this->arr, this->cpt);
	this->arr = other.arr;
	this->n = other.n;
	this->cpt = other.cpt;
	other.arr = nullptr;
	other.n = 0;
	other.cpt = 0;
	return *this;		
}
template<typename T, typename Allocator>
constexpr typename vector<T,Allocator>::vector<T, Allocator>& 
vector<T,Allocator>::operator=(std::initializer_list<T> il) {
	T* new_arr;
	size_t new_cpt = il.size() * 2;
	size_t new_n = il.size();
	size_t i{0};
	auto il_p = data(il);
	new_arr = Traits::allocate(allocator, new_cpt, arr);
	try {
		for (; i < new_n; ++i) {
			Traits::construct(allocator, new_arr + i, il_p[i]);	
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
	n = new_n;
	cpt = new_cpt;
	return *this;
}
template<typename T, typename Allocator>
constexpr void vector<T,Allocator>::swap(vector& other) noexcept(Traits::propagate_on_container_swap::value || Traits::is_always_equal::value) {
		auto tmp_arr = this->arr;
		auto tmp_n = this->n;
		auto tmp_cpt = this->cpt;
		this->arr = other.arr;
		this->n = other.n;
		this->cpt = other.cpt;
		other.arr = tmp_arr;
		other.n = tmp_n;
		other.cpt = tmp_cpt;
}
}
