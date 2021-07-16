#include "forward_list.h"
#include <concepts>
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
			derived_node* new_node = create_default_node();
			curr->next = static_cast<base_node*>(new_node);
			curr = curr->next;
		}
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(std::initializer_list<T> const& il, Allocator const& alloc) : forward_list(alloc) {
		derived_node* curr = static_cast<derived_node*>(pre_head);
		derived_node* new_node = nullptr;
		try {
			for (auto const& each : il)	{
				new_node = create_node(each);
				curr->next = static_cast<base_node*>(new_node);
				curr = static_cast<derived_node*>(curr->next);
			}
		}
		catch (...) {
			std::cerr << "call to constructor unsuccessful\n";
			this->clear();
		}
	}
	// T must be CopyInsertable
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(size_type count, T const& value, Allocator const& alloc) : forward_list(alloc) {
		base_node* curr = pre_head;
		derived_node* new_node = nullptr;
		try {
			for (size_t i {0}; i < count; ++i) {
				new_node = create_node(value);
				curr->next = static_cast<base_node*>(new_node);
				curr = curr->next;
			}
		}
		catch (...) {
			clear();
			std::cerr << "constructor call unsuccessful\n";
			delete_node(new_node);
		}
	}
	template <typename T, typename Allocator>
	template <typename It, typename std::iterator_traits<It>::pointer>
	forward_list<T,Allocator>::forward_list(It first, It last, Allocator const& alloc) : forward_list(alloc) {
		derived_node* curr = static_cast<derived_node*>(pre_head);
		derived_node* new_node = nullptr;
		try {
			for (auto it = first; it != last; ++it) {
				new_node = create_node(*it);
				curr->next = new_node;
				curr = static_cast<derived_node*>(curr->next);
			}
		}
		catch (...) {
			std::cerr << "call to constructor unsuccessful\n";
			clear();
			delete_node(new_node);
		}
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> const& other):forward_list() {
		// copy all of other to this.
		base_node* curr = this->pre_head;
		derived_node* other_curr = static_cast<derived_node*>(other.pre_head->next);
		derived_node* new_node = nullptr;
		try {
			while (other_curr != nullptr) {
				// why does this not call create_node(T const& val) ?
				// why does other->val not evaluate to type T?
				new_node = create_node(other_curr->val);
				curr->next = static_cast<base_node*>(new_node);
				curr = curr->next;
				other_curr = static_cast<derived_node*>(other_curr->next);
			}
		}
		catch(...) {
			std::cerr << "constructor call unsuccessfull\n";
			clear();
			delete_node(new_node);
		}
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> const& other, Allocator const& alloc) : forward_list(other){
		value_allocator = alloc;
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> && other) {
		pre_head = other.pre_head;
		other.pre_head = nullptr;
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> && other, Allocator const& alloc) : forward_list(std::move(other)) {
		value_allocator = alloc;
	}
	// modifiers
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::push_front( T const& val) {
		base_node* old_head = pre_head->next;
		derived_node* new_node = nullptr;
		try {
			new_node = create_node(val);
		}
		catch (...) {
			std::cerr << "push_front unsuccessful\n";
			delete_node(new_node);
			return;
		}
		pre_head->next = static_cast<base_node*>(new_node);
		pre_head->next->next = old_head;
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::push_front(T &&val) {
		base_node* old_head = pre_head->next;
		derived_node* new_node = nullptr; 
		try {
			new_node = create_node(std::move(val));
		} 
		catch (...) {
			std::cerr << "push_front unsuccessful\n";
			delete_node(new_node);
			return;
		}
		pre_head->next = static_cast<base_node*>(new_node);
		pre_head->next->next = old_head;
	}
	// I am not sure if a type-catch block in nessasary to provide exception safety
	// for any of the destructive methods
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::pop_front() {
		base_node* old_head = pre_head->next;
		pre_head->next = old_head->next;
		delete_node(old_head);
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
			delete_node(new_head);
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
			std::cerr << "element with value not inserted due to exception\n";
			delete_node(new_node);
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
			delete_node(new_node);
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
				delete_node(temp);
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
		if (pre_head == nullptr) return;
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

		}
		pre_head->next = nullptr;
	}
	// observers
	template <typename T, typename Allocator>
	T& forward_list<T, Allocator>::front() {
		return static_cast<derived_node*>(pre_head->next)->val;	
	}
	template <typename T, typename Allocator>
	T const& forward_list<T, Allocator>::front() const {
		return static_cast<derived_node*>(pre_head->next)->val;
	}
}
