#pragma once
#include "list.h"
namespace brian { 
// constuctors
// default constructor
template <typename T, typename Allocator>
list<T,Allocator>::list() : pre_head(new base_node()),aft_tail(new base_node()) {
	pre_head->next = aft_tail;
	aft_tail->prev = pre_head;
}
// modifiers
template <typename T, typename Allocator>
typename list<T,Allocator>::iterator list<T,Allocator>::insert(const_iterator pos, T const&val) {
	base_node* before_node = pos.itr_curr->prev;
	node* new_node = create_node(val);
	connect_nodes(before_node,new_node);
	connect_nodes(new_node,pos.itr_curr);
	return iterator(new_node);
}
template <typename T, typename Allocator>
typename list<T,Allocator>::iterator
list<T,Allocator>::insert(const_iterator pos, T && val) {
	node* new_node = create_node(std::move(val));
	base_node* before_pos = pos.itr_curr->prev;
	connect_nodes(before_pos,new_node);
	connect_nodes(new_node, pos.itr_curr);
	return iterator(new_node);
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
