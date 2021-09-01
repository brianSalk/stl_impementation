#include "set.h"
namespace brian { 
template<typename Key, typename Compare, typename Allocator>
set<Key,Compare,Allocator>::set() {
	root = nullptr;
	n = 0;
}

}
