#pragma once
#include "list.h"
namespace brian { 
// constuctors
// default constructor
template <typename T, typename Allocator>
list<T,Allocator>::list() : pre_head(new base_node()),aft_tail(new base_node()) {}
// modifiers
template <typename T, typename Allocator>
typename list<T,Allocator>::iterator list<T,Allocator>::insert(const_iterator pos, T const&val) {
	base_node* before_node = pos.itr_curr->prev;
	base_node* new_node;
	// QUESTION: do I need to wrap this in a try block for exception safety?
	new_node = create_node(val);
	before_node->next = new_node;
	new_node->prev = before_node;
	new_node->next = pos.itr_curr;
	return iterator();
}
}// END OF NAMESPACE BRIAN
