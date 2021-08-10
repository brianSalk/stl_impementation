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
list<T, Allocator>::list(allocator_type const& alloc) : list() {
	this->value_allocator = alloc;
}
template <typename T, typename Allcoator> 
list<T, Allcoator>::list(size_type count, T const& val, Allcoator const& alloc) : list(alloc) {
	base_node* curr = pre_head;
	for (size_t i{0};i < count; ++i) {
		node* new_node = create_node(curr,val);
		curr->next = new_node;
		curr = curr->next;
	}
	n = count;
}
template <typename T, typename Allocator>
list<T, Allocator>::list(std::initializer_list<T> il, Allocator const&) : list() {
	auto beg = il.begin();
	__insert(begin(),beg, il.end());
}
template <typename T, typename Allocator>
template <typename It, typename std::iterator_traits<It>::pointer>
list<T, Allocator>::list(It first, It last) :list() {
	// FIX ME: this is a test, replace with real code
	__insert(iterator(begin()), first, last);
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
	return __insert(pos,beg,end);
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
