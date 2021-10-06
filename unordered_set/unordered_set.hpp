#include <iterator>
#include <memory>
#include <type_traits>

template <typename Key, typename Hash=std::hash<Key>, typename KeyEqual = std::equal_to<Key>, typename Allocator = std::allocator<Key>>
class unordered_set {

};

