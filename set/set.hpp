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
		root->parent = NIL;
		root->color = Color::BLACK;
		n = 1;
		return {iterator(root), true};
	}
	base_node_pointer curr = root, prev;
	while (curr != NIL) {
		prev = curr;
		if (comp(val,static_cast<node_pointer>(curr)->val)) { curr = curr->left; }
		else if (comp(static_cast<node_pointer>(curr)->val,val)) { curr = curr->right; }
		else { return {iterator(curr), false };}
	}
	base_node_pointer new_node = create_node(val);
	new_node->parent = prev;
	if (comp(val,static_cast<node_pointer>(prev)->val)) { prev->left = new_node; }
	else { prev->right = new_node; }
	fix_insert(new_node);
	this->root->color = Color::BLACK;
	++n;
	return {iterator(new_node), true};

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
