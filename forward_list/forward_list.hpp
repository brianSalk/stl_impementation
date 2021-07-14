#include "forward_list.h"
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <utility>
namespace brian {
	// constructors
	template<typename T, typename Allocator>
	forward_list<T,Allocator>::forward_list() {
		pre_head = new base_node();
	}
	template<typename T, typename Allocator>
	forward_list<T,Allocator>::forward_list(Allocator const& alloc) :forward_list() {
		std::cerr << "not yet implemented\n";				
	}
	
	template <typename T, typename Allocator>
	forward_list<T,Allocator>::forward_list(size_type count, Allocator const& alloc) : forward_list() {
			
		base_node* curr = pre_head;
		for (size_t i = 0; i < count; ++i) {
			curr->next = static_cast<base_node*>(new derived_node(T()));
			curr = curr->next;
		}
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(std::initializer_list<T> const& il, Allocator const& alloc) : forward_list() {
		derived_node* curr = static_cast<derived_node*>(pre_head);
		for (auto const& each : il)	{
			curr->next = static_cast<base_node*>(new derived_node(each));
			curr = static_cast<derived_node*>(curr->next);
		}
	
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::push_front( T const& val) {
		base_node* old_head = pre_head->next;
		derived_node* new_node = Traits::allocate(node_allocator, 1);
		Traits::construct(node_allocator,new_node, val);
		pre_head->next = static_cast<base_node*>(new_node);
		pre_head->next->next = old_head;
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::push_front(T &&val) {
		base_node* old_head = pre_head->next;
		derived_node* new_node = Traits::allocate(node_allocator,1);
		Traits::construct(node_allocator, new_node,val);
		pre_head->next = static_cast<base_node*>(new_node);
		pre_head->next->next = old_head;
	}
	template <typename T, typename Allocator>
	template <typename ...Args>
	void forward_list<T, Allocator>::emplace_front(Args && ...args) {
		base_node* old_head = pre_head->next;
		derived_node* new_head = Traits::allocate(node_allocator, 1);
		Traits::construct(node_allocator, new_head, std::forward<Args>(args)...);
		pre_head->next = static_cast<base_node*>(new_head);
		pre_head->next->next = old_head;
	}
	template<typename T, typename Allocator>
	typename forward_list<T, Allocator>::iterator forward_list<T, Allocator>::insert_after(const_iterator pos,T const& val) {
		base_node* curr = pos.itr_curr;	
		base_node* next_node = curr->next;
		curr->next = static_cast<base_node*>(new derived_node(val));
		curr->next->next = next_node;
		return iterator(curr);
	}
	template <typename T, typename Allocator>
	typename forward_list<T, Allocator>::iterator forward_list<T, Allocator>::insert_after(const_iterator pos, T&& val) {
		base_node* curr = pos.itr_curr;
		base_node* next_node = curr->next;
		curr->next = static_cast<base_node*>(new derived_node(std::move(val)));
		curr->next->next = next_node;
		return iterator(curr);
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::~forward_list() {
		derived_node* curr = static_cast<derived_node*>(this->begin().itr_curr);	
		derived_node* temp = curr;
		while (curr != nullptr) {
			temp = static_cast<derived_node*>(curr->next);
			Traits::destroy(node_allocator, curr);
			Traits::deallocate(node_allocator, curr, 1);
			curr = temp;
		}
		base_node* before_begin_node = this->before_begin().itr_curr;
		delete before_begin_node;
	}

}
