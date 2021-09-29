#include "set.h"
#include <utility>
namespace brian {
template<typename Key, typename Comp, typename Allocator>
set<Key,Comp,Allocator>::set() {
	root = nullptr;	
	n = 0;
	NIL = BTraits::allocate(base_node_allocator, 1); 
	BTraits::construct(base_node_allocator, NIL);
	NIL->color = Color::BLACK;
}
template<typename Key, typename Comp, typename Allocator>
std::pair<typename set<Key,Comp,Allocator>::iterator,bool> set<Key,Comp,Allocator>::insert(Key const& val) {
	return __insert(std::forward<Key>(val));
}
template <typename Key, typename Comp, typename Allocator>
std::pair<typename set<Key,Comp,Allocator>::iterator, bool>
set<Key,Comp,Allocator>::insert(Key && val) {
	return __insert(std::forward<Key>(val));
}
template<typename Key, typename Comp, typename Allocator>
set<Key,Comp,Allocator>::~set() {
	std::stack<base_node_pointer> s;
	base_node_pointer curr = root;
	while (curr != NIL || !s.empty()) {
		while (curr != NIL) {
			s.push(curr);
			curr = curr->left;
		}
		curr = s.top();
		s.pop();
		node_pointer del_node = static_cast<node_pointer>(curr);
		curr = static_cast<node_pointer>(curr->right);
		delete_node(del_node);
	}
	BTraits::destroy(base_node_allocator, NIL);
	BTraits::deallocate(base_node_allocator, NIL, 1);
}

template<typename Key, typename Comp, typename Allocator>
typename set<Key,Comp,Allocator>::iterator 
set<Key,Comp,Allocator>::insert(const_iterator hint, Key const& val) {
	// first try to insert directly before the hint
	/* I have no idea how to do this */			
	// if insertition before hint fails, do regular insert
	return __insert(std::forward<Key>(val)).first;
}
template<typename Key, typename Comp, typename Allocator>
bool set<Key,Comp,Allocator>::empty() const noexcept {
	return root == nullptr;
}
template<typename Key, typename Comp, typename Allocator>
size_t set<Key,Comp,Allocator>::size() const noexcept {
	return n;
}
template<typename Key, typename Comp, typename Allocator>
size_t set<Key,Comp,Allocator>::max_size() const noexcept {
	return Traits::max_size(node_allocator);
}
}
