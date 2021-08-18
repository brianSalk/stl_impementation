#pragma once
#include <cstddef>
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
	template <bool Is_Const>
	class vector_iterator {
		// VIDPR
		using value_type = T;
		using iterator_category = std::contiguous_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using pointer = std::conditional_t<Is_Const,vector::const_pointer, vector::pointer>;
		using reference = std::conditional<Is_Const,vector::const_reference, vector::reference>;
		vector_iterator() = default;
		vector_iterator(size_t i) : itr_i(i) {}
		vector_iterator(vector_iterator const& it) = default;
		template <bool Was_Const, typename std::enable_if<Is_Const || !Was_Const>::type>
		vector_iterator(vector_iterator const& it) : itr_i(it.itr_i) {}
		private:
		size_t itr_i;


	};
public:
	// constructors
	vector();
	// observers
	// destructor
	~vector();
}; // END CLASS VECTOR
} // END NAMESPACE BRIAN
#include "vector.hpp"
