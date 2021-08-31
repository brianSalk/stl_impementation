#pragma once
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>
#include <concepts>
#include <utility>
#include <cmath> // min
#include "../my_concepts.h"
namespace brian {
template <typename T, typename Allocator = std::allocator<T>> 
class vector {
	// forward declaration of terators
	template <bool Is_Const>
	class vector_iterator;
public:
	using value_type = T;
	using allocator_type = Allocator;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = value_type const&;
	using pointer = typename std::allocator_traits<allocator_type>::pointer;
	using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
	using iterator = vector_iterator<false>;
	using const_iterator = vector_iterator<true>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
private:
	pointer arr;
	allocator_type allocator;
	using Traits = std::allocator_traits<allocator_type>;
	size_type cpt;
	size_type n;
	// TO DO: change the constructors so they accept a pointer to arr instead of size_t
	template <bool Is_Const>
	class vector_iterator {
		// VIDPR;
		friend vector<T,Allocator>;
	public:
		using value_type = T;
		using iterator_category = std::contiguous_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using pointer = typename std::conditional_t<Is_Const,vector::const_pointer, vector::pointer>;
		using reference = typename std::conditional_t<Is_Const,vector::const_reference, vector::reference>;
		vector_iterator() = default;
		vector_iterator(pointer i) : itr_p(i) {}
		vector_iterator(vector_iterator const& it) = default;
		template <bool Was_Const, typename = std::enable_if<Is_Const || !Was_Const>::type>
		vector_iterator(vector_iterator<Was_Const> const& it) : itr_p(it.itr_p) {}

		vector_iterator operator++() {
			return vector_iterator(++itr_p);
		}
		vector_iterator operator++(int) {
			return vector_iterator(itr_p++);
		}
		vector_iterator operator--() {
			return vector_iterator(--itr_p);
		}
		vector_iterator operator--(int) {
			return vector_iterator(itr_p--);
		}
		friend bool operator==(vector_iterator const& lhs, vector_iterator const& rhs) {
			return lhs.itr_p == rhs.itr_p;
		}
		friend vector_iterator operator +=(vector_iterator& it, size_t n) {
			return it.itr_p += n;
		}
		friend vector_iterator operator -=(vector_iterator& it, size_t n) {
			return it.itr_p -= n;
		}
		friend vector_iterator operator +(vector_iterator const& it, size_t n) {
			return vector_iterator(it.itr_p + n);
		}
		friend vector_iterator operator -(vector_iterator const& it, size_t n) {
			return it.itr_p - n;
		}
		friend size_t operator -(vector_iterator const& lhs, vector_iterator const& rhs) {
	return lhs.itr_p - rhs.itr_p;
		}
		auto operator<=>(vector_iterator const& i) const {
			return this->itr_p <=> i.itr_p;
		}
		reference operator*() {
			return *itr_p;
		}
		
	private:
		pointer itr_p;
	};
public:
	// constructors
	constexpr vector() noexcept(noexcept(Allocator()));
	constexpr explicit vector(Allocator const& alloc) noexcept;
	constexpr vector(size_t,T const&, Allocator const& alloc = Allocator());
	constexpr explicit vector(size_t count, Allocator const& alloc = Allocator());
	constexpr vector(vector const& other);
	constexpr vector(vector const& other, Allocator const& alloc);
	constexpr vector(vector && other) noexcept;
	constexpr vector(vector && other, Allocator const& alloc);
	template <typename It, typename std::iterator_traits<It>::pointer=nullptr>
	constexpr vector(It,It);
	vector(std::initializer_list<T> il);
	constexpr void shrink_to_fit();

	// observers
	constexpr Allocator get_allocator() const noexcept { return allocator; }
	constexpr size_t size() const noexcept { return n; }
	constexpr size_t max_size() const noexcept { return Traits::max_size(allocator); }
	constexpr size_t capacity() const noexcept { return cpt; }
	constexpr reference operator[](size_t pos) { return arr[pos]; }
	constexpr const_reference operator[](size_t pos) const { return arr[pos]; }
	constexpr reference front() { return arr[0]; }
	constexpr const_reference front() const { return arr[0]; }
	constexpr reference back() { return arr[n-1]; }
	constexpr const_reference back() const { return arr[n-1]; }
	constexpr reference at(size_t pos) { return (pos<n) ? arr[pos] : throw std::out_of_range("at attempted to access element not in range"); }
	constexpr const_reference at(size_t pos) const { return at(pos); }
	[[nodiscard]]constexpr bool empty() const noexcept { return begin() == end(); }


	iterator 				begin() { return iterator(arr); }
	const_iterator          begin() const { return const_iterator(arr); }
	const_iterator          cbegin() const { return const_iterator(arr); }
	const_reverse_iterator  crbegin() const { return const_reverse_iterator(arr + n); }
	const_reverse_iterator  rbegin() { return reverse_iterator(arr + n); }
	reverse_iterator 		rbegin() const { return const_reverse_iterator(arr + n); }

	iterator 				end() { return iterator(arr + n); }
	const_iterator			end() const { return const_iterator(arr + n); } 			

	const_iterator			cend() const { return const_iterator(arr + n); }
	const_reverse_iterator  crend() const { return const_reverse_iterator(arr); }
	const_reverse_iterator  rend() { return const_reverse_iterator(arr); }
	reverse_iterator 		rend() const { return const_reverse_iterator(arr); }
	// destructor
	/*mutators	*/
	constexpr void push_back(T const& val);
	constexpr void push_back(T && val);
	template <typename ...Args>
	constexpr T& emplace_back(Args &&... args);
	constexpr void pop_back();
	constexpr void clear() noexcept;
	template <typename ...Args>
	constexpr iterator emplace(const_iterator pos, Args &&...args);
	constexpr void reserve(size_t new_cap);
	/*insert*/
	constexpr iterator insert(const_iterator pos, T const& val);
	constexpr iterator insert(const_iterator pos, T && val);
	constexpr iterator insert(const_iterator pos, size_t count, T const& val);
	template <typename It, typename std::iterator_traits<It>::pointer=nullptr>
	constexpr iterator insert(const_iterator pos, It first, It last);
	constexpr iterator insert(const_iterator pos, std::initializer_list<T> il);
	/*erase*/
	constexpr iterator erase(const_iterator pos);
	constexpr iterator erase(const_iterator first, const_iterator last);
	/*assign*/
	constexpr void assign(size_t count, T const& val);
	template <typename It, typename std::iterator_traits<It>::pointer=nullptr>
	constexpr void assign(It first, It last);
	constexpr void assign(std::initializer_list<T> il);
	/*assignment operator*/
	constexpr vector& operator=(vector const& other);
	constexpr vector& operator=(vector && other) noexcept(Traits::propagate_on_container_move_assignment::value || Traits::is_always_equal::value);
	constexpr vector& operator=(std::initializer_list<T> it);
	constexpr void swap(vector& other) noexcept(Traits::propagate_on_container_swap::value || Traits::is_always_equal::value);
	~vector();
private:
	// helpers
	vector(Allocator const& alloc,size_t s,size_t c) : n(s), cpt(c), arr(Traits::allocate(allocator,c)), allocator(alloc) {}
	template <typename It>
	iterator __insert(It first, It last, T* ptr, size_t offset) {
		size_t i = 0;
		for (auto it = first; it != last; ++it) {
			Traits::construct(allocator, arr + i + offset, *it);
			++i;
		}
		return iterator(ptr + offset);
	}
	void __grow() {
		size_t new_capacity = cpt * 2;
		pointer new_arr = Traits::allocate(allocator,new_capacity,arr);
		size_t i{0};
		try {
			for (; i < n; ++i) {
				Traits::construct(allocator, new_arr + i,std::move_if_noexcept(arr[i]));
			}
			// I might need to do something different here if
			// T was moved vs if T was copied
		} catch(...) {
			// if construction fails, clean up
			for (size_t j{0}; j < i; ++j) {
				Traits::destroy(allocator, new_arr+j);
			}
			Traits::deallocate(allocator, new_arr, new_capacity);
			throw;
		}
		// TO DO: should I loop to cpt or to n?
		for (size_t i{0}; i < n; ++i) {
			Traits::destroy(allocator,arr+i); 
		}
		Traits::deallocate(allocator,arr,cpt);
		arr = new_arr;
		cpt = new_capacity;
	}
	template <typename ...Args>
	iterator __insert_or_emplace_one(const_iterator const& pos, Args &&...args) {
	size_t i{0};
	if (n == cpt) {
		size_t new_cpt = cpt*2;
		T* new_arr = Traits::allocate(allocator,new_cpt,arr);
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
		for(i=n;arr+i != pos;--i) {
			arr[i] = std::move_if_noexcept(arr[i-1]);
		}
	}
	Traits::construct(allocator,arr+i,std::forward<Args>(args)...);
	++n;
	return iterator(arr + i);
	}

	template <typename ...Args>
	iterator __insert_many(const_iterator const& pos, size_t count, T const& val) {
		auto before_pos = pos-1;
		size_t i{0};
		if (n + count >= cpt) {
			std::cout << "grower\n";
			size_t new_cpt = (cpt + count) * 2;
			T* new_arr = Traits::allocate(allocator,new_cpt,arr);

			// just do a regular copy to new_arr
			for (   ;arr + i != pos.itr_p;++i) {
				Traits::construct(allocator, new_arr + i, std::move_if_noexcept(arr[i]));
			}
			// now copy with offset of count
			for (size_t j{n-1}; j >= i; --j) {
				Traits::construct(allocator,new_arr + j + count,std::move_if_noexcept(arr[j]));
			}
			for (size_t j{0}; j < n;++j) {
				Traits::destroy(allocator, arr + j);
			}
			//TO DO: copy count val's into arr
			Traits::deallocate(allocator, arr, cpt);
			arr = new_arr;
			cpt = new_cpt;
		} else /*no grow*/	 {
			// shift from pos by count
			// shift everything after pos back by count
			for (i = n-1;true;--i) {
				Traits::construct(allocator, arr + i + count, std::move_if_noexcept(arr[i]));
				Traits::destroy(allocator, arr + i);
				if (arr + i == pos.itr_p) break;
			}
		}
		for (size_t j{i}; j < i + count; ++j) {
			Traits::construct(allocator,arr + j, val);	
		}
		n += count;
		return iterator(arr + i);
	}
	template <typename It>
	iterator __insert_range(const_iterator pos, It first, It last) {
		// first get size of range [first, last)
		size_t count = __get_size(first, last);
		// first copy range [first,last) to a seperate array
		size_t i{0};
		if (n + count > cpt) {
			std::cout << "grower\n";
			size_t new_cpt = (cpt + count) * 2;
			T* new_arr = Traits::allocate(allocator,new_cpt,arr);

			// just do a regular copy to new_arr
			for (   ;arr + i != pos.itr_p;++i) {
				Traits::construct(allocator, new_arr + i, std::move_if_noexcept(arr[i]));
			}
			// now copy with offset of count
			for (size_t j{n-1}; j >= i; --j) {
				Traits::construct(allocator,new_arr + j + count,std::move_if_noexcept(arr[j]));
			}
			for (size_t j{0}; j < n;++j) {
				Traits::destroy(allocator, arr + j);
			}
			//TO DO: copy count val's into arr
			Traits::deallocate(allocator, arr, cpt);
			arr = new_arr;
			cpt = new_cpt;
		} else {
			// shift from pos by count
			// shift everything after pos back by count
			for (i = n-1;true;--i) {
				Traits::construct(allocator, arr + i + count, std::move_if_noexcept(arr[i]));
				Traits::destroy(allocator, arr + i);
				if (arr + i == pos.itr_p) break;
			}
		}
		size_t j{i};
		for (auto it = first; it != last; ++it) {
			Traits::construct(allocator,arr + j++, *it);	
		}
		n += count;
		return iterator(arr + i);
	}
	template <typename It>
	requires at_least_random_access_iterator<It> 
	size_t __get_size(It first, It last) {
		return last - first;
	}
	template <typename It> 
	requires at_most_bidirectional_iterator<It>
	size_t __get_size(It first, It last) {
		size_t size{0};
		for (auto it{first}; it != last; ++it) { ++size; }
		return size;
	}
	iterator __erase_range(const_iterator first, const_iterator last) {
		// destroy elements in range [first,last)
		size_t num_destroyed{last - first};
		for (auto it{first}; it != last; ++it) {
			Traits::destroy(allocator, it.itr_p);
		}
		// shift remaining elements after range to the left by num_destroyed
		n -= num_destroyed;
		iterator curr(const_cast<T*>(first.itr_p));
		for (; curr != end(); ++curr) {
			*curr = std::move_if_noexcept(*(curr+num_destroyed));
			Traits::destroy(allocator, (curr+num_destroyed).itr_p);
		}
		return curr;
	}
	/*friend functions/operator*/
	friend bool operator==(vector<T> const& lhs, vector<T> const& rhs) {
		if (lhs.size() != rhs.size()) { return false; }
		for (size_t i{0}; i < lhs.size(); ++i) {
			if (lhs[i] != rhs[i]) { return false; }
		}
		return true;
	}
public:
	auto operator<=>(vector<T> const& rhs) const {
		size_t s = std::min(this->size(),rhs.size());
		for (size_t i{0};i < s;++i) {
			auto cmp = this->operator[](i) <=> rhs[i];
			if (cmp != 0) { return cmp; }
		}
		return this->size() <=> rhs.size();
	}
}; // END CLASS VECTOR
template <typename T, typename Alloc, typename U>
constexpr typename std::allocator_traits<Alloc>::size_type
erase(vector<T,Alloc>& vec, U const& val) {
	size_t i{0}, replace_with_indx{0};
	for (; replace_with_indx < vec.size(); ++i, ++replace_with_indx) {
		while (vec[replace_with_indx] == val) {
			if (++replace_with_indx == vec.size()) { break; }
		}
		if (replace_with_indx == vec.size()) { break; }
		vec[i] = vec[replace_with_indx];
	}
	vec.erase(vec.begin() + i,vec.end());
	return replace_with_indx - i;
}
} // END NAMESPACE BRIAN
#include "vector.hpp"
