#include "set.h"
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
	if (root == nullptr) {
		root = create_node(val);
		n = 1;
		return {iterator(root), true};
	}
	base_node_pointer curr = root, prev;
	while (curr != NIL) {
		prev = curr;
		if (val < static_cast<node_pointer>(curr)->val) { curr = curr->left; }
		else if (val > static_cast<node_pointer>(curr)->val) { curr = curr->right; }
		else { return {iterator(curr), false };}
	}
	base_node_pointer new_node = create_node(val);
	if (val < static_cast<node_pointer>(prev)->val) { prev->left = new_node; }
	else { prev->right = new_node; }
	// fix_insert(new_node);
	return {iterator(new_node), true};

}
}
