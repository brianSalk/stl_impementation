#pragma once
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <compare>
namespace brian {
	template <typename T, size_t N>
	struct array {
	T arr[N];
	private:
	template <bool Is_Const>
	class array_iterator;
	public:
		using value_type = T;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type&;
		using const_reference = value_type const&;
		using pointer = value_type*;
		using const_pointer = value_type const*;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	//	array() = default;
		~array() = default;
	//	array& operator=(array const&) = default;
		// observers
		constexpr size_t size() const {return N;}
		constexpr reference front() { return *arr; }
		constexpr const_reference front() const { return *arr; }
		constexpr reference back() { return arr[N-1]; }
		constexpr const_reference back() const { return arr[N-1]; }
		constexpr reference operator[](size_t i) { return arr[i]; }
		constexpr const_reference operator[](size_t i) const { return arr[i]; }
		constexpr reference at(size_t i) { if (!(i < N)) { throw std::out_of_range("value out of range"); } return arr[i]; }
		constexpr const_reference at(size_t i) const { if (!(i < N)) { throw std::out_of_range("value out of range"); } return arr[i]; }
		constexpr pointer data() noexcept { return arr; }
		constexpr const_pointer data() const noexcept { return arr; }
		[[nodiscard]]
		constexpr bool empty() const noexcept { return N == 0; }
		constexpr size_t max_size() const noexcept { return N; }
		// modifiers 
		constexpr void fill(T const& val);
		// iterator methods
		constexpr iterator begin() { return arr; }
		constexpr const_iterator begin() const { return arr; }
		constexpr const_iterator cbegin() const { return arr; }

		constexpr iterator end() { return iterator(arr+N); }
		constexpr const_iterator end() const { return arr+N; }
		constexpr const_iterator cend() const { return arr+N; }
		// non-member functions and friends
		constexpr friend bool operator==(array const& lhs, array const& rhs) {
			if (lhs.size() != rhs.size()) { return false; }
			for (size_t i{0}; i < rhs.size(); ++i) { if (lhs[i] != rhs[i]) { return false; } }
			return true;
		}
		std::strong_ordering operator<=>(array const& other) const {
			for (size_t i{0}; i < N; ++i) {
				if (this->operator[](i) != other[i]) {
					return this->operator[](i) <=> other[i];
				}
			}
			return this->size() <=> other.size();
		}
			
	}; // END CLASS ARRAY
} // END NAMESPACE BRIAN
#include "array.hpp"
