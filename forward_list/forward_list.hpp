#pragma once
#include "forward_list.h"
#include <bits/c++config.h>
#include <concepts>
#include <ctime>
#include <forward_list>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <utility>
#include <limits>
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
		try {
			for (size_t i = 0; i < count; ++i) {
				derived_node* new_node = create_default_node();
				curr->next = static_cast<base_node*>(new_node);
				curr = curr->next;
			}
		}
		catch(...) {
			~forward_list();
			throw;
		}
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(std::initializer_list<T> const& il, Allocator const& alloc) : forward_list(alloc) {
		derived_node* curr = static_cast<derived_node*>(pre_head);
		derived_node* new_node = nullptr;
		// QUESTION: why can i not catch this exception
		try {
			for (auto const& each : il)	{
				//new_node = create_node(each);
//				new_node = Traits::allocate(node_allocator, 1);
//				Traits::construct(node_allocator, new_node, each);
				new_node = create_node(each);
				curr->next = static_cast<base_node*>(new_node);
				curr = static_cast<derived_node*>(curr->next);
			}
		}
		catch (...) {
			std::cerr << "call to constructor unsuccessful\n";
			this->~forward_list());
			throw;
		}
	}
	// T must be CopyInsertable
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(size_type count, T const& value, Allocator const& alloc) : forward_list(alloc) {
		base_node* curr = pre_head;
		derived_node* new_node = nullptr;
		try {
			for (size_t i {0}; i < count; ++i) {
				//new_node = create_node(value);
				new_node = Traits::allocate(node_allocator,1);
				Traits::construct(node_allocator,new_node,value);
				curr->next = static_cast<base_node*>(new_node);
				curr = curr->next;
			}
		}
		catch (...) {
			~forward_list();
			throw;
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
			~forward_list();
			throw;
		}
	}
	// copy constructor
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> const& other):forward_list() {
		// FIX ME: Do I need to create a new node_allocator after this?
		value_allocator = std::allocator_traits<allocator_type>::select_on_container_copy_construction(other.get_allocator()); 
		base_node* curr = this->pre_head;
		derived_node* other_curr = static_cast<derived_node*>(other.pre_head->next);
		derived_node* new_node = nullptr;
		try {
			while (other_curr != nullptr) {
				new_node = create_node(other_curr->val);
				curr->next = static_cast<base_node*>(new_node);
				curr = curr->next;
				other_curr = static_cast<derived_node*>(other_curr->next);
			}
		}
		catch(...) {
			~forward_list();
			throw;
		}
	}
	// allocator extended copy constructor
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> const& other, Allocator const& alloc) : forward_list(other){
		value_allocator = alloc;
	}
	// move constructor
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> && other) : forward_list() {
		value_allocator = std::move(other.value_allocator);
		pre_head->next = other.pre_head->next;
		other.pre_head->next = nullptr;
	}
	// move constructor allocator extended
	// the else condition is probably wrong and needs to be tested
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> && other, Allocator const& alloc) : forward_list() {
		if (alloc == other.get_allocator()) {
			std::cout << "not in else\n";
			value_allocator = std::move(other.value_allocator);
			this->pre_head->next = other.pre_head->next;
		} else {
			std::cout << "in else\n";
			value_allocator = std::move(other.value_allocator);
			base_node* other_curr = other.pre_head->next;
			base_node* this_curr = this->pre_head;
			base_node* other_temp;
			// point this_curr to other_curr then point other curr off
			while (other_curr != nullptr) {
				other_temp = other_curr;
				other_curr = other_curr->next;
				this_curr->next = std::move(other_temp);
				this_curr = this_curr->next;
			}
		}
		other.pre_head->next = nullptr;
	}
	// operators
	// copy assignment
	template <typename T, typename Allocator>
	forward_list<T,Allocator>& forward_list<T, Allocator>::operator=(forward_list const& other) {
		// check if we shoud assign other.get_allocator() to this
		if (std::allocator_traits<decltype(other.value_allocator)>::propagate_on_container_copy_assignment::value) {
			auto old_alloc = value_allocator;
			auto new_alloc = other.value_allocator;
			// if old != new, deallocate with old_alloc and allocate with new_alloc
			if (old_alloc != new_alloc) {
				clear();
				value_allocator = new_alloc;
				base_node* this_curr = this->pre_head;
				base_node* other_curr = other.pre_head->next;

				while (other_curr != nullptr) {
					derived_node* new_node;
					new_node = create_node(static_cast<derived_node*>(other_curr)->val);
					this_curr->next = static_cast<base_node*>(new_node);
					this_curr = this_curr->next;
					other_curr = other_curr->next;
				}
				return *this;
			} 
		}
		// if we do not want to assign other.get_allocator() to this, then the old and new iterators will be equal
		// if old_alloc == new_alloc, you may reuse the memory and just reassign the values	
		base_node* this_curr = this->pre_head->next;
		base_node* this_temp = this->pre_head;
		base_node* other_curr = other.pre_head->next;
		while (this_curr != nullptr && other_curr != nullptr) {
			this_temp = this_curr;
			static_cast<derived_node*>(this_curr)->val = static_cast<derived_node*>(other_curr)->val;
			this_curr = this_curr->next;
			other_curr = other_curr->next;
		}
		// if this is longer than other, we need to delete the extra nodes in this
		if (this_curr != nullptr && other_curr == nullptr) {
			// delete extra nodes in this, point temp to nullptr
			this_temp->next = nullptr;
			while (this_curr != nullptr) {
				this_temp = this_curr;
				this_curr = this_curr->next;
				delete_node(this_temp);
			}
		}
		// if other is longer than this, we need to allocate more space to add the extra nodes to this
		else if (this_curr == nullptr && other_curr != nullptr) {
			try {
			while (other_curr != nullptr) {
				this_temp->next = static_cast<base_node*>(create_node(static_cast<derived_node*>(other_curr)->val));
				this_temp = this_temp->next;
				other_curr = other_curr->next;
			}
			} catch (...) {
				this_temp->next = nullptr;
				throw;
			}
			this_temp->next = nullptr;
		}
		// if they are both the same length, just point this_temp->next to nullptr
		else {
			this_temp->next = nullptr;
		}
		
		return *this;
	}
	// move assignment
	template <typename T, typename Allocator>
	forward_list<T,Allocator>& forward_list<T,Allocator>::operator=(forward_list && other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value) {
		if (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
			this->value_allocator = std::move(other.get_allocator());
		}	
		if (this->value_allocator == other.get_allocator()) {
		// now empty this
		this->clear();
		// this points to first value of other
		this->before_begin().itr_curr->next = other.begin().itr_curr;
		// other is now empty and valid
		other.before_begin().itr_curr->next = nullptr;
		return *this;
		}
		else {
			// if we got here we ARE NOT propagating the other allocator, so just use the old one
			// first we clear this
			this->clear();
			// now we loop through other and move each element to this
			base_node* other_curr = other.begin().itr_curr;
			base_node* this_curr = this->before_begin().itr_curr;
			try {
			while (other_curr != nullptr) {
				derived_node* new_node = create_node(std::move(static_cast<derived_node*>(other_curr)->val));
				this_curr->next = static_cast<base_node*>(new_node);
				this_curr = this_curr->next;
				other_curr = other_curr->next;
			}
			} catch (...) {
				this_curr->next = nullptr;
				throw;
			}
			other.pre_head->next = nullptr;
			return *this;
		}
	} 
	template <typename T, typename Allocator>
	forward_list<T,Allocator>& 
	forward_list<T,Allocator>::operator=(std::initializer_list<T> il) {
		// QUESTION: can I reassign the existing nodes in this	
		//+or do I need to delete them all and reallocate them?
		derived_node* temp_curr;
		derived_node* temp_head;
		auto il_it = il.begin();
		try {
			temp_head = create_node(*il_it++);
			temp_curr = temp_head;
			for (auto it = il_it; it != il.end(); ++it) {
				temp_curr->next = static_cast<base_node*>(create_node(*it));
				temp_curr = static_cast<derived_node*>(temp_curr->next);
			}	
		} catch (...) {
			std::cerr << "operator= failed\n";
			derived_node* rm_curr = temp_head;
			derived_node* rm_temp;
			while (rm_curr != nullptr) {
				rm_temp = rm_curr;
				rm_curr = static_cast<derived_node*>(rm_curr->next);
				delete_node(rm_temp);
			}
			throw;
		}
		
		this->clear();
		this->pre_head->next = static_cast<base_node*>(temp_head);
		return *this;
	}
	
	// modifiers
	// assign provides a basic exception guarentee
	// if an excption is thrown from assign, there will be no leaks
	template <typename T, typename Allocator>
	void forward_list<T,Allocator>::assign(size_type count, T const& val) {
		derived_node* this_curr = static_cast<derived_node*>(this->begin().itr_curr);
		base_node* temp = this->before_begin().itr_curr;
		size_type this_count = 0;
		while (this_curr != nullptr && this_count < count) {
			temp = this_curr;
			this_curr->val = val;
			this_curr = static_cast<derived_node*>(this_curr->next);
			++this_count;
		}
		if (this_count < count) {
			while (this_count < count) {
				// we need to allocate more nodes
				temp->next = static_cast<base_node*>(create_node(val));
				temp = temp->next;
				++this_count;
			}
		}
		else {
			// we need to delete the extra nodes
			base_node* last_node = temp;
			temp = temp->next;
			base_node* del_node;
			while (temp != nullptr) {
				del_node = temp;
				temp = temp->next;
				delete_node(del_node);
			}
			last_node->next=nullptr;
		}
	}
	template <typename T, typename Allocator>
	template <typename It, typename std::iterator_traits<It>::pointer>
	void forward_list<T,Allocator>::assign(It first, It last) {
		derived_node* this_curr = static_cast<derived_node*>(this->begin().itr_curr);
		base_node* temp = this->before_begin().itr_curr;
		while (first != last && this_curr != nullptr) {
			temp = this_curr;
			this_curr->val = *first++;
			this_curr = static_cast<derived_node*>(this_curr->next);
		}
		if (first != last) {
			// we need to allocate more nodes
			while (first != last) {
				temp->next = create_node(*first++);
				temp = temp->next;
			}
		} else {
			// we need to delete nodes
			base_node* last_node = temp;
			base_node* del_node;
			temp = temp->next;
			while (temp != nullptr) {
				del_node = temp;
				temp = temp->next;
				delete_node(del_node);
			}
			last_node->next = nullptr;
		}
	}
	template <typename T, typename Allocator>
	void forward_list<T,Allocator>::assign(std::initializer_list<T> il) {
		this->assign(il.begin(), il.end());
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::push_front( T const& val) {
		base_node* old_head = pre_head->next;
		derived_node* new_node = nullptr;
		new_node = create_node(val);
		pre_head->next = static_cast<base_node*>(new_node);
		pre_head->next->next = old_head;
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::push_front(T &&val) {
		base_node* old_head = pre_head->next;
		derived_node* new_node = nullptr; 
		new_node = create_node(std::move(val));
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
		new_head = create_node(std::forward<Args>(args)...);
		pre_head->next = static_cast<base_node*>(new_head);
		pre_head->next->next = old_head;
	}
	template<typename T, typename Allocator>
	typename forward_list<T, Allocator>::iterator forward_list<T, Allocator>::insert_after(const_iterator pos,T const& val) {
		base_node* curr = pos.itr_curr;	
		base_node* new_node = curr->next;
		new_node = create_node(val);
		curr->next = static_cast<base_node*>(new_node);
		curr->next->next = new_node;
		return iterator(curr);
	}
	template <typename T, typename Allocator>
	typename forward_list<T, Allocator>::iterator forward_list<T, Allocator>::insert_after(const_iterator pos, T&& val) {
		base_node* curr = pos.itr_curr;
		base_node* next_node = curr->next;
		derived_node* new_node;
		new_node = create_node(std::move(val));
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
			sublist_head = create_node(val);
			curr = sublist_head;
			for (size_t i {0}; i < count-1; ++i) {
				new_node = create_node(val);
				curr->next = static_cast<base_node*>(new_node);
				curr = static_cast<derived_node*>(curr->next);
			}
		} catch (...) {
			// now clean up memory
			curr = sublist_head;
			derived_node* temp = nullptr;
			while (curr != nullptr) {
				temp = curr;
				curr = static_cast<derived_node*>(curr->next);
				delete_node(temp);
			}
			throw;
		}
		curr->next = pos.itr_curr->next;
		pos.itr_curr->next = sublist_head;
		return iterator(static_cast<base_node*>(curr));
	}
	template <typename T, typename Allocator>
	template <typename It, typename std::iterator_traits<It>::pointer >
	typename forward_list<T,Allocator>::iterator 
	forward_list<T,Allocator>::insert_after(const_iterator pos, It first, It last) {
		auto this_first = pos;			
		auto this_rest = ++pos;
		derived_node* temp_head = nullptr;
		derived_node* new_node = nullptr;
		derived_node* curr = nullptr;
		try {
			// create a new new list, if it succeeds, insert it into this,
			// if it fails, clean up all the memory
			curr = create_node(*first);
			temp_head = curr;
			for (auto other_it = ++first; other_it != last; ++other_it) {
				// allocate and construct new node
				new_node = create_node(*other_it);
				curr->next = new_node;
				curr = static_cast<derived_node*>(curr->next);
			}
		} catch (...) {
			derived_node* temp = temp_head;
			derived_node* c = temp_head;
			while(c != nullptr) {
				temp = c;
				c = static_cast<derived_node*>(c->next);
				delete_node(temp);
			}
			throw;
		}
		// if no exception was thrown, insert the new list
		this_first.itr_curr->next = temp_head;
		curr->next = this_rest.itr_curr; 
		return iterator(curr);
	}
	template <typename T, typename Allocator>
	typename forward_list<T,Allocator>::iterator
	forward_list<T,Allocator>::insert_after(const_iterator pos, std::initializer_list<T> il) {
		return insert_after(pos,il.begin(),il.end());
	}
	template <typename T, typename Allocator>
	template <typename ... Args>
	typename forward_list<T,Allocator>::iterator forward_list<T, Allocator>::emplace_after(const_iterator pos, Args && ...args) {
		derived_node* new_node;
		new_node = create_node(std::forward<Args>(args)...);
		base_node* next_node = pos.itr_curr->next;
		pos.itr_curr->next = static_cast<base_node*>(new_node);
		new_node->next = next_node;
		return iterator(new_node);
	}
	template <typename T, typename Allocator>
	typename forward_list<T,Allocator>::iterator
	forward_list<T,Allocator>::erase_after(const_iterator pos) {
		base_node* del_node = pos.itr_curr->next;
		pos.itr_curr->next = del_node->next;
		delete_node(del_node);
		return iterator(pos.itr_curr->next);
	}
	template <typename T, typename Allocator>
	typename forward_list<T, Allocator>::iterator
	forward_list<T,Allocator>::erase_after(const_iterator first, const_iterator last) {
		base_node* del_curr = first.itr_curr->next;
		base_node* del_temp = first.itr_curr;
		while (del_curr != last.itr_curr) {
			del_temp = del_curr->next;
			delete_node(del_curr);
			del_curr = del_temp;
		}
		first.itr_curr->next = last.itr_curr;
		return iterator(last.itr_curr);
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::resize(size_type new_size) {
		__resize(new_size);
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::resize(size_type new_size, T const& val) {
		__resize(new_size, val);
	}
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::~forward_list() {
		clear();
		delete pre_head;
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::clear() noexcept {
		// there should never be a situation where a list has a null pre_head
		// uncomment line below to debug for null pre_head
		//if (pre_head == nullptr) {std::cout << "FIX ME, pre_head is null\n";return;}
		base_node* curr = pre_head->next;
		base_node* temp  = curr;
		try {
			while (curr != nullptr) {
				temp = curr;
				curr = curr->next;
				delete_node(temp);
			}
		}
		// If a destructor throws (which it never should) you at least to get to clear the list
		catch (...) {
			std::cerr << "element threw an exception upon destruction/deallocation, you may have leaked memory\n";
		}
		pre_head->next = nullptr;
	}
	template <typename T, typename Allocator>
	void forward_list<T,Allocator>::swap(forward_list& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value) {
		__swap(typename std::allocator_traits<Allocator>::propagate_on_container_swap::type(),other);
	}
	template <typename T, typename Allocator>
	void forward_list<T,Allocator>::splice_after(const_iterator pos, forward_list& other) {
		__splice_after(pos,std::forward<forward_list>(other));
	}
	template <typename T, typename Allocator>
	void forward_list<T,Allocator>::splice_after(const_iterator pos, forward_list && other) {
		__splice_after(pos, std::forward<forward_list>(other));
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::splice_after(const_iterator pos, forward_list& other, const_iterator it) {
		__splice_after(pos, std::forward<forward_list>(other), it);
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::splice_after(const_iterator pos, forward_list&& other, const_iterator it) {
		__splice_after(pos, std::forward<forward_list>(other), it);
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::splice_after(const_iterator pos, forward_list & other, const_iterator first, const_iterator last) {
		__splice_after(pos, std::forward<forward_list>(other), first, last);
	}
	template <typename T, typename Allocator>
	void forward_list<T, Allocator>::splice_after(const_iterator pos, forward_list && other, const_iterator first, const_iterator last) {
		__splice_after(pos, std::forward<forward_list>(other), first, last);
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
	template <typename T, typename Allocator>
	size_t forward_list<T,Allocator>::max_size() const noexcept {
		// I think the standard says the answer to this is implementation defined.
		// but this should work...
		return std::allocator_traits<NodeAlloc_t>::max_size(node_allocator);
	}
	// algorithms
	// remove is no-throw
	template <typename T, typename Allocator>
	std::size_t	forward_list<T,Allocator>::remove(T const& val) {
		return __remove([val](T const& v){return val == v;});
	}
	// remove_if is no-throw unless pred throws,
	// then there is a basic guarentee
	template <typename T, typename Allocator>
	template<typename Pred>
	requires std::predicate<Pred,T>
	std::size_t forward_list<T,Allocator>::remove_if(Pred p) {
		return __remove(p);
	}
	template <typename T, typename Allocator>
	void forward_list<T,Allocator>::reverse() noexcept {
		base_node* one = pre_head->next, *two, *three;
		if (one && one->next) {
			two = one->next;
		} else {
			return;
		}
		if (two && two->next) {
			three = two->next;
		} else {
			two->next = one;
			pre_head->next = two;
			one->next = nullptr;
			return;
		}
		one->next = nullptr;
		while (three != nullptr) {
			two->next = one;
			one = two;
			two = three;
			three = three->next;
		}
		two->next = one;
		pre_head->next = two;
	}
	template <typename T, typename Allocator>
	std::size_t forward_list<T,Allocator>::unique() {
		return __unique([](T const& a, T const& b){return a == b;});
	}
	template <typename T, typename Allocator>
	template <typename Pred>
	requires std::predicate<Pred,T,T>
	std::size_t forward_list<T,Allocator>::unique(Pred p) {
		return __unique(p);
	}
	template <typename T, typename Allocator>
	void forward_list<T,Allocator>::merge(forward_list & other) {
		__merge(std::forward<forward_list>(*this), 
				std::forward<forward_list>(other), 
				[](T const& a, T const& b){return a < b;});
	}
	template <typename T, typename Allocator>
	void forward_list<T,Allocator>::merge(forward_list && other) {
		__merge(std::forward<forward_list>(*this),
				std::forward<forward_list>(other),
				[](T const& a, T const& b){return a < b;});
	}
	template <typename T, typename Allocator>
	template <typename Cmp>
	requires std::predicate<Cmp,T,T>
	void forward_list<T,Allocator>::merge(forward_list& other,Cmp cmp) {
		__merge(std::forward<forward_list>(*this),
				std::forward<forward_list>(other),
				cmp);
	}
	template <typename T, typename Allocator>
	template <typename Cmp>
	requires std::predicate<Cmp,T,T>
	void forward_list<T,Allocator>::merge(forward_list&& other, Cmp cmp) {
		__merge(std::forward<forward_list>(*this),
				std::forward<forward_list>(other),
				cmp);
	}
	template <typename T, typename Allocator>
	template <typename U>
	requires __less_than_comparable<U>
	void forward_list<T,Allocator>::sort() {
		__sort([](U const& a, U const& b){return a < b; });
	}
	template <typename T, typename Allocator>
	template <typename Cmp>
	requires std::predicate<Cmp,T,T>
	void forward_list<T,Allocator>::sort(Cmp cmp) {
		__sort(cmp);
	}
} 
