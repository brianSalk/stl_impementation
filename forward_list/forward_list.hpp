#include "forward_list.h"
#include <initializer_list>
#include <iostream>
#include <utility>
namespace brian {

	template<typename T>
	forward_list<T>::forward_list() {
		pre_head = new base_node();
	}
	template <typename T>
	forward_list<T>::forward_list(std::initializer_list<T> const& il) : forward_list() {
		
		derived_node* curr = static_cast<derived_node*>(pre_head);
		for (auto const& each : il)	{
			curr->next = static_cast<base_node*>(new derived_node(each));
			curr = static_cast<derived_node*>(curr->next);
		}
	
	}
	template <typename T>
	void forward_list<T>::push_front( T const& val) {
		base_node* old_head = pre_head->next;
		pre_head->next = static_cast<base_node*>(new derived_node(val));
		pre_head->next->next = old_head;
	}
	template <typename T>
	template <typename ...Args>
	void forward_list<T>::emplace_front(Args && ...args) {
		base_node* old_head = pre_head->next;
		pre_head->next = static_cast<base_node*>(new derived_node(std::forward<Args>(args)...));
		pre_head->next->next = old_head;
	}
	template<typename T>
	typename forward_list<T>::const_iterator forward_list<T>::insert_after(const_iterator pos,T const& val) {
		base_node* curr = pos.itr_curr;	
		base_node* next_node = curr->next;
		curr->next = static_cast<base_node*>(new derived_node(val));
		curr->next->next = next_node;
		return pos;
	}



}
