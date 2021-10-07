#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>

template <typename Key, typename Hash=std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<Key>>
class unordered_set {
	class set_iterator;
	class local_set_iterator;
	struct base_node {
		base_node* next;
	};
	struct node {

	};
public:
	using key_type = Key;
	using value_type = Key;
	using size_type = size_t;
	using difference_type = std::ptrdiff_t;
	using hasher = Hash;
	using key_equal = KeyEqual;
	using allocator_type = Allocator;
	using reference = Key&;
	using const_reference = Key const&;
	using pointer = Key*;
	using const_pointer = Key const*;
	using iterator = set_iterator;
	using const_iterator = set_iterator;
	using local_iterator = local_set_iterator;
	using const_local_iterator = local_set_iterator;
// node_type needs to have the following methods:  empty(), get_allocator(), swap(node_type other_node),  operator bool, operator =, value();
	class node_type {
	public:
		node_type()
	};
	// insert return type contains three fields named:  (inserted, node, position)
	using insert_return_type = ???;

};

