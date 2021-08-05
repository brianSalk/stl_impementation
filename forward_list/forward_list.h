#pragma once
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <sys/types.h> 
#include <type_traits>
#include <utility>
#include <compare>
#include <algorithm>
namespace brian {
template <typename T, typename Allocator = std::allocator<T>>
class forward_list {
	// forward declaration of list_iterator class template
	template <bool IsConst> 
	class list_iterator;
public:
	// aliases 
	using value_type = T;
	using allocator_type = Allocator;
	using size_type = size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = value_type const&; 
	using pointer = typename std::allocator_traits<Allocator>::pointer;
	using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
	using iterator = list_iterator<false>;
	using const_iterator = list_iterator<true>;
private:

	allocator_type value_allocator;
	// base_node needs to be constructed because the
	// list must have one node before the head of the list
	// since we cannot assume that the template type T
	// has a default constructor, we cannot instantiate the
	// type using the default constructor, therefor we need a
	// special node called base_node for the first node of the list
	struct base_node {
		base_node* next;
		base_node() : next(nullptr){}
		base_node(base_node* n) : next(n) {}
	};
	struct derived_node : base_node {
		T val;
		derived_node() : base_node() {} 
		template <typename ...Args>
		derived_node(Args && ...args) : val(std::forward<Args>(args)...) {}
	};
	using NodeAlloc_t = typename std::allocator_traits<Allocator>::template rebind_alloc<derived_node>;
	NodeAlloc_t node_allocator;
	using Traits = typename std::allocator_traits<NodeAlloc_t>;
	base_node* pre_head;
public:
	// DELETE ME: this method is for debugging only
	void dump() const { 
		std::cout << "DUMP" << std::endl;
		derived_node* curr = static_cast<derived_node*>(pre_head);
		curr = static_cast<derived_node*>(curr->next);
		while (curr != nullptr) {
			std::cout << curr->val << '\n';
			curr = static_cast<derived_node*>(curr->next);
		}
	}
	// constructors
	forward_list();
	explicit forward_list(Allocator const& alloc);
	// T must be DefaultInsertable in order to participate in this overload
	explicit forward_list(size_type count, Allocator const& alloc = Allocator());
	// t must be 
	explicit forward_list(size_type count, T const& value, Allocator const& alloc = Allocator());
	template <typename It, typename std::iterator_traits<It>::pointer = nullptr>
	forward_list(It first, It last, Allocator const& alloc = Allocator());
	forward_list(forward_list const& other);
	forward_list(forward_list const& other, Allocator const& alloc);
	forward_list(forward_list&& other);
	forward_list(forward_list&& other, Allocator const& alloc);
	forward_list(std::initializer_list<T> const& il, Allocator const& alloc = Allocator());

	// mutators
	void push_front(T const& val);
	void push_front(T&& val);
	void pop_front();
	void resize(size_type count);
	void resize(size_type count,value_type const& val);
	void swap(forward_list& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value);
	void merge(forward_list& other);
	void merge(forward_list&& other);
	template <typename Cmp>
	requires std::predicate<Cmp,T,T>
	void merge(forward_list& other, Cmp comp);
	template <typename Cmp>
	requires std::predicate<Cmp,T,T>
	void merge(forward_list&& other, Cmp comp);
	void splice_after(const_iterator pos, forward_list& other);
	void splice_after(const_iterator pos, forward_list&& other); 
	void splice_after(const_iterator pos, forward_list& other, const_iterator it);
	void splice_after(const_iterator pos, forward_list&& other, const_iterator it);
	void splice_after(const_iterator pos, forward_list& other, const_iterator first, const_iterator last);
	void splice_after(const_iterator pos, forward_list&& other, const_iterator first, const_iterator last);
	// algorithms
	size_type remove(T const& val);
	template <typename Pred>
	requires std::predicate<Pred,T>
	size_type remove_if(Pred p);
	void reverse() noexcept;
	size_type unique();
	template<typename Pred>
	requires std::predicate<Pred,T,T>
	size_type unique(Pred p);
	void sort();
	template <typename Cmp>
	requires std::predicate<Cmp,T,T>
	void sort(Cmp comp);
	
	template<typename ...Args>
	void emplace_front(Args && ...arg);
	iterator insert_after(const_iterator pos, T const& val);
	iterator insert_after(const_iterator pos, T&& value);
	iterator insert_after(const_iterator pos, size_type count, T const& val);
	// FIX ME: use concepts for It to make sure its an iterator
	template <typename It, typename std::iterator_traits<It>::pointer=nullptr>
	iterator insert_after(const_iterator pos, It first, It last);
	iterator insert_after(const_iterator pos, std::initializer_list<T> il);
	template <typename ...Args>
	iterator emplace_after(const_iterator pos, Args&& ...args);
	iterator erase_after(const_iterator pos);
	iterator erase_after(const_iterator first, const_iterator last);
	forward_list& operator=(forward_list const& other);
	forward_list& operator=(forward_list && other) noexcept(std::allocator_traits<allocator_type>::is_always_equal::value);
	forward_list& operator=(std::initializer_list<T> il);
	void assign(size_type count, T const& value);	
	// FIX ME: use concepts to ensure It is input-iterator
	template<typename It, typename std::iterator_traits<It>::pointer=nullptr>
	void assign(It first, It last);
	void assign(std::initializer_list<T> il);
	void clear() noexcept;
	~forward_list();

	// observers
	const_iterator begin() const { return const_iterator(pre_head->next);}
	iterator begin() { return iterator(pre_head->next); }
	const_iterator end() const { return const_iterator(nullptr); }
	iterator end()  { return iterator(nullptr); }
	const_iterator cend() const { return const_iterator(nullptr); }
	const_iterator cbegin() const { return const_iterator(pre_head->next); }
	iterator before_begin() { return iterator(pre_head); }
	const_iterator before_begin() const { return const_iterator(pre_head); }
	const_iterator cbefore_begin() const { return const_iterator(pre_head); }
	allocator_type get_allocator() const noexcept { return value_allocator; }
	reference front();
	const_reference front() const;
	// constructs in c++ surrounded in [[ ... ]] are known as attributes
	// the nodiscard attribute creates a compiler warning if the value of a
	// method is not evaluated or assigned
	[[nodiscard]] bool empty() const noexcept { return pre_head->next == nullptr; }
	size_type max_size() const noexcept;
private:
	template<bool IsConst>
	class list_iterator {
		public:
		friend forward_list<T>;
			using value_type = T;
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using pointer = typename std::conditional<IsConst, T const*, T*>::type;
			using reference = typename std::conditional<IsConst, T const&, T&>::type;
			// default copy construtor 
			list_iterator(list_iterator const&) = default;
			// converting constructor that converts between const and non-cost iterators.
			// hat tip to https://quuxplusone.github.io/blog/2018/12/01/const-iterator-antipatterns/https://quuxplusone.github.io/blog/2018/12/01/const-iterator-antipatterns/
			template<bool WasConst, typename = std::enable_if_t<IsConst || !WasConst>>
				list_iterator(list_iterator<WasConst> const& that) : itr_curr(that.itr_curr) {}
			list_iterator(base_node* c) :itr_curr(c) {}	
			// prefix incrament
			list_iterator<IsConst>& operator++() {
				itr_curr = itr_curr->next;
				return *this;
			}
			// postfix incrament
			list_iterator<IsConst> operator++(int) {
				auto cp = itr_curr;
				itr_curr = itr_curr->next;
				return list_iterator(cp);
			}
			// derference operator
			reference operator*() {
				return static_cast<derived_node*>(itr_curr)->val; 
			}
			bool operator!=(list_iterator that) const {
				return this->itr_curr != that.itr_curr;
			}
			bool operator==(list_iterator that) const {
				return this->itr_curr == that.itr_curr;
			}
			
		private:
			base_node* itr_curr;

	};
	// helper methods
	derived_node* create_default_node() {
		derived_node* new_node = Traits::allocate(node_allocator,1);
		Traits::construct(node_allocator, new_node);
		return new_node;
	}
	template <typename ...Args>
	derived_node* create_node(Args && ...args) {
		derived_node* new_node = Traits::allocate(node_allocator,1);
		try {
		Traits::construct(node_allocator, new_node, std::forward<Args>(args)...);
		} catch (...) {
			delete_node(new_node);
			new_node = nullptr;
			throw;
		}
		return new_node;
	}
	void delete_node(derived_node* node) {
		Traits::destroy(node_allocator, node);
		Traits::deallocate(node_allocator, node, 1);
	}
	void delete_base_node(base_node* node) {
		Traits::destroy(node_allocator, node);
		Traits::deallocate(node_allocator, static_cast<derived_node*>(node), 1);
	}
	template <typename U>
	void __splice_after(const_iterator pos,U  && other) {
		std::cout << "called\n";
		base_node* rest_of_this = pos.itr_curr->next;
		pos.itr_curr->next = other.pre_head->next;
		// find end of other
		base_node* end_of_other = other.pre_head;
		while (end_of_other->next != nullptr) {
			end_of_other = end_of_other->next;
		}
		end_of_other->next = rest_of_this;
		other.pre_head->next = nullptr;
	}
	// FIX ME: im pretty sure this can be optimized
	template <typename U>
	void __splice_after(const_iterator pos, U && other, const_iterator it) {
		if (pos == it || pos == ++it) return;
		base_node* node_to_spice_from_other = it.itr_curr->next;
		base_node* rest_of_this = pos.itr_curr->next;
		base_node* rest_of_other = it.itr_curr->next->next;
		pos.itr_curr->next = node_to_spice_from_other;
		node_to_spice_from_other->next = rest_of_this;
		it.itr_curr->next = rest_of_other;
	}
	template <typename U>
	void __splice_after(const_iterator pos, U && other, const_iterator first, const_iterator last) {
		// move elements from the range (first, last) after pos		
		base_node* node_to_spice_from_other = first.itr_curr->next;
		base_node* rest_of_this = pos.itr_curr->next;
		base_node* rest_of_other = last.itr_curr->next;
		base_node* other_curr = node_to_spice_from_other;
		base_node* this_curr = pos.itr_curr;
		while (other_curr != last.itr_curr) {
			this_curr->next = other_curr;
			this_curr = this_curr->next;
			other_curr = other_curr->next;
		}
		this_curr->next = rest_of_this;
		first.itr_curr->next = last.itr_curr;
	}
	size_t __get_length() const {
		size_t count = 0;
		base_node* curr = pre_head->next;
		while (curr != nullptr) {
			curr = curr->next;
			++count;
		}
		return count;
	}
	base_node* __split(base_node* start, size_t len, base_node*& next_sublist) {
		base_node* slow = start;
		base_node* fast = start->next;
		// find the middle node, split the list there
		// QUESTION: why are we counting the length of the list?
		// ANSWER: we want to move fast to the end of the sublist and slow to
		// one before the middle of the sublist.
		for (size_t i = 1;i < len && (fast->next || slow->next); ++i) {
			if (fast->next) {
				fast = (fast->next->next) ? fast->next->next : fast->next;
			}
			if (slow->next) {
				slow = slow->next;
			}
		}
		base_node* mid = slow->next;
		next_sublist = fast->next;
		slow->next = nullptr;
		fast->next = nullptr;
		return mid;
	}
	template <typename Cmp>
	base_node* __merge_nodes(base_node* list1, base_node* list2,base_node*& tail, Cmp const& less) {
		base_node dummy_head;
		base_node* new_tail = &dummy_head;
		while (list1 && list2) {
			if (less(static_cast<derived_node*>(list1)->val,static_cast<derived_node*>(list2)->val)) {
				new_tail->next = list1;
				list1 = list1->next;
				new_tail = new_tail->next;
			} else {
				new_tail->next = list2;
				list2 = list2->next;
				new_tail = new_tail->next;
			}
		}
		if (list1) {
			new_tail->next = list1;
		} else {
			new_tail->next = list2;
		}
		while (new_tail->next) {
			new_tail = new_tail->next;
		}
		tail->next = dummy_head.next;
		tail = new_tail;

		return dummy_head.next;
	}
	template <typename Cmp>
	void __sort(Cmp const& cmp) {
		// if length < 2, don't do anything.
		base_node* tail;
		base_node* next_sublist;
		if (pre_head->next == nullptr || pre_head->next->next == nullptr) {
			return;
		}
		size_t n = __get_length();
		base_node* start = pre_head->next;
		for (size_t size{1}; size < n; size*=2) {
			tail = pre_head;
			while (start) {
				if (start->next == nullptr) {
					tail->next = start;
					break;
				}
				base_node* mid = __split(start, size, next_sublist);
				__merge_nodes(start,mid, tail, cmp);
				start = next_sublist;
			}
			start = pre_head->next;
		}
	}
	// FIX ME:
	// make sure that this algo is still exception safe even if pred throws.
	template <typename Pred>
	size_type __remove(Pred p) {
		derived_node* curr = static_cast<derived_node*>(pre_head->next);
		derived_node* temp = static_cast<derived_node*>(pre_head);
		derived_node* del_node;
		size_t count = 0;
		base_node del_list_pre_head;
		base_node* del_list_curr = del_list_pre_head;
		while (curr != nullptr) {
			try {
				while (curr != nullptr && p(curr->val)) {
					count++;
					del_node = curr;
					curr = static_cast<derived_node*>(curr->next);
					del_list_curr->next = del_node;
					del_list_curr = del_list_curr->next;
				}
			}
			// g++ and clang both leave the container in the
			// same state as it was before the method call if an exception
			// is thrown, although only a basic exception guarentee
			// is required by the standard.
			// here I am doing the same, 
			// even though it is not a strict requirement
			catch (...) {
				// if p throws, then make sure there are no leaks and
				std::cerr << "predicate threw exception in remove_if\n";
			}
			temp->next = curr;
			temp = curr;
			if (curr != nullptr)
				curr = static_cast<derived_node*>(curr->next);
		}
		return count;
	}
	template <typename Pred>
	size_t __unique(Pred const& p) {
		size_t count = 0;
		if (!pre_head->next || !pre_head->next->next) return count;
		derived_node* curr = static_cast<derived_node*>(pre_head->next->next);
		derived_node* temp = static_cast<derived_node*>(pre_head->next);
		derived_node* del_node;
		while (curr != nullptr) {
			// much like with remove_if, if pred throws,
			// there is a basic excpetion guarentee
			// except it looks like g++ and clang
			// do not preserve the excact state of the
			// container if pred throws at any time
			try {
			while(curr && p(temp->val,curr->val)) {
				count++;
				del_node = curr;
				curr = static_cast<derived_node*>(curr->next);
				temp->next = del_node;
				delete_node(del_node);
			}
			}
			catch (...) {
				temp->next = curr;
				temp = curr;
				throw;
			}
			temp->next = curr;
			temp = curr;
			if (curr != nullptr)
				curr = static_cast<derived_node*>(curr->next);
		}
		return count;
	}
	template <typename Cmp>
	forward_list<T,Allocator>& __merge(forward_list && lhs,
		forward_list && rhs, Cmp less) {
		// reconnect nodes, moving them form rhs to lhs.
		derived_node* r_curr = static_cast<derived_node*>(rhs.begin().itr_curr);
		derived_node* l_curr = static_cast<derived_node*>(lhs.begin().itr_curr);
		derived_node* temp_head;
		if (l_curr != nullptr && r_curr != nullptr) {
			if (less(l_curr->val, r_curr->val)) {
				temp_head = l_curr;
				l_curr = static_cast<derived_node*>(l_curr->next);
			} else {
				temp_head = r_curr;
				r_curr = static_cast<derived_node*>(r_curr->next);
			}
		}
		derived_node* temp_curr = temp_head;
		while (l_curr != nullptr && r_curr != nullptr) {
			if (less(l_curr->val,r_curr->val)) {
				// add l_curr to list
				temp_curr->next = static_cast<base_node*>(l_curr);
				l_curr = static_cast<derived_node*>(l_curr->next);
			} else {
				temp_curr->next = static_cast<base_node*>(r_curr);
				r_curr = static_cast<derived_node*>(r_curr->next);
			}
			temp_curr = static_cast<derived_node*>(temp_curr->next);
		}
		while (l_curr != nullptr) {
			temp_curr->next = static_cast<base_node*>(l_curr);
			l_curr = static_cast<derived_node*>(l_curr->next);
			temp_curr = static_cast<derived_node*>(temp_curr->next);
		}
		while (r_curr != nullptr) {
			temp_curr->next = static_cast<base_node*>(r_curr);
			r_curr = static_cast<derived_node*>(r_curr->next);
			temp_curr = static_cast<derived_node*>(temp_curr->next);
		}
		lhs.before_begin().itr_curr->next = static_cast<base_node*>(temp_head);
		rhs.before_begin().itr_curr->next = nullptr;
		return *this;
	}
	template <typename ...Args>
	void __resize(size_t new_size, Args  const&...args) {
		size_t count = 0;
		// keep moving curr along until count > new_size or curr->next == nullptr
		base_node* curr = pre_head;
		while (count < new_size && curr->next) {
			curr = curr->next;
			++count;
		}
		if (!curr->next) {
			// add new_size - count elements
			try {
				while (count < new_size) {
					curr->next = create_node(args...);
					curr = curr->next;
					++count;
				}
			}
			catch (...) {
				// if resize fails,
				// the new node was never created or was destroyed
				// within create_node.
				// just point curr->next at nullptr
				// to provide a basic exception safety;
				curr->next = nullptr;
			}
		} else {
			// remove all nodes after curr
			base_node* del_node = curr->next;
			base_node* last = curr;
			base_node* temp;
			while (del_node) {
				temp = del_node;
				del_node = del_node->next;
				delete_base_node(temp);
			}
			curr->next = nullptr;
		}
	}
	// default comparison operators
	

	// non-member functions
	// TO DO: I want to optimize this by using the itr_curr and obviating all the iterator constructions, but this is tough because itr_curr is private to list_iterator<bool>
	friend bool operator==(forward_list<T,Allocator> const& lhs, forward_list<T,Allocator> const& rhs) {
		auto lhs_curr = lhs.cbegin();
		auto rhs_curr = rhs.cbegin();
		while (lhs_curr != lhs.end()) {
			if (rhs_curr == rhs.cend() || *lhs_curr != *rhs_curr) {
				return false;
			}
			++rhs_curr;
			++lhs_curr;
		}
		return rhs_curr == rhs.end();
	}
public:
	friend std::strong_ordering operator<=>(forward_list const& lhs,forward_list const& rhs) {
		auto lhs_curr = lhs.cbegin();
		auto rhs_curr = rhs.cbegin();
		while (rhs_curr != nullptr && lhs_curr != nullptr) {
			if (rhs_curr == nullptr) {
				return std::strong_ordering::greater;
			} else if (*lhs_curr != *rhs_curr) {
				return *lhs_curr <=> *rhs_curr;
			}
			++rhs_curr;
			++lhs_curr;
		}
	// return equal if rhs and lhs are same length, else rhs is longer so return less
		if (lhs_curr == nullptr && rhs_curr != nullptr) {
			return std::strong_ordering::less;
		} else if (rhs_curr == lhs_curr) {
			return std::strong_ordering::equal;
		} else {
			return std::strong_ordering::greater;
		}
	}
};
}
#include "forward_list.hpp"
