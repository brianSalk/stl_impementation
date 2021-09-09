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
	node_pointer root;
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
	/*observers*/
	/*debuggers*/
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

}; // END CLASS SET

} // END NAMESPACE BRIAN

#include "set.hpp"
