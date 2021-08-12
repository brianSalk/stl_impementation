#pragma once
#include "list.h"
#include <initializer_list>
#include <iterator>
#include <memory>
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
template <typename T, typename Allcoator> 
list<T, Allcoator>::list(size_type count, T const& val, Allcoator const& alloc) : list(alloc) {
	base_node* curr = pre_head;
	for (size_t i{0};i < count; ++i) {
		node* new_node = create_node(curr,val);
		curr->next = new_node;
		curr = curr->next;
	}
	connect_nodes(curr,aft_tail);
	n = count;
}
// initialization list constructor
template <typename T, typename Allocator>
list<T, Allocator>::list(std::initializer_list<T> il, Allocator const& alloc) : list(alloc) {
	auto beg = il.begin();
	__insert(begin(),beg, il.end());
}
// range constructor
template <typename T, typename Allocator>
template <typename It, typename std::iterator_traits<It>::pointer>
list<T, Allocator>::list(It first, It last, Allocator const& alloc) :list(alloc) {
	__insert(iterator(begin()), first, last);
}
// copy constructor
template <typename T, typename Allocator>
list<T,Allocator>::list(list const& other) :list() {
	this->value_allocator = std::allocator_traits<Allocator>::select_on_container_copy_construction(other.get_allocator());
	base_node* other_curr = other.pre_head->next;
	base_node* this_curr = this->pre_head;
	while (other_curr != other.end()) {
		node* new_node = create_node(this_curr,static_cast<node*>(other_curr)->val);
		this_curr->next = new_node;	
		this_curr = new_node;
		other_curr = other_curr->next;
	}
	connect_nodes(this_curr,aft_tail);
	this->n = other.n;
}
// observers
template <typename T, typename Allcoator>
size_t list<T,Allcoator>::max_size() const noexcept {
	return Traits::max_size(node_allocator);
}
// modifiers
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
list<T,Allocator>::~list() {
	base_node* curr = pre_head;
	base_node* del_node;
	std::cout << "destructor\n";
	while (curr) {
		std::cout << "here\n";
		del_node = curr;
		curr = curr->next;
		delete_node(del_node);
	}
}
}// END OF NAMESPACE BRIAN
