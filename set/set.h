#pragma once
#include <cstddef>
#include <iostream> // for debugging
#include <functional> // for std::less
#include <iterator>
#include <memory>
#include <stack> // for debugging only
#include <type_traits>
#include <utility>
namespace brian {
template <typename Key, typename Comp = std::less<Key>, typename Allocator = std::allocator<Key>>
class set {
private:
	enum struct Color { RED, BLACK };
	template <bool Is_Const>
	class set_iterator;
	struct node;
	struct base_node;
	template <typename Iter, typename Node>
	struct insert_return_type_template;
public:
	using key_type = Key;
	using value_type = Key;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using key_compare = Comp;
	using value_compare = Comp;
	using allocator_type = Allocator;
	using reference = Key&;
	using const_reference = Key const&;
	using pointer = typename std::allocator_traits<allocator_type>::pointer;
	using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
	using iterator = set_iterator<false>;
	using const_iterator = set_iterator<true>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using node_type = node;
	using insert_return_type = insert_return_type_template<iterator, node_type>;
private:
	Comp comp;
	allocator_type value_allocator;
	// private aliases, for sanity purposes
	using node_alloc_t = typename std::allocator_traits<Allocator>:: template rebind_alloc<node>;
	using base_node_alloc_t = typename std::allocator_traits<Allocator>:: template rebind_alloc<base_node>;
	node_alloc_t node_allocator;
	base_node_alloc_t base_node_allocator;
	using Traits = std::allocator_traits<node_alloc_t>;
	using BTraits = std::allocator_traits<base_node_alloc_t>;	
	using node_pointer = typename Traits::pointer;
	using base_node_pointer = typename BTraits::pointer;
	size_t n;
	base_node_pointer root;
	base_node_pointer NIL;
	struct base_node {
		base_node_pointer left, right, parent;
		Color color;
		base_node() = default;
		base_node(base_node_pointer nil) : left(nil), right(nil), parent(nullptr) {}
	};
	struct node : base_node {
		Key val;
		template <typename ...Args>
		node(Args ...args) : val(std::forward<Args>(args)...) {}
	};
public:
	/*constructors*/
	set();
	/*modifiers*/
	std::pair<iterator,bool> insert(Key const& val);
	std::pair<iterator,bool> insert(Key && val);
	~set();
	/*observers*/
	/*debuggers*/
	void PATH_TO(Key const& val) {
		base_node_pointer curr = root;
		while (curr != NIL) {
			std::cout << static_cast<node_pointer>(curr)->val << ' ';
			if (comp(val,static_cast<node_pointer>(curr)->val)) { curr = curr->left; }
			else { curr = curr->left; }
		}	
	}
	void ROOT_DUMP() { std::cout << static_cast<node_pointer>(root)->val; }
	void DUMP() {
		std::stack<base_node_pointer> s;
		base_node_pointer curr = root;
		while (!s.empty() || curr != NIL) {
			while (curr != NIL) {
				s.push(curr);
				curr = curr->left;
			}
			curr = s.top();
			s.pop();
			std::cout << static_cast<node_pointer>(curr)->val << " ";
			curr = curr->right;
		}
	}
private:
	/*iterators*/
	template <bool Is_Const>
	class set_iterator {
	public:
		using value_type = set::value_type;
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = set::difference_type;
		using reference = std::conditional_t<Is_Const,set::const_reference, set::reference>;
		using pointer = std::conditional_t<Is_Const, set::const_pointer, set::pointer>;
		set_iterator(set_iterator const&) = default;
		template <bool Was_Const, typename = std::enable_if_t<!Was_Const && Is_Const,int*>>
		set_iterator(set_iterator<Was_Const> const& i) : itr_curr(i) {}
		set_iterator(base_node_pointer p) : itr_curr(p) {}

	private:
		base_node_pointer itr_curr;
	};
	/*helpers*/
	template <typename ...Args>
	node_pointer create_node(Args &&...args) {
		node_pointer new_node = Traits::allocate(node_allocator, 1);
		Traits::construct(node_allocator, new_node, std::forward<Args>(args)...);
		new_node->color = Color::RED;
		new_node->left = NIL;
		new_node->right = NIL;
		return new_node;
	}
	base_node_pointer create_base_node() {
		base_node_pointer new_base_node = BTraits::allocate(base_node_allocator,1);
		BTraits::construct(base_node_allocator, new_base_node);
		new_base_node->color = Color::BLACK;
		return new_base_node;
	}
	void delete_node(node_pointer curr) {
		Traits::destroy(node_allocator, curr);
		Traits::deallocate(node_allocator, curr, 1);
	}
	void rotate_right(base_node_pointer curr) {
		// make curr right child of curr's left child
		base_node_pointer left_child = curr->left;
		curr->left = left_child->right;
		if (left_child->right != NIL) {
			left_child->right->parent = curr;
		}
		left_child->parent = curr->parent;
		if (curr->parent == NIL) {
			root = left_child;
		}
		else if (curr == curr->parent->right) {
			curr->parent->right = left_child;
		}
		else {
			curr->parent->left = left_child;
		}
		left_child->right = curr;
		curr->parent = left_child;
	}
	void rotate_left(base_node_pointer curr) {
		// make curr left child of curr's right child
		base_node_pointer right_child = curr->right;
		curr->right = right_child->left;
		if (right_child->left != NIL) {
			right_child->left->parent = curr;
		}
		right_child->parent = curr->parent;
		if (curr->parent == NIL) {
			root = right_child;
		}
		else if (curr == curr->parent->left) {
			curr->parent->left = right_child;
		}
		else {
			curr->parent->right = right_child;
		}
		right_child->left = curr;
		curr->parent = right_child;
	}
	std::pair<iterator, bool> __insert(Key && val) {
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
		base_node_pointer new_node = create_node(std::forward<Key>(val));
		new_node->parent = prev;
		if (comp(val,static_cast<node_pointer>(prev)->val)) { prev->left = new_node; }
		else { prev->right = new_node; }
		fix_insert(new_node);
		this->root->color = Color::BLACK;
		++n;
		return {iterator(new_node), true};
	}
	void fix_insert(base_node_pointer curr) {
		while (curr->parent->color == Color::RED) {
			if (curr->parent == curr->parent->parent->left) {
				base_node_pointer uncle = curr->parent->parent->right;

				if (uncle->color == Color::RED) {
					curr->parent->color = Color::BLACK;
					uncle->color = Color::BLACK;
					curr->parent->parent->color = Color::RED;
					curr = curr->parent->parent;
				}
				else {
					if (curr == curr->parent->right) {
						curr = curr->parent;
						rotate_left(curr);
					}
					curr->parent->color = Color::BLACK;
					curr->parent->parent->color = Color::RED;
					rotate_right(curr->parent->parent);
				}
			}
			else {
				base_node_pointer uncle = curr->parent->parent->left;

				if (uncle->color == Color::RED) {
					curr->parent->color = Color::BLACK;
					uncle->color = Color::BLACK;
					curr->parent->parent->color = Color::RED;
					curr = curr->parent->parent;
				}
				else {
					if (curr == curr->parent->left) {
						curr = curr->parent;
						rotate_right(curr);
					}
					curr->parent->color = Color::BLACK;
					curr->parent->parent->color = Color::RED;
					rotate_left(curr->parent->parent);
				}
			}
		}
	}

}; // END CLASS SET

} // END NAMESPACE BRIAN

#include "set.hpp"
