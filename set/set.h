#pragma once
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <ostream>
#include <type_traits>
#include <utility>
#include <stack>
#include <iostream>// for debugging
namespace brian {
template <typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key> >
class set {
	enum class Color {RED,BLACK};
	template <bool Was_Const>
	class set_iterator;
	// forward declaration of node
	class node;	
	class base_node;
	friend base_node;
public:
	// DELETE ME: for debugging only
	void dump() const {
		std::stack<base_node_pointer> s;
		base_node_pointer curr = root;
		std::cout << "DUMP" << '\n';
		while (curr != NIL|| !s.empty()) {
			while (curr != NIL) {
				s.push(curr);
				curr = curr->left;
			}
			curr = s.top();
			s.pop();
			std::cout << *static_cast<node_pointer>(curr) << '\n';
			curr = curr->right;
		
		}
	}
	void print_path_to(Key const& val) {
		auto curr = root;
		std::cout << "PATH TO " << val << '\n';
		while (curr != NIL) {
			std::cout << *static_cast<node_pointer>(curr) << '\n';
			if (val < to_node(curr)->val) {
				curr = curr->left;
			} else if (val > to_node(curr)->val) {
				curr = curr->right;
			} else {
				return;
			}

		}
	}
	using key_type = Key;
	using value_type = Key;	
	using size_type = size_t;
	using difference_type = std::ptrdiff_t;
	using key_compare = Compare;
	using value_compare = Compare;
	using allocator_type = Allocator;
	using reference = value_type&;
	using const_reference = value_type const&;
	using pointer = typename std::allocator_traits<allocator_type>::pointer;
	using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
	using iterator = set_iterator<false>;
	using const_iterator = set_iterator<true>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	using node_type = node;
	template <typename Iter, typename Node>
	struct insert_return_type {
		Iter position;
		bool inserted;
		Node node;
		insert_return_type<Iter, Node>() {}
		insert_return_type<Iter,Node>(Iter it, Node n) : position(it), node(n) {}
	};
private:
	using node_alloc_t = typename std::allocator_traits<allocator_type>:: template rebind_alloc<node>;
	using base_node_alloc_t = typename std::allocator_traits<allocator_type>:: template rebind_alloc<base_node>;
	using BTraits = std::allocator_traits<base_node_alloc_t>;
	using Traits = std::allocator_traits<node_alloc_t>;
	// hopefully I wont have to use this...

	node_alloc_t node_allocator;
	using node_pointer = typename Traits::pointer;
	using base_node_pointer = typename BTraits::pointer;
	// try making this const
	// this is the base_node instance that all base_node_pointer's point to
	base_node NIL_OBJ;
	base_node_pointer NIL = std::pointer_traits<base_node_pointer>::pointer_to(NIL_OBJ);

	template <bool Is_Const>
	class set_iterator {
	public:	
		using value_type = set::value_type;
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = set::difference_type;
		using pointer = std::conditional_t<Is_Const, set::const_pointer, set::pointer>;
		using reference = std::conditional<Is_Const, set::const_reference, set::reference>;
		set_iterator() : itr_curr(nullptr) {}
		set_iterator(node_pointer c) : itr_curr(c) {}
		set_iterator(set_iterator const&) = default;
		template <bool Was_Const, typename = std::enable_if_t<Is_Const || !Was_Const>>
		set_iterator(set_iterator<Was_Const> const& it) : itr_curr(it.itr_curr) {}
			


	private:
		node_pointer itr_curr;
	};
	// need to make sure that base_node_pointer is convertable to node_pointer
	// reguardless of what node_pointer type is
	struct base_node {
		base_node_pointer left, right, parent;
		Color color;
		base_node() = default;
		base_node(base_node_pointer p) :left(nullptr), right(nullptr), parent(p), color(Color::BLACK){}
	};

	struct node : base_node {
		Key val;
		// val
		template <typename U,typename ...Args, typename = std::enable_if_t<!std::is_convertible<U,node_pointer>::value>>
		node(Args &&...args) : val(std::forward<Args>(args)...) {}
		// parent, val
		template <typename ...Args>
		node( base_node_pointer p, Args &&...args) : base_node(p), val(std::forward<Args>(args)...) {}
		// for debugging 
		friend std::ostream& operator<<(std::ostream& os, node const& n) {
			os << n.val << ", " << ((n.color == Color::RED) ? "red" : "black");
			return os;
		}
		Key value() const noexcept {
			return val;
		}
	};
	base_node_pointer root = nullptr;
	size_t n;
public:	
	/*constructors*/
	set();
	/*mutators*/
	std::pair<iterator, bool> insert(value_type const& val);

private:
	/*helpers*/
	node_pointer to_node(base_node_pointer n) { return static_cast<node_pointer>(n); }

	template <typename ...Args>
	node_pointer create_node(Args &&...args) {
		node_pointer new_node = Traits::allocate(node_allocator, 1);
		Traits::construct(node_allocator, new_node, std::forward<Args>(args)...);	
		return new_node;
	}
	template <typename ...Args>
	node_pointer create_node_with_hint(base_node_pointer hint,Args &&...args) {
		node_pointer new_node = Traits::allocate(node_allocator, 1, to_node(hint));
		Traits::construct(node_allocator, new_node, std::forward<Args>(args)...);	
		return new_node;
	}
	void setup_new_node(base_node_pointer n) {
		n->left = NIL;
		n->right = NIL;
		n->color = Color::RED;
	}
	// helpers with balancing tree
	void left_rotate(base_node_pointer parent) {
		// left_child of parent is not null
		// grand_parent points to left_child with whichever link it used to point to parent
		// right_child's left child becomes parent's new right
		// parent becomes right_child's new left child.
		base_node_pointer right_child = parent->right;
		parent->right = right_child->left;
		if (right_child->left != NIL) { right_child->left->parent = parent; }
		right_child->parent = parent->parent;
		if (parent->parent != NIL) { root = right_child; }
		else if (parent->parent->left == parent) { parent->parent->left = right_child;  }
		else { parent->parent->right = right_child; }
		right_child->left = parent;
		parent->parent = right_child;
	}
	void right_rotate(base_node_pointer parent) {
		base_node_pointer left_child = parent->left;
		parent->left = left_child->right;
		if (left_child->right != NIL) { left_child->right->parent = parent; }
		left_child->parent = parent->parent;
		if (parent->parent != NIL) { root = left_child; }
		else if (parent->parent->right == parent) { parent->parent->right = left_child;  }
		else { parent->parent->left = left_child; }
		left_child->right = parent;
		parent->parent = left_child;
	}

}; // END CLASS SET
}// END NAMESPACE BRIAN
#include "set.hpp"
