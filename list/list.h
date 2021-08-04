#pragma once
#include <cstddef>
#include <memory>

namespace brian {
template <typename T, typename Allocator = std::allocator<T>>
class list {
	template <bool Is_Const>
	class __base_iterator;
	template <bool Is_Const>
	class list_iterator : public __base_iterator<Is_Const>;
	template <bool Is_Const>
	class reverse_list_iterator : public __base_iterator<Is_Const>;
	public:
	using value_type = T;
	using allocator_type = Allocator;
	using size_type  = std::size_t;
	using difference_type = std::ptrdiff_t;
	using referene = value_type&;
	using const_reference = value_type const&;
	using pointer = typename std::allocator_traits<Allocator>::pointer;
	using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
	using iterator = list_iterator<false>;
	using const_iterator = list_iterator<true>;
	using reverse_iterator = reverse_list_iterator<false>;
	using const_reverse_iterator = reverse_list_iterator<true>;


	
};// END CLASS LIST
}// END NAMESPACE BRIAN
