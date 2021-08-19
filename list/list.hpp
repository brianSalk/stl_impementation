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
list<T,Allocator>::list() : pre_head(&PRE_HEAD),aft_tail(&AFT_TAIL) {
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
			node* new_node = create_node_with_hint(curr,curr,val);
			curr->next = new_node;
			curr = curr->next;
		}
	} catch (...) {
		curr->next = aft_tail;
		throw;
	}
	connect_nodes(curr,aft_tail);
	n = count;
}
// default fill constructor
template <typename T, typename Allocator>
list<T,Allocator>::list(size_t count, Allocator const& alloc) : list(alloc) {
	base_node* curr = pre_head;
	try {
		for (size_t i{0};i < count; ++i) {
			node* new_node = create_node_with_hint(curr,new_node,curr);
			curr->next = new_node;
			curr = curr->next;
			std::cout << "loop\n";
		}
	} catch (...) {
		connect_nodes(curr, aft_tail);
		throw;
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
		try {
			while (other_curr != other.end()) {
				node* new_node = create_node_with_hint(this_curr,this_curr,std::move(other_curr->val));
				this_curr->next = new_node;
				this_curr = new_node;
				other_curr = static_cast<node*>(other_curr->next);
			}
		} catch (...) {
			connect_nodes(this_curr, this->aft_tail);
			throw;
		}
		connect_nodes(this_curr,this->aft_tail);
		this->n = other.n;
	} else { 
		this->value_allocator = std::move(other.value_allocator);
		connect_nodes(this->pre_head,other.pre_head->next);
		connect_nodes(other.aft_tail->prev,this->aft_tail);
		this->n = other.n;
		other.pre_head->next = other.aft_tail;
	}
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
	connect_nodes(pre_head,aft_tail);
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
// assign
template <typename T, typename Allocator>
void list<T,Allocator>::assign(size_t new_size, T const& val) {
	auto curr = pre_head;
	size_t count = 0;
	if (count < new_size) {
		while (count < new_size && count < n) {
			static_cast<node*>(curr->next)->val = val;
			curr = curr->next;
			++count;
		}
		try {
			while (count++ < new_size) {
				curr->next = create_node(curr,val);
				curr = curr->next;
				n++;
			}
		} catch (...) {
			connect_nodes(curr,aft_tail);
			throw;
		}
	} 
	auto temp = curr->next;	
	while (temp != nullptr) {
		auto del_node = temp;
		temp = temp->next;
		delete_node(del_node);
	}
	connect_nodes(curr,aft_tail);
}
// assign with iterators
template <typename T, typename Allocator>
template <typename It, typename std::iterator_traits<It>::pointer>
void list<T,Allocator>::assign(It first, It last) {
	// first try to reassign preexisting nodes
	base_node* curr = pre_head->next;
	while (first != last && curr != end()) {
		static_cast<node*>(curr)->val = *first;
		++first;
		curr = curr->next;
	}
	if (first != last) {
		base_node* temp_head = create_node(*first);
		auto new_curr = temp_head;
		++first;
		try {
			while (first != last) {
				new_curr->next = create_node(new_curr,*first);
				new_curr = new_curr->next;
				++first;
				--n;
			}
		} catch (...) {
			connect_nodes(curr->prev,temp_head);
			connect_nodes(new_curr,aft_tail);
			throw;
		}
		connect_nodes(curr->prev,temp_head);
		connect_nodes(new_curr,aft_tail);
		return;
	}
	if (curr != end()) {
		auto new_curr = curr;
		curr = curr->prev;
		while (new_curr != end()) {
			base_node* del_node = new_curr;	
			new_curr = new_curr->next;
			delete_node(del_node);
			--n;
		}
		connect_nodes(curr,aft_tail);
	}
}
template <typename T, typename Allocator>
void list<T,Allocator>::assign(std::initializer_list<T> il) {
	assign(il.begin(), il.end());
}
// assignment operators
template <typename T, typename Allocator>
typename list<T,Allocator>::list<T,Allocator> &
list<T,Allocator>::operator=(list<T,Allocator> const& other) {
	if (std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value) {
		// if other.value_allocator does not equal this->value_allocator,
		// then the old allocator needs to clear all memory in this,
		// when allocators are unequal, that means that the allocators cannot deallocate eachothers memory
		if (this->value_allocator != other.value_allocator) {
			this->clear();
			delete_node(pre_head);
			delete_node(aft_tail);

			value_allocator = other.value_allocator;
			pre_head = new base_node();
			aft_tail = new base_node();
			pre_head->next = aft_tail;
			aft_tail->prev = pre_head;
			this->n = 0;
		} else {
			this->value_allocator = other.value_allocator;
		}
	}
	// now it is guarenteed that the allocator can deallocate its own memory
	// if this is not empty, reuse the nodes
	base_node* curr = pre_head->next;	
	base_node* other_curr = other.pre_head->next;
	while (curr != this->aft_tail && other_curr != other.aft_tail) {
		static_cast<node*>(curr)->val = static_cast<node*>(other_curr)->val;
		curr = curr->next;
		other_curr = other_curr->next;
	}
	// if the other list is longer, allocate new nodes
	if (other_curr != other.aft_tail) {
		base_node* temp_head;
		base_node* new_curr;
		try {
			temp_head = create_node(static_cast<node*>(other_curr)->val);
			new_curr = temp_head;
			other_curr = other_curr->next;
			while (other_curr != other.aft_tail) {
				new_curr->next = create_node(new_curr,static_cast<node*>(other_curr)->val);
				new_curr = new_curr->next;
				other_curr = other_curr->next;
			}
		} catch (...) {
			connect_nodes(curr->prev, temp_head);
			connect_nodes(new_curr, this->aft_tail);
			throw;
		}
		connect_nodes(curr->prev, temp_head);
		connect_nodes(new_curr,this->aft_tail);
	}
	// if this is longer we need to delete nodes;
	if (curr != this->aft_tail) {
		auto end_node = curr->prev;
		while (curr != this->aft_tail) {
			auto del_node = curr;	
			curr = curr->next;
			delete_node(del_node);
		}
		connect_nodes(end_node,aft_tail);
	} 
	return *this;
}
template <typename T, typename Allocator>
typename list<T,Allocator>::list<T,Allocator> &
list<T,Allocator>::operator=(list<T,Allocator> && other) {
	if (std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value) {
		this->clear();
	} else if (this->value_allocator != other.value_allocator) {
		// QUESTION: is this correct? am I allowed to reuse nodes here?
		// each element must be move-constructed
		// also, try to refactor this so your code is less WET
		base_node* curr = pre_head->next;	
		base_node* other_curr = other.pre_head->next;
		while (curr != this->aft_tail && other_curr != other.aft_tail) {
			static_cast<node*>(curr)->val = std::move(static_cast<node*>(other_curr)->val);
			curr = curr->next;
			other_curr = other_curr->next;
		}
		// if the other list is longer, allocate new nodes
		if (other_curr != other.aft_tail) {
			base_node* temp_head;
			base_node* new_curr;
			try {
				temp_head = create_node(std::move(static_cast<node*>(other_curr)->val));
				new_curr = temp_head;
				other_curr = other_curr->next;
				while (other_curr != other.aft_tail) {
					new_curr->next = create_node(new_curr,static_cast<node*>(other_curr)->val);
					new_curr = new_curr->next;
					other_curr = other_curr->next;
				}
			} catch (...) {
				connect_nodes(curr->prev, temp_head);
				connect_nodes(new_curr,this->aft_tail);
				throw;
			}
			connect_nodes(curr->prev, temp_head);
			connect_nodes(new_curr,this->aft_tail);
		}
		// if this is longer we need to delete nodes;
		if (curr != this->aft_tail) {
			auto end_node = curr->prev;
			while (curr != this->aft_tail) {
				auto del_node = curr;	
				curr = curr->next;
				delete_node(del_node);
			}
			connect_nodes(end_node,aft_tail);
		} 
		return *this;
	}
	this->value_allocator = other.value_allocator;
	connect_nodes(this->pre_head,other.pre_head->next);
	connect_nodes(other.aft_tail->prev, this->aft_tail);
	connect_nodes(other.pre_head,other.aft_tail);
	return *this;
}
template <typename T, typename Allocator>
typename list<T,Allocator>::list<T,Allocator> &
list<T,Allocator>::operator=(std::initializer_list<T> il) {
	assign(il.begin(), il.end());
	return *this;
}

// algorithms
template <typename T, typename Allocator>
void list<T,Allocator>::reverse() noexcept {
	std::cout << "rev\n";
	if (size() < 2) return;
	base_node* a=pre_head->next,*b,*c;
	base_node* new_end = a;
	b = a->next;
	c = b->next;

	while (c != aft_tail) {
		std::cout << static_cast<node*>(a)->val << ' ';
		connect_nodes(b,a);
		a = b;
		b = c;
		c = c->next;
	}
	std::cout << static_cast<node*>(b)->val << '\n';
	connect_nodes(b,a);
	connect_nodes(new_end,aft_tail);
	connect_nodes(pre_head,b);
}
template <typename T, typename Allocator>
list<T,Allocator>::~list() {
	clear();
}
template <typename T, typename Allocator>
size_t list<T,Allocator>::remove(T const& val) {
	return __remove_if([&val](T const& a){return a == val;});
}
template <typename T, typename Allocator>
template <typename Pred>  
requires std::predicate<Pred,T>
size_t list<T,Allocator>::remove_if(Pred pred) {
	return __remove_if(pred);
}
template <typename T, typename Allocator>
void list<T,Allocator>::swap(list& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value) {
	if (std::allocator_traits<Allocator>::propagate_on_container_swap::value) {
		// is this an unqualified call to swap?
		std::swap(this->value_allocator,other.value_allocator);
	}
	auto temp_first = this->pre_head;
	auto temp_last = this->aft_tail;
	this->pre_head = other.pre_head;
	this->aft_tail = other.aft_tail;
	other.pre_head = temp_first;
	other.aft_tail = temp_last;
	auto temp_size = this->n;
	this->n = other.n;
	other.n = temp_size;
}
template <typename T, typename Allocator>
void list<T,Allocator>::merge(list & other) {
	__merge(static_cast<node*>(other.pre_head->next), other.aft_tail, [](T const& a, T const& b){return a < b;});
}

template <typename T, typename Allocator>
void list<T,Allocator>::merge(list && other) {
	__merge(static_cast<node*>(other.pre_head->next), other.aft_tail, [](T const& a, T const& b){return a < b;});
}
template <typename T, typename Allocator>
template <typename Cmp>
requires std::predicate<Cmp,T,T>
void list<T,Allocator>::merge(list& other,Cmp cmp) {
	__merge(static_cast<node*>(other.pre_head->next), other.aft_tail,cmp);
}

template <typename T, typename Allocator>
template <typename Cmp>
requires std::predicate<Cmp,T,T>
void list<T,Allocator>::merge(list&& other,Cmp cmp) {
	__merge(static_cast<node*>(other.pre_head->next), other.aft_tail,cmp);
}
template <typename T, typename Allocator>
void list<T,Allocator>::splice(const_iterator pos, list& other) {
	__splice(pos,std::forward<list>(other),other.begin(),other.end());
}

template <typename T, typename Allocator>
void list<T,Allocator>::splice(const_iterator pos, list&& other) {
	__splice(pos,std::forward<list>(other),other.begin(),other.end());
}

template <typename T, typename Allocator>
void list<T,Allocator>::splice(const_iterator pos, list& other, const_iterator it) {
	auto it2 = it;
	++it2;
	__splice(pos,std::forward<list>(other),it,it2);
}

template <typename T, typename Allocator>
void list<T,Allocator>::splice(const_iterator pos, list&& other, const_iterator it) {
	auto it2 = it;
	++it2;
	__splice(pos,std::forward<list>(other),it,it2);
}


template <typename T, typename Allocator>
void list<T,Allocator>::splice(const_iterator pos, list& other, const_iterator first, const_iterator last) {
	__splice(pos,std::forward<list>(other),first,last);
}

template <typename T, typename Allocator>
void list<T,Allocator>::splice(const_iterator pos, list&& other, const_iterator first, const_iterator last) {
	__splice(pos,std::forward<list>(other),first,last);
}
template <typename T, typename Allocator>
size_t list<T,Allocator>::unique() {
	return __unique([](T const& a, T const& b){return a == b;});
}

template <typename T, typename Allocator>
template <typename Eq>
requires std::predicate<Eq,T,T>
size_t list<T,Allocator>::unique(Eq eq) {
	return __unique(eq);
}
template <typename T, typename Allocator>
void list<T, Allocator>::sort() noexcept {
	__sort([](T const& a, T const& b){return a < b;});
}
template <typename T, typename Allocator>
template <typename Cmp>
requires std::predicate<Cmp, T, T>
void list<T,Allocator>::sort(Cmp cmp) {
	__sort(cmp);
}
}// END OF NAMESPACE BRIAN
