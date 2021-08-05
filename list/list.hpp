#pragma once
#include "list.h"
namespace brian { 
// constuctors
// default constructor
template <typename T, typename Allocator>
list<T,Allocator>::list() : pre_head(new base_node()),aft_tail(new base_node()) {}
// modifiers
}// END OF NAMESPACE BRIAN
