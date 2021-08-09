#pragma once
#include "list.h"
#include <initializer_list>
#include <iterator>
namespace brian { 
// constuctors
// default constructor
template <typename T, typename Allocator>
list<T,Allocator>::list() : pre_head(new base_node()),aft_tail(new base_node()) {
	pre_head->next = aft_tail;
	aft_tail->prev = pre_head;
	n = 0;
}
template <typename T, typename Allocator>
list<T, Allocator>::list(std::initializer_list<T> il, Allocator const&) : list() {
	// FIX_ME: this needs to be optimized, but it works for now
	for (auto const& each : il) {
		insert(end(),each);
	}	
	n = il.size();
}
// modifiers
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
template <typename T, typename Allcoator>
template <typename It, typename std::iterator_traits<It>::pointer>
typename list<T,Allcoator>::iterator
list<T,Allcoator>::insert(const_iterator pos, It beg, It end) {
	// il.begin() and il.end() are equal if il is empty, so don't worry about reusing this for the std::initializer_list overload
	if (beg == end) return iterator(pos.itr_curr);
		base_node* temp_head = create_node(*beg);
		base_node* curr = temp_head;
	try {
		auto it = ++beg;
		for (it = beg; it != end;++it) {
			curr->next = create_node(curr,*it);
			curr = curr->next;
		}
	} catch(...) {
		// time to clean up
		base_node* del_node;
		curr = temp_head;
		while (curr) {
			del_node = curr;
			curr = curr->next;
			delete_node(del_node);
		}
		throw;
	}

	connect_nodes(pos.itr_curr->prev,temp_head);
	connect_nodes(curr,pos.itr_curr);
	return iterator(temp_head);
}
template <typename T, typename Allcoator>
typename list<T, Allcoator>::iterator
list<T, Allcoator>::insert(const_iterator pos, std::initializer_list<T> il) {
	return insert(pos,il.begin(), il.end());
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
