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

		array() = default;
		~array() = default;
		array& operator=(array const&) = default;
		// observers
		constexpr size_t size() const {return N;}
		constexpr reference front() { return *arr; }
		constexpr const_reference front() const { return *arr; }
		constexpr reference back() { return arr[N-1]; }
		constexpr const_reference back() const { return arr[N-1]; }
		constexpr reference operator[](size_t i) { return arr[i]; }
		constexpr const_reference operator[](size_t i) const { return arr[i]; }
		constexpr reference at(size_t i) { return (i < N) ? arr[i]: throw std::out_of_range("index out of range");}
		constexpr const_reference at(size_t i) const {  return at(i);}
		constexpr pointer data() noexcept { return arr; }
		constexpr const_pointer data() const noexcept { return arr; }
		[[nodiscard]]
		constexpr bool empty() const noexcept { return N == 0; }
		constexpr size_t max_size() const noexcept { return N; }
		// modifiers 
		constexpr void fill(T const& val);
		constexpr void swap(array& other) noexcept(std::is_nothrow_swappable_v<T>) {
			for (size_t i{0}; i < N; ++i) {
				std::swap(this->arr[i], other.arr[i]);
			}
		}
		// iterator methods
		constexpr iterator begin() { return arr; }
		constexpr const_iterator begin() const { return arr; }
		constexpr const_iterator cbegin() const { return arr; }
		constexpr reverse_iterator rbegin() { return reverse_iterator(arr+N); }
		constexpr reverse_iterator rbegin() const { return const_reverse_iterator(arr+N); }
		constexpr const_reverse_iterator crbegin() const { return const_reverse_iterator(arr+N); }

		constexpr iterator end() { return iterator(arr+N); }
		constexpr const_iterator end() const { return arr+N; }
		constexpr const_iterator cend() const { return arr+N; }
		constexpr reverse_iterator rend() { return reverse_iterator(arr); }
		constexpr const_reverse_iterator rend() const { return const_reverse_iterator(arr); }
		constexpr const_reverse_iterator crend() const { return const_reverse_iterator(arr); }

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
	template <size_t I, typename T, size_t N> 
	constexpr T& get(array<T,N> & arr) noexcept {
		return arr[I];
	}
	template <size_t I, typename T, size_t N> 
	constexpr T&& get(array<T,N> && arr) noexcept {
		return arr[I];
	}
	template <size_t I, typename T, size_t N> 
	constexpr T const& get(array<T,N> & arr) noexcept {
		return arr[I];
	}
	template <size_t I, typename T, size_t N> 
	constexpr T const&& get(array<T,N> && arr) noexcept {
		return arr[I];
	}
} // END NAMESPACE BRIAN
// the following helper classes allow brian::array to be treated like a tuple
namespace std {
	template <typename T, size_t N>
	constexpr brian::array<std::remove_cv_t<T>, N> to_array(T (&arr)[N]) {
		brian::array<T,N> array;
		for (size_t i{0}; i < N; ++i) {
			array[i] = arr[i];
		}
		return array;
	}
	template <typename T, size_t N>
	constexpr brian::array<std::remove_cv_t<T>, N> to_array(T (&&arr)[N]) {
		brian::array<T,N> array;
		for (size_t i{0}; i < N; ++i) {
			array[i] = std::move(arr[i]);
		}
		return array;
	}
	template <typename T>
	struct tuple_size;
	template <typename T, size_t N>
	struct tuple_size<brian::array<T,N> > : std::integral_constant<size_t, N> {
		static size_t const value = N;	
		using value_type = size_t;
		using type = std::integral_constant<value_type,value>;
		operator size_t () { return value; }
		size_t operator()() { return value; }
	};
	template <size_t I, typename T>
	struct tuple_element;
	template <size_t I, typename T, size_t N>
	struct tuple_element<I, brian::array<T,N>> {
		using type = T;	
	};
} // END NAMESPACE STD
#include "array.hpp"
