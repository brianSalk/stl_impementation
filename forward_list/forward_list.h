#pragma once
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <sys/types.h>
#include <type_traits>
#include <utility>
namespace brian {
template <typename T, typename Allocator = std::allocator<T>>
class forward_list {
public:
	// forward declaration of list_iterator class template
	template <bool IsConst> 
	class list_iterator;
	// aliases 
	using value_type = T;
	using allocator_type = Allocator;
	using size_type = size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = value_type const&; 
	using pointer = typename std::allocator_traits<Allocator>::pointer;
	using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
	using iterator = list_iterator<false>;
	using const_iterator = list_iterator<true>;
	// DELETE ME: this method is for debugging only
	void dump() const { 
		std::cout << "DUMP" << std::endl;
		derived_node* curr = static_cast<derived_node*>(pre_head);
		curr = static_cast<derived_node*>(curr->next);
		while (curr != nullptr) {
			std::cout << curr->val << '\n';
			curr = static_cast<derived_node*>(curr->next);
		}
	}
	// constructors
	forward_list();
	forward_list(std::initializer_list<T> const& il);
	// mutators
	void push_front(T const& val);
	template<typename ...Args>
	void emplace_front(Args && ...arg);
	const_iterator insert_after(const_iterator pos, T const& val);
	// observers
	const_iterator begin() const { return const_iterator(pre_head->next);}
	iterator begin() { return iterator(pre_head->next); }
	const_iterator end() const { return const_iterator(nullptr); }
	iterator end()  { return iterator(nullptr); }
	const_iterator const_end() const { return const_iterator(nullptr); }
	const_iterator const_begin() const { return const_iterator(pre_head->next); }
	iterator before_begin() { return iterator(pre_head); }
	const_iterator before_begin() const { return const_iterator(pre_head); }
	const_iterator cbefore_begin() const { return const_iterator(pre_head); }
private:
	// base_node needs to be constructed because the
	// list must have one node before the head of the list
	// since we cannot assume that the template type T
	// has a default constructor, we cannot instantiate the
	// type using the default constructor, therefor we need a
	// special node called base_node for the first node of the list
	struct base_node {
		base_node* next;
		base_node() : next(nullptr){}
		base_node(base_node* n) : next(n) {}
	};
	struct derived_node : base_node {
		T val;
		derived_node() : base_node() {} 
		template <typename ...Args>
		derived_node(Args && ...args) : val(T(std::forward<Args>(args)...)) {}
		//FIX ME: why do i need to use the baseclass name when
		//referencing a baseclass member here?
		derived_node(base_node* n, T const& v) : base_node(n),
			val(v) {}
	};
	base_node* pre_head;
public:
	template<bool IsConst>
	class list_iterator {
		friend forward_list<T>;
		public:
			using value_type = T;
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using pointer = typename std::conditional<IsConst, T const*, T*>::type;
			using reference = typename std::conditional<IsConst, T const&, T&>::type;
			// default copy construtor 
			list_iterator(list_iterator const&) = default;
			// converting constructor that converts between const and non-cost iterators.
			// hat tip to https://quuxplusone.github.io/blog/2018/12/01/const-iterator-antipatterns/https://quuxplusone.github.io/blog/2018/12/01/const-iterator-antipatterns/
			template<bool WasConst, typename = std::enable_if_t<IsConst || !WasConst>>
				list_iterator(list_iterator<WasConst> const& that) : itr_curr(that.itr_curr) {}
			list_iterator(base_node* c) :itr_curr(c) {}	
			// prefix incrament
			list_iterator<IsConst>& operator++() {
				itr_curr = itr_curr->next;
				return *this;
			}
			// postfix incrament
			list_iterator<IsConst> operator++(int) {
				auto cp = itr_curr;
				itr_curr = itr_curr->next;
				return const_iterator(cp);
			}
			// derference operator
			reference operator*() {
				return static_cast<derived_node*>(itr_curr)->val; 
			}
			bool operator!=(iterator that) const {
				return this->itr_curr != that.itr_curr;
			}
			
		private:
			base_node* itr_curr;

	};

};
}
