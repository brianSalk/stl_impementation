#pragma once
#include <iostream> // DELTE ME: debug only
#include "list.h"
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
namespace brian { 
// constuctors
// default constructor
template <typename T, typename Allocator>
list<T,Allocator>::list() : pre_head(new base_node()),aft_tail(new base_node()) {
	pre_head->next = aft_tail;
	aft_tail->prev = pre_head;
	n = 0;
}
// allocator constructor
template <typename T, typename Allocator>
list<T, Allocator>::list(allocator_type const& alloc) : list() {
	this->value_allocator = alloc;
}
// fill constructor
template <typename T, typename Allocator> 
list<T, Allocator>::list(size_type count, T const& val, Allocator const& alloc) : list(alloc) {
	base_node* curr = pre_head;
	try {
		for (size_t i{0};i < count; ++i) {
			node* new_node = create_node(curr,val);
			curr->next = new_node;
			curr = curr->next;
		}
	} catch (...) {
		// connect aft_tail to make list complete
		connect_nodes(curr,aft_tail);
		clear();
		delete_node(pre_head);
		pre_head = nullptr;
		delete_node(aft_tail);
		throw;
	}
	connect_nodes(curr,aft_tail);
	n = count;
}
// default fill construcotr
template <typename T, typename Allocator>
list<T,Allocator>::list(size_t count, Allocator const& alloc) : list(alloc) {
	base_node* curr = pre_head;
	try {
		for (size_t i{0};i < count; ++i) {
			node* new_node = create_node(curr);
			curr->next = new_node;
			curr = curr->next;
		}
	} catch (...) {
		connect_nodes(curr,aft_tail);
		clear();
		delete_node(pre_head);
		pre_head = nullptr;
		delete_node(aft_tail);
		throw;
	}
	connect_nodes(curr,aft_tail);
	n = count;
}
// initialization list constructor
template <typename T, typename Allocator>
list<T, Allocator>::list(std::initializer_list<T> il, Allocator const& alloc) : list(alloc) {
	auto beg = il.begin();
	try {
		__insert(begin(),beg, il.end());
	} catch (...) {
		delete_node(pre_head);
		delete_node(aft_tail);
		pre_head = nullptr;
	}
}
// range constructor
template <typename T, typename Allocator>
template <typename It, typename std::iterator_traits<It>::pointer>
list<T, Allocator>::list(It first, It last, Allocator const& alloc) :list(alloc) {
	try {
		__insert(iterator(begin()), first, last);
	} catch (...) {
		delete_node(pre_head);
		delete_node(aft_tail);
		pre_head=nullptr;
		throw;
	}
}
// copy constructor
template <typename T, typename Allocator>
list<T,Allocator>::list(list const& other) :list() {
	this->value_allocator = std::allocator_traits<Allocator>::select_on_container_copy_construction(other.get_allocator());
	__cp(other);
}
// copy constructor allocator-extended
template <typename T, typename Allocator>
list<T,Allocator>::list(list const& other, Allocator const& alloc) : list(alloc) {
	__cp(other);
}
// move constructor
template <typename T, typename Allocator>
list<T, Allocator>::list(list && other) :list() {
	this->value_allocator = std::move(other.value_allocator);
	connect_nodes(this->pre_head,other.pre_head->next);
	connect_nodes(other.aft_tail->prev,this->aft_tail);
	this->n = other.n;
	other.pre_head->next = other.aft_tail;
}
// move constructor allocator exctended
// FIX ME: exception safety with move semantics is hard!
template <typename T, typename Allocator>
list<T, Allocator>::list(list && other, Allocator const& alloc) :list() {
	if (alloc != other.get_allocator()) {
		this->value_allocator = std::move(other.value_allocator);
		node* other_curr = static_cast<node*>(other.pre_head->next);	
		base_node* this_curr = this->pre_head;
		while (other_curr != other.end()) {
			node* new_node = create_node(this_curr,std::move(other_curr->val));
			this_curr->next = new_node;
			this_curr = new_node;
			other_curr = static_cast<node*>(other_curr->next);
		}
		connect_nodes(this_curr,this->aft_tail);
		this->n = other.n;
	}
	this->value_allocator = std::move(other.value_allocator);
	connect_nodes(this->pre_head,other.pre_head->next);
	connect_nodes(other.aft_tail->prev,this->aft_tail);
	this->n = other.n;
	other.pre_head->next = other.aft_tail;
}
// observers
template <typename T, typename Allocator>
size_t list<T,Allocator>::max_size() const noexcept {
	return Traits::max_size(node_allocator);
}
// modifiers
template <typename T,typename Allocator>
void list<T,Allocator>::clear() noexcept {
	base_node* curr = pre_head->next;
	while (curr != end().itr_curr) {
		base_node* del_node = curr;	
		curr = curr->next;
		delete_node(del_node);
	}
	n = 0;
}
// insert
template <typename T, typename Allocator>
typename list<T,Allocator>::iterator 
list<T,Allocator>::insert(const_iterator pos, T const&val) {
	base_node* before_node = pos.itr_curr->prev;
	node* new_node = create_node(val);
	connect_nodes(before_node,new_node);
	connect_nodes(new_node,pos.itr_curr);
	++n;
	return iterator(new_node);
}
template <typename T, typename Allocator>
typename list<T,Allocator>::iterator
list<T,Allocator>::insert(const_iterator pos, T && val) {
	node* new_node = create_node(std::move(val));
	base_node* before_pos = pos.itr_curr->prev;
	connect_nodes(before_pos,new_node);
	connect_nodes(new_node, pos.itr_curr);
	++n;
	return iterator(new_node);
}
template <typename T, typename Allocator>
typename list<T,Allocator>::iterator
list<T,Allocator>::insert(const_iterator pos, size_t count, T const& val) {
	if (count == 0) return pos.itr_curr;
	base_node* temp_head = nullptr;
	base_node* curr;
	try {
		temp_head = create_node(val);
		curr = temp_head;	
		base_node* temp;	
		for (size_t i = 1; i < count; ++i) {
			base_node* new_node = create_node(curr,val);
			curr->next = new_node;
			curr = curr->next;
		}
	} catch (...) {
		// clean up memory here
		curr = temp_head;
		base_node* del_node;
		while (curr != nullptr) {
			del_node = curr;
			curr = curr->next;
			delete_node(del_node);
		}
		// propogate exception to user
		throw;
	}
	connect_nodes(pos.itr_curr->prev, temp_head);
	connect_nodes(curr,pos.itr_curr);
	n+=count;
	return iterator(temp_head);
}
template <typename T, typename Allocator>
template <typename It, typename std::iterator_traits<It>::pointer>
typename list<T,Allocator>::iterator
list<T,Allocator>::insert(const_iterator pos, It beg, It end) {
	return __insert(pos,beg,end);
}
template <typename T, typename Allocator>
typename list<T, Allocator>::iterator
list<T, Allocator>::insert(const_iterator pos, std::initializer_list<T> il) {
	return insert(pos,il.begin(), il.end());
}
template <typename T, typename Allocator>
void list<T, Allocator>::pop_back() {
	base_node* del_node = aft_tail->prev;
	connect_nodes(del_node->prev, aft_tail);
	delete_node(del_node);
	--n;
}
template <typename T, typename Allocator>
void list<T, Allocator>::pop_front() {
	base_node* del_node = pre_head->next;		
	connect_nodes(pre_head, del_node->next);
	delete_node(del_node);
	--n;
}
// erase
template <typename T, typename Allocator>
typename list<T, Allocator>::iterator
list<T, Allocator>::erase(const_iterator pos) {
	base_node* del_node = pos;
	connect_nodes(pos.itr_curr->prev, pos.itr_curr->next);
	delete_node(del_node);
	--n;
}
template <typename T, typename Allocator>
typename list<T,Allocator>::iterator
list<T,Allocator>::erase(const_iterator first, const_iterator last) {
	base_node* temp;
	base_node* end_of_first = first.itr_curr->prev;
	size_t to_add = 0;
	while (first != last) {
		temp = first.itr_curr;
		++first;
		delete_node(temp);
		++to_add;
	}
	n -= to_add;
	connect_nodes(end_of_first,last.itr_curr);
	auto ret = (last == end()) ? end() : last.itr_curr->next;
	return ret;
}
// pushers
template <typename T, typename Allocator>
void list<T, Allocator>::push_front(T const& val) {
	auto new_node = create_node(val);
	connect_nodes(new_node, pre_head->next);
	connect_nodes(pre_head, new_node);
	++n;
}

template <typename T, typename Allocator>
void list<T, Allocator>::push_front(T && val) {
	auto new_node = create_node(std::move(val));
	connect_nodes(new_node, pre_head->next);
	connect_nodes(pre_head, new_node);
	++n;
}
template <typename T, typename Allocator>
void list<T,Allocator>::push_back(T const& val) {
	auto new_node = create_node(val);	
	connect_nodes(aft_tail->prev, new_node);
	connect_nodes(new_node, aft_tail);
	++n;
}

template <typename T, typename Allocator>
void list<T,Allocator>::push_back(T && val) {
	auto new_node = create_node(std::move(val));	
	connect_nodes(aft_tail->prev, new_node);
	connect_nodes(new_node, aft_tail);
	++n;
}
// emplace methods
template <typename T, typename Allocator>
template <typename ...Args>
typename list<T,Allocator>::iterator
list<T,Allocator>::emplace(const_iterator pos, Args && ...args) {
	auto new_node = create_node(std::forward<Args>(args)...);
	connect_nodes(pos.itr_curr->prev,new_node);
	connect_nodes(new_node,pos.itr_curr);
	++n;
	return iterator(new_node);
}
template <typename T, typename Allocator>
template <typename ...Args>
void list<T,Allocator>::emplace_front(Args &&...args) {
		auto new_node = create_node(std::forward<Args>(args)...);
		connect_nodes(new_node, pre_head->next);
		connect_nodes(pre_head, new_node);
		++n;
}
template <typename T, typename Allocator>
template <typename ...Args>
void list<T,Allocator>::emplace_back(Args && ...args) {
	auto new_node = create_node(std::forward<Args>(args)...);	
	connect_nodes(aft_tail->prev,new_node);
	connect_nodes(new_node, aft_tail);
}
// resize provides a basic guarentee
// TO DO: benchmark and compare difference between using curr to delete
// or using pop_back() and push_back
template <typename T, typename Allocator>
void list<T,Allocator>::resize(size_t new_size) {
	__resize(new_size);
}
template <typename T, typename Allocator>
void list<T,Allocator>::resize(size_t new_size, T const& val) {
	__resize(new_size,val);
}
template <typename T, typename Allocator>
list<T,Allocator>::~list() {
	base_node* curr = pre_head;
	base_node* del_node;
	while (curr) {
		del_node = curr;
		curr = curr->next;
		delete_node(del_node);
	}
}

}// END OF NAMESPACE BRIAN
