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
				//new_node = create_node(value);
				new_node = Traits::allocate(node_allocator,1);
				Traits::construct(node_allocator,new_node,value);
				curr->next = static_cast<base_node*>(new_node);
				curr = curr->next;
			}
		}
		catch (...) {
			clear();
			std::cerr << "constructor call unsuccessful\n";
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
	// allocator extended copy constructor
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> const& other, Allocator const& alloc) : forward_list(other){
		value_allocator = alloc;
	}
	// move constructor
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> && other) {
		value_allocator = std::move(other.value_allocator);
		pre_head = other.pre_head;
		other.pre_head = nullptr;
	}
	// move constructor allocator extended
	// the else condition is probably wrong and needs to be tested
	template <typename T, typename Allocator>
	forward_list<T, Allocator>::forward_list(forward_list<T,Allocator> && other, Allocator const& alloc) {
		if (alloc == other.get_allocator()) {
			value_allocator = std::move(other.value_allocator);
			pre_head = other.pre_head;
		} else {
			value_allocator = std::move(other.value_allocator);
			base_node* other_curr = other.pre_head;
			base_node* this_curr = this->pre_head;
			base_node* other_temp;
			// point this_curr to other_curr then point other curr off
			while (other_curr != nullptr) {
				other_temp = other_curr;
				other_curr = other_curr->next;
				this_curr = std::move(other_temp);
			}
		}
		other.pre_head = nullptr;
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
			while (other_curr != nullptr) {
				this_temp->next = static_cast<base_node*>(create_node(static_cast<derived_node*>(other_curr)->val));
				this_temp = this_temp->next;
				other_curr = other_curr->next;
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
			while (other_curr != nullptr) {
				derived_node* new_node = create_node(std::move(static_cast<derived_node*>(other_curr)->val));
				this_curr->next = static_cast<base_node*>(new_node);
				this_curr = this_curr->next;
				other_curr = other_curr->next;
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
			return *this;
		}
		
		this->clear();
		this->pre_head->next = static_cast<base_node*>(temp_head);
		return *this;
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
			new_node = create_node(val);
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
//			sublist_head = Traits::allocate(node_allocator,1);
//			Traits::construct(node_allocator, sublist_head, val);
			sublist_head = creaet_node(val);
			
			curr = sublist_head;
			for (size_t i {0}; i < count-1; ++i) {
//				new_node = Traits::allocate(node_allocator, 1);
//				Traits::construct(node_allocator, new_node, val);
				new_node = create_node(val);
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
//			curr = Traits::allocate(node_allocator, 1);
//			Traits::construct(node_allocator, curr, *first);
			curr = create_node(*first);
			temp_head = curr;
			for (auto other_it = ++first; other_it != last; ++other_it) {
				// allocate and construct new node
				new_node = create_node(std::move(*other_it));
//				new_node = Traits::allocate(node_allocator, 1);
//				Traits::construct(node_allocator, curr, *other_it);
				curr->next = new_node;
				curr = static_cast<derived_node*>(curr->next);
			}
		} catch (...) {
			std::cerr << "insert_after failed\n";
		
			derived_node* temp = temp_head;
			derived_node* c = temp_head;
			while(c != nullptr) {
				temp = c;
				std::cout << "temp->val = " << temp->val << '\n';
				c = static_cast<derived_node*>(c->next);
				delete_node(temp);
				std::cout << "deleted\n";
			}
			return iterator(pos.itr_curr);
		}
		// if no exception was thrown, insert the new list
		this_first.itr_curr->next = temp_head;
		curr->next = this_rest.itr_curr; 
		return iterator(curr);
	}
	template <typename T, typename Allocator>
	typename forward_list<T,Allocator>::iterator
	forward_list<T,Allocator>::insert_after(const_iterator pos, std::initializer_list<T> il) {
		derived_node* temp_head;
		derived_node* temp_curr;
		try {

			auto il_it = il.begin();
			temp_head = create_node(*il_it++);
			temp_curr = temp_head;
			for (auto it = il_it; it != il.end(); ++it) {
				temp_curr->next = static_cast<base_node*>(create_node(*it));
				temp_curr = static_cast<derived_node*>(temp_curr->next);
			}
		}
		catch (...) {
			std::cerr << "insert_after failed\n";
			derived_node* rm_curr = temp_head;
			derived_node* rm_temp;
			while (rm_curr != nullptr) {
				rm_temp = rm_curr;
				rm_curr = static_cast<derived_node*>(rm_curr->next);
				delete_node(rm_temp);
			}
			return iterator(pos.itr_curr);
		}
		derived_node* rest_of_this = static_cast<derived_node*>(pos.itr_curr->next);
		pos.itr_curr->next = static_cast<derived_node*>(temp_head);
		temp_curr->next = static_cast<base_node*>(rest_of_this);		
		return iterator(temp_curr);
	}
	template <typename T, typename Allocator>
	template <typename ... Args>
	typename forward_list<T,Allocator>::iterator forward_list<T, Allocator>::emplace_after(const_iterator pos, Args && ...args) {
		derived_node* new_node = nullptr;
		try {
//			new_node = Traits::allocate(node_allocator,1);
//			Traits::construct(node_allocator,new_node, std::forward<Args>(args)...);
			new_node = create_node(std::forward<Args>(args)...);
		} catch (...) {
			std::cerr << "emplace_after failed\n";
			delete_node(new_node);
			return iterator(pos.itr_curr);
		}
		base_node* next_node = pos.itr_curr->next;
		pos.itr_curr->next = static_cast<base_node*>(new_node);
		new_node->next = next_node;
		return iterator(new_node);
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
				delete_node(temp);
			}
		}
		catch (...) {
			std::cerr << "element threw an exception upon destruction/deallocation, you may have leaked memory\n";

		}
		pre_head->next = nullptr;
	}
	template <typename T, typename Allocator>
	void forward_list<T,Allocator>::swap(forward_list& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value) {
		base_node* tmp = other.pre_head;
		other.pre_head = this->pre_head;
		this->pre_head = tmp;
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
	// algorithms
	// this algorithm provides a basic exception guarentee, meaning that
	// if an element throws upon deletion, the forward list will be left in a 
	// valid state and no memory is leaked
	template <typename T, typename Allocator>
	std::size_t	forward_list<T,Allocator>::remove(T const& val) {
		return __remove([val](T const& v){return val == v;});
	}
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

} 
