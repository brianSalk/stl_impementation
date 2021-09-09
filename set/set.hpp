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

}
