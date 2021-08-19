#pragma once
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <type_traits>
namespace brian {
template <typename T, typename Allocator = std::allocator<T>> 
class vector {
	// forward declaration of iterators
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
	size_type capacity;
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
		template <bool Was_Const, typename std::enable_if<Is_Const || !Was_Const>::type>
		vector_iterator(vector_iterator const& it) : itr_p(it.itr_p) {}

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
			return it.itr_p + n;
		}
		friend vector_iterator operator -(vector_iterator const& it, size_t n) {
			return it.itr_p - n;
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
	template <typename It, typename std::iterator_traits<It>::pointer=nullptr>
	constexpr vector(It,It);
	vector(std::initializer_list<T> il);
	// observers
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
	~vector();
private:
	// helpers
	vector(Allocator const& alloc,size_t s,size_t c) : n(s), capacity(c), arr(Traits::allocate(allocator,arr,c)), allocator(alloc) {}
}; // END CLASS VECTOR
} // END NAMESPACE BRIAN
#include "vector.hpp"