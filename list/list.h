#pragma once
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace brian {
template <typename T, typename Allocator = std::allocator<T>>
class list {
	//__base_iterator contains the code that is common to forward and backwards iterators
	template <bool Is_Const>
	class __base_iterator;
	template <bool Is_Const>
	class list_iterator;
	template <bool Is_Const>
	class reverse_list_iterator;
	struct base_node {
		base_node* next, * prev;
		base_node() :next(nullptr), prev(nullptr) {}
		base_node(int,base_node*n):next(n) {}
		base_node(base_node* p) : prev(p) {}
		base_node(base_node* p, base_node* n) :next(n),prev(p){}

	};
	struct node {
		T val;
		template <typename ...Args>
		node(Args &&...args) : base_node(), val(std::forward<Args>(args)...){}
		template <typename ...Args>
		node(node* p, Args && ...args) : base_node(p),val(std::forward<Args>()...){}
		template <typename ...Args>
		node(Args && ... args, node* n) : base_node(0,n),val(std::forward<Args>(args)...){}
		template <typename ...Args>
		node(node* p,Args && ... args, node* n) : base_node(p,n),val(std::forward<Args>(args)...){}
	};
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
	private:
	using node_allocator_t = typename std::allocator_traits<allocator_type>
		::template rebind_alloc<node>;
	using Traits = std::allocator_traits<node_allocator_t>;
	base_node* pre_head;	
	base_node* aft_tail;
	public:
	// constructors
	list();
	explicit list(allocator_type const& alloc);
	// modifiers
	list(size_type count, value_type const& val, allocator_type const& alloc = allocator_type());
	explicit list(size_type count, allocator_type const& alloc = allocator_type());
	template <typename It, typename std::iterator_traits<It>::pointer>
	list(list const& other);
	list(list const& other, allocator_type const& alloc);
	list(list && other);
	list(list && other, allocator_type const& alloc);
	list(std::initializer_list<T> il, allocator_type const& alloc = allocator_type());
	// modifiers
	void clear() noexcept;	
	iterator insert(const_iterator pos, T const& val); 

private:
	template <bool Is_Const>
	class base_iterator {
	public:
		using value_type = T;
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using pointer = typename std::conditional<Is_Const, T const*, T*>::type;
		using reference = typename std::conditional<Is_Const, T const&, T&>;

	private:
	}	
};// END CLASS LIST
}// END NAMESPACE BRIAN
#include "list.hpp"
