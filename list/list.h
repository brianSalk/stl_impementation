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
	template <bool Is_Const>
	class list_iterator;
	template <bool Is_Const>
	class reverse_list_iterator;
	struct base_node {
		base_node* next, * prev;
		base_node() :next(nullptr), prev(nullptr) {}
		base_node(int,base_node*n):prev(nullptr),next(n) {}
		base_node(base_node* p) : prev(p), next(nullptr) {}
		base_node(base_node* p, base_node* n) :next(n),prev(p){}

	};
	struct node :public base_node{
		T val;
		// constructor overload used when no node is passed
		template <typename U,typename ...Args, typename = 
			std::enable_if_t<!std::is_same<U,base_node*>::value && !std::is_same<U,node*>::value>>
		node(U && u,Args &&...args) : base_node(), val(std::forward<U>(u),std::forward<Args>(args)...){}
		// constructor overload used when node is passed
		template <typename ...Args>
		node(base_node* p, Args && ...args) : base_node(p),val(std::forward<Args>(args)...){}
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
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	private:
	using node_allocator_t = typename std::allocator_traits<allocator_type>
		::template rebind_alloc<node>;
	node_allocator_t node_allocator;
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
	iterator insert(const_iterator pos, T && val);
	iterator insert(const_iterator pos, size_t, T const& val);
	template <typename It, typename std::iterator_traits<It>::pointer = nullptr>
	iterator insert(const_iterator pos, It beg, It end);
	iterator insert(const_iterator pos, std::initializer_list<T> il);
	// iterator_methods
	iterator begin() { return iterator(pre_head->next); }
	iterator end() { return iterator(aft_tail); }
	const_iterator begin() const { return const_iterator(pre_head->next); }
	const_iterator end() const { return const_iterator(aft_tail); }
	const_iterator cbegin() const { return const_iterator(pre_head->next); }
	const_iterator cend() const { return const_iterator(aft_tail); }

	reverse_iterator rbegin() { return reverse_iterator(pre_head->next); }
	reverse_iterator rend() { return reverse_iterator(aft_tail); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(pre_head->next); }
	const_reverse_iterator rend() const { return const_reverse_iterator(aft_tail); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(pre_head->next); }
	const_reverse_iterator crend() const { return const_reverse_iterator(aft_tail); }
		
	
	~list();
	// list_iterator is the base class of reverse_iterator
// that contains the code that is common to both forward and reverse iterator classes
private:
	template <bool Is_Const>
	class list_iterator {
		friend list<T,Allocator>;
	public:
		using value_type = T;
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using pointer = typename std::conditional<Is_Const, T const*, T*>::type;
		using reference = typename std::conditional<Is_Const, T const&, T&>::type;
		list_iterator() = default;
		list_iterator(base_node* c) : itr_curr(c) {}
		// default copy constructor
		list_iterator(list_iterator<Is_Const> const&) = default;
		// converts iterator to const_iterator 
		template <bool Was_Const,typename = std::enable_if_t<Is_Const || !Was_Const>>
			list_iterator(list_iterator<Was_Const> const& i) : itr_curr(i.itr_curr){}
		friend bool operator==(list_iterator<Is_Const> const& lhs, list_iterator<Is_Const> const& rhs) {
			return lhs.itr_curr == rhs.itr_curr;
		}
		friend bool operator!=(list_iterator const& lhs, list_iterator const& rhs) {
			return lhs.itr_curr != rhs.itr_curr;
		}
		reference operator*() {
				return static_cast<node*>(itr_curr)->val;
		}
		// stuff unique to forward_iterator
		// prefix incrament
		list_iterator& operator++() {
			itr_curr = itr_curr->next;
			return *this;
		}
		// suffix incrament
		list_iterator operator++(int) {
			auto tmp = itr_curr;
			itr_curr = itr_curr->next;
			return list_iterator(itr_curr);
		}
		// prefix decrament
		list_iterator& operator--() {
			itr_curr = itr_curr->prev;
			return *this;
		}
		// suffix decrament
		list_iterator operator--(int) {
			auto tmp = itr_curr;
			itr_curr = itr_curr->prev;
			return list_iterator(itr_curr);
		}
		// overload the -> operator

	private:
		base_node* itr_curr;
	};	
	// helper methods
	template <typename ...Args>
	node* create_node(Args &&...args) {
		node* new_node = Traits::allocate(node_allocator,1);
		try {
			Traits::construct(node_allocator,new_node,std::forward<Args>(args)...);
		}
		catch (...) {
			Traits::deallocate(node_allocator,new_node,1);
			throw;
		}
		return new_node;
	}
	void delete_node(base_node* del_node) {
		Traits::destroy(node_allocator, static_cast<node*>(del_node));
		Traits::deallocate(node_allocator, static_cast<node*>(del_node),1);
	}
	// used to connect 2 preexisting nodes
	void connect_nodes(base_node* first, base_node* second) {
		first->next = second;
		second->prev = first;
	}
};// END CLASS LIST
}// END NAMESPACE BRIAN
#include "list.hpp"
