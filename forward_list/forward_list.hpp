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
		value_allocator = alloc;
	}
	
	template <typename T, typename Allocator>
	forward_list<T,Allocator>::forward_list(size_type count, Allocator const& alloc) : forward_list(alloc) {
		base_node* curr = pre_head;
		for (size_t i = 0; i < count; ++i) {
			derived_node* new_node = Traits::allocate(node_allocator, 1);	
			Traits::construct(node_allocator,new_node,T());
			curr->next = static_cast<base_node*>(new_node);
			curr = curr->next;
		}
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(std::initializer_list<T> const& il, Allocator const& alloc) : forward_list(alloc) {
		derived_node* curr = static_cast<derived_node*>(pre_head);
		try {
			for (auto const& each : il)	{
				derived_node* new_node = Traits::allocate(node_allocator, 1);
				Traits::construct(node_allocator, new_node, each);
				curr->next = static_cast<base_node*>(new_node);
				curr = static_cast<derived_node*>(curr->next);
			}
		}
		catch (...) {
			std::cerr << "call to constructor unsuccessful\n";
			this->clear();
		}
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(size_type count, T const& value, Allocator const& alloc) : forward_list(alloc) {
		base_node* curr = pre_head;
		for (size_t i {0}; i < count; ++i) {
			derived_node* new_node = Traits::allocate(node_allocator, 1);
			Traits::construct(node_allocator, new_node, value);
			curr->next = static_cast<base_node*>(new_node);
			curr = curr->next;
		}
	}
	template <typename T, typename Allocator>
	template <typename It, typename std::iterator_traits<decltype(std::declval<It>)>::pointer>
	forward_list<T,Allocator>::forward_list(It first, It last, Allocator const& alloc) : forward_list(alloc) {
		derived_node* curr = static_cast<derived_node*>(pre_head);
		for (auto it = first; it != last; ++it) {
			derived_node* new_node = Traits::allocate(node_allocator,1);
			Traits::construct(node_allocator, new_node, *it);
			curr->next = new_node;
			curr = static_cast<derived_node*>(curr->next);
		}
	}
	// modifiers
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
		derived_node* new_head = nullptr;
		try {
			new_head = create_node(std::forward<Args>(args)...);
		}
		catch (...) {
			std::cerr << "could not emplace_front\n";
			return;
		}
		pre_head->next = static_cast<base_node*>(new_head);
		pre_head->next->next = old_head;
	}
	template<typename T, typename Allocator>
	typename forward_list<T, Allocator>::iterator forward_list<T, Allocator>::insert_after(const_iterator pos,T const& val) {
		base_node* curr = pos.itr_curr;	
		base_node* new_node = curr->next;
		try {
			derived_node* new_node = create_node(val);
		}
		catch (...) {
			std::cerr << "element with value " << val << "not inserted due to exception\n";
			return iterator(pos.itr_curr);
		}
		curr->next = static_cast<base_node*>(new_node);
		curr->next->next = new_node;
		return iterator(curr);
	}
	template <typename T, typename Allocator>
	typename forward_list<T, Allocator>::iterator forward_list<T, Allocator>::insert_after(const_iterator pos, T&& val) {
		base_node* curr = pos.itr_curr;
		base_node* next_node = curr->next;
		derived_node* new_node = nullptr;
		try {
			new_node = create_node(std::move(val));
		} catch (...) {
			std::cerr << "value " << val << " not inserted due to exception\n";
			return iterator(pos.itr_curr);
		}
		curr->next = static_cast<base_node*>(new_node);
		curr->next->next = next_node;
		return iterator(curr);
	}
	template <typename T, typename Allocator>
	typename forward_list<T,Allocator>::iterator forward_list<T, Allocator>::insert_after(const_iterator pos, size_type count, T const& val) {
		if (count == 0) return iterator(pos.itr_curr);
		base_node* pos_node = pos.itr_curr;
		// create list starting from sublist_head inside try block, then after try-block connect it to the rest of your list
		derived_node* sublist_head = nullptr;
		derived_node* curr = nullptr;
		derived_node* new_node = nullptr;
		try {
			sublist_head = Traits::allocate(node_allocator,1);
			Traits::construct(node_allocator, sublist_head, val);
			curr = sublist_head;
			for (size_t i {0}; i < count-1; ++i) {
				new_node = Traits::allocate(node_allocator, 1);
				Traits::construct(node_allocator, new_node, val);
				curr->next = static_cast<base_node*>(new_node);
				curr = static_cast<derived_node*>(curr->next);
			}
		} catch (...) {
			std::cerr << "insert_after of " << count << " elements unsuccessful, your data structure has not been modified\n";
			// now clean up memory
			Traits::destroy(node_allocator, new_node);
			Traits::deallocate(node_allocator, new_node, 1);
			curr = sublist_head;
			derived_node* temp = nullptr;
			while (curr != nullptr) {
				temp = curr;
				curr = static_cast<derived_node*>(curr->next);
				Traits::destroy(node_allocator, temp);
				Traits::deallocate(node_allocator, temp, 1);
			}
			return iterator(pos.itr_curr);
		}
		curr->next = pos.itr_curr->next;
		pos.itr_curr->next = sublist_head;
		return iterator(static_cast<base_node*>(curr));
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::~forward_list() {
		clear();
		delete pre_head;
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::clear() noexcept {
		derived_node* curr = static_cast<derived_node*>(pre_head->next);
		derived_node* temp  = curr;
		try {
			while (curr != nullptr) {
				temp = curr;
				curr = static_cast<derived_node*>(curr->next);
				Traits::destroy(node_allocator, temp);
				Traits::deallocate(node_allocator, temp, 1);
			}
		}
		catch (...) {
			std::cerr << "element threw an exception upon destruction/deallocation, you may have leaked memory\n";
			pre_head->next = nullptr;

		}
		pre_head->next = nullptr;

	}
}
