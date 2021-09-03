#include "set.h"
namespace brian { 
template<typename Key, typename Compare, typename Allocator>
set<Key,Compare,Allocator>::set() {
	root = nullptr;
	n = 0;
}

template<typename Key, typename Compare, typename Allocator>
std::pair<typename set<Key,Compare,Allocator>::iterator, bool> set<Key,Compare,Allocator>::insert(value_type const& val) {
	if (root == nullptr) {
		root = create_node(NIL,val);
		root->left = NIL;
		root->right = NIL;
		root->color = Color::BLACK;	
		++n;
		return std::pair<iterator, bool>(to_node(root), true);
	} else {
		base_node_pointer temp = root;
		base_node_pointer curr = root;
		while (curr != NIL) {
			temp = curr;
			if (val < static_cast<node_pointer>(curr)->val) {
				curr = curr->left;
			}
			else if (val > to_node(curr)->val) {
				curr = curr->right;
			}
			else {
				return std::pair<iterator,bool>(to_node(curr), false);
			}
		}
		base_node_pointer new_node =create_node_with_hint(temp,temp,val);
		setup_new_node(new_node);
		++n;
		if (val < to_node(temp)->val) {
			temp->left = new_node;
		} else {
			temp->right = new_node;
		}
		return std::pair<iterator, bool>(to_node(new_node), true);
	}
}
}
