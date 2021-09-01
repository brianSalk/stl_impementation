#pragma once
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <ostream>
namespace brian {
template <typename Key, typename Compare = std::less<Key>, typename Allocator = std::allocator<Key> >
class set {
private:
	struct base_node {
		base_node* left;
		base_node* right;
		base_node* parent;
		enum color {BLACK,RED};
		base_node() : left(nullptr), right(nullptr), parent(nullptr) {}
		base_node(base_node* p) : left(nullptr), right(nullptr), parent(p) {}

	};
	class node : base_node {
		Key data;
		// val
		template <typename ...Args>
		node(Args &&...args) : base_node(), data(std::forward<Args>(args)...) {}
		// val, parent
		node(Key d, node* p) : data(d), 
			base_node::left(nullptr), 
			base_node::right(nullptr), 
			base_node::parent(p) {}
		// for debugging 
	public:
		friend std::ostream& operator<<(std::ostream& os, node const& n) {
			os << n.data << ", " << n.color;
			return os;
		}
		Key value() const noexcept {
			return data;
		}
	};
	node pre_root;
public:
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
	using iterator = typename set_iterator<false>;
	using const_iterator = typename set_iterator<true>;
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
	


}; // END CLASS SET
}// END NAMESPACE BRIAN
