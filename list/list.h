#pragma once
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <iostream> // DELETE ME: debugging
#include <concepts>

namespace brian {
template <typename T, typename Allocator = std::allocator<T>>
class list {
	template <bool Is_Const>
	class list_iterator;
	template <bool Is_Const>
	class reverse_list_iterator;
	struct base_node {
		base_node* next, * prev;
		base_node() :next(nullptr), prev(nullptr) {}
		base_node(int,base_node*n):prev(nullptr),next(n) {}
		base_node(base_node* p) : prev(p), next(nullptr) {}
		base_node(base_node* p, base_node* n) :next(n),prev(p){}

	};
	struct node :public base_node{
		T val;
		node() : base_node() {}
		// constructor overload used when no node is passed
		template <typename U,typename ...Args, typename = 
			std::enable_if_t<!std::is_same<U,base_node*>::value && !std::is_same<U,node*>::value>>
		node(U && u,Args &&...args) : base_node(), val(std::forward<U>(u),std::forward<Args>(args)...){}
		// constructor overload used when node is passed
		template <typename ...Args>
		node(base_node* p, Args && ...args) : base_node(p),val(std::forward<Args>(args)...){}
	};
	public:
	using value_type = T;
	using allocator_type = Allocator;
	using size_type  = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type&;
	using const_reference = value_type const&;
	using pointer = typename std::allocator_traits<Allocator>::pointer;
	using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
	using iterator = list_iterator<false>;
	using const_iterator = list_iterator<true>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;
	private:
	using node_allocator_t = typename std::allocator_traits<allocator_type>
		::template rebind_alloc<node>;
	node_allocator_t node_allocator;
	using Traits = std::allocator_traits<node_allocator_t>;
	Allocator value_allocator;

	base_node* pre_head;	
	base_node* aft_tail;
	size_t n;
	public:
	// constructors
	list();
	explicit list(allocator_type const& alloc);
	list(size_t count, T const& val, allocator_type const& alloc = Allocator());
	explicit list(size_type count, allocator_type const& alloc = allocator_type());
	template <typename It, typename std::iterator_traits<It>::pointer=nullptr>
	list(It first, It last, Allocator const& alloc = Allocator());
	list(list const& other);
	list(list const& other, Allocator const& alloc);
	list(list && other);
	list(list &&other, Allocator const& alloc);
	list(std::initializer_list<T> il, Allocator const& alloc = Allocator());
	// modifiers
	void clear() noexcept;	
	// insert
	iterator insert(const_iterator pos, T const& val); 
	iterator insert(const_iterator pos, T && val);
	iterator insert(const_iterator pos, size_t, T const& val);
	template <typename It, typename std::iterator_traits<It>::pointer = nullptr>
	iterator insert(const_iterator pos, It beg, It end);
	iterator insert(const_iterator pos, std::initializer_list<T> il);
	// pop methods
	void pop_back();
	void pop_front();
	// erase
	iterator erase(const_iterator pos);
	iterator erase(const_iterator first, const_iterator last);
	// pushers
	void push_front(T const& val);
	void push_front(T && val);
	void push_back(T const& val);
	void push_back(T && val);
	// emplace methods
	template <typename ...Args>
	iterator emplace(const_iterator pos, Args && ...args);
	template <typename ...Args>
	void emplace_front(Args && ...args);
	template <typename ...Args>
	void emplace_back(Args && ...args);
	// FIX ME: work on making a template or constraint
	void resize(size_t new_size);
	void resize(size_t new_size, T const& val);
	// assign
	void assign(size_t new_size, T const& val);
	template <typename It, typename std::iterator_traits<It>::pointer=nullptr>
	void assign(It beg, It end);
	void assign(std::initializer_list<T> il);
	// assignment operators
	list& operator=(list const& other);
	list& operator=(list && other);
	list& operator=(std::initializer_list<T> il);
	// algorithms
	void reverse() noexcept;
	size_t remove(T const& val);
	template <typename Pred>
	requires std::predicate<Pred,T>
	size_t remove_if(Pred pred);
	void swap(list& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value);
	void merge(list& other);
	void merge(list&& other);
	template <typename Pred>
	requires std::predicate<Pred,T,T>
	void merge(list& other, Pred pred);
	template <typename Pred>
	requires std::predicate<Pred,T,T>
	void merge(list&& other, Pred pred);
	void splice(const_iterator pos, list& other);
	void splice(const_iterator pos, list&& other);
	void splice(const_iterator pos, list& other, const_iterator it);
	void splice(const_iterator pos, list&& other, const_iterator it);
	void splice(const_iterator pos, list& other, const_iterator first, const_iterator last);
	
	void splice(const_iterator pos, list&& other, const_iterator first, const_iterator last);
	size_t unique();
	template <typename Eq>
	requires std::predicate<Eq,T,T>
	size_t unique(Eq eq);
	void sort() noexcept;
	template <typename Cmp>
	requires std::predicate<Cmp,T,T>
	void sort(Cmp cmp);
	// observers
	size_t size() const noexcept { return n; }
	[[nodiscard]] bool empty() const noexcept { return begin() == end(); }
	size_t max_size() const noexcept;
	Allocator get_allocator() const noexcept { return value_allocator; }
	reference front() { return *begin(); }
	const_reference front() const { return *begin(); }
	reference back() { return end().itr_curr->prev; }
	const_reference back() const { return end().itr_curr->prev; }
	// iterator_methods
	iterator begin() { return iterator(pre_head->next); }
	iterator end() { return iterator(aft_tail); }
	const_iterator begin() const { return const_iterator(pre_head->next); }
	const_iterator end() const { return const_iterator(aft_tail); }
	const_iterator cbegin() const { return const_iterator(pre_head->next); }
	const_iterator cend() const { return const_iterator(aft_tail); }

	reverse_iterator rbegin() { return reverse_iterator(aft_tail); }
	reverse_iterator rend() { return reverse_iterator(pre_head->next); }
	const_reverse_iterator rbegin() const { return const_reverse_iterator(aft_tail); }
	const_reverse_iterator rend() const { return const_reverse_iterator(pre_head->next); }
	const_reverse_iterator crbegin() const { return const_reverse_iterator(aft_tail); }
	const_reverse_iterator crend() const { return const_reverse_iterator(pre_head->next); }
		
	
	~list();
private:
	template <bool Is_Const>
	class list_iterator {
		friend list<T,Allocator>;
	public:
		using value_type = T;
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using pointer = typename std::conditional<Is_Const, T const*, T*>::type;
		using reference = typename std::conditional<Is_Const, T const&, T&>::type;
		list_iterator() = default;
		list_iterator(base_node* c) : itr_curr(c) {}
		// default copy constructor
		list_iterator(list_iterator<Is_Const> const&) = default;
		// converts iterator to const_iterator 
		template <bool Was_Const,typename = std::enable_if_t<Is_Const || !Was_Const>>
			list_iterator(list_iterator<Was_Const> const& i) : itr_curr(i.itr_curr){}
		friend bool operator==(list_iterator<Is_Const> const& lhs, list_iterator<Is_Const> const& rhs) {
			return lhs.itr_curr == rhs.itr_curr;
		}
		friend bool operator!=(list_iterator const& lhs, list_iterator const& rhs) {
			return lhs.itr_curr != rhs.itr_curr;
		}
		reference operator*() {
				return static_cast<node*>(itr_curr)->val;
		}
		// stuff unique to forward_iterator
		// prefix incrament
		list_iterator& operator++() {
			itr_curr = itr_curr->next;
			return *this;
		}
		// suffix incrament
		list_iterator operator++(int) {
			auto tmp = itr_curr;
			itr_curr = itr_curr->next;
			return list_iterator(itr_curr);
		}
		// prefix decrament
		list_iterator& operator--() {
			itr_curr = itr_curr->prev;
			return *this;
		}
		// suffix decrament
		list_iterator operator--(int) {
			auto tmp = itr_curr;
			itr_curr = itr_curr->prev;
			return list_iterator(itr_curr);
		}
		// overload the -> operator
		pointer operator->() {
			return &static_cast<node*>(itr_curr)->val;
		}

	private:
		base_node* itr_curr;
	};	
	// helper methods
	template <typename ...Args>
	node* create_node(Args &&...args) {
		node* new_node = Traits::allocate(node_allocator,1);
		try {
			Traits::construct(node_allocator,new_node,std::forward<Args>(args)...);
		}
		catch (...) {
			Traits::deallocate(node_allocator,new_node,1);
			throw;
		}
		return new_node;
	}
	void delete_node(base_node* del_node) {
		Traits::destroy(node_allocator, static_cast<node*>(del_node));
		Traits::deallocate(node_allocator, static_cast<node*>(del_node),1);
	}
	// used to connect 2 preexisting nodes
	void connect_nodes(base_node* first, base_node* second) {
		first->next = second;
		second->prev = first;
	}
	// FIX_ME: this can be simplified with concepts
	template <typename It, typename std::enable_if_t<std::is_same<typename std::iterator_traits<It>::iterator_category,std::random_access_iterator_tag>::value || std::is_same<typename std::iterator_traits<It>::iterator_category,std::contiguous_iterator_tag>::value,int > = 0 >
	iterator __insert(const_iterator const& pos, It& beg, It const& end) {
	// il.begin() and il.end() are equal if il is empty, so don't worry about reusing this for the std::initializer_list overload
	if (beg == end) return iterator(pos.itr_curr);
	auto __beg = beg;
	base_node* temp_head = create_node(*beg);
	base_node* curr = temp_head;
	try {
		auto it = ++__beg;
		for (it = __beg; it != end;++it) {
			curr->next = create_node(curr,*it);
			curr = curr->next;
		}
	} catch(...) {
		// time to clean up
		base_node* del_node;
		curr = temp_head;
		while (curr) {
			del_node = curr;
			curr = curr->next;
			delete_node(del_node);
		}
		throw;
	}

	connect_nodes(pos.itr_curr->prev,temp_head);
	connect_nodes(curr,pos.itr_curr);
	n += (end-beg);
	return iterator(temp_head);
	}
// this is the overload for iterators that do not support += operator
	template <typename It, typename std::enable_if_t<!std::is_same<typename std::iterator_traits<It>::iterator_category,std::random_access_iterator_tag>::value && !std::is_same<typename std::iterator_traits<It>::iterator_category,std::contiguous_iterator_tag>::value,int > = 0 >
	iterator __insert(const_iterator const& pos, It& beg, It const&  end) {
	// il.begin() and il.end() are equal if il is empty, so don't worry about reusing this for the std::initializer_list overload
	if (beg == end) return iterator(pos.itr_curr);
		base_node* temp_head = create_node(*beg);
		base_node* curr = temp_head;
		++n;
	try {
		auto it = ++beg;
		for (it = beg; it != end;++it) {
			curr->next = create_node(curr,*it);
			curr = curr->next;
			++n;
		}
	} catch(...) {
		// time to clean up
		base_node* del_node;
		curr = temp_head;
		while (curr != end().itr_curr) {
			del_node = curr;
			curr = curr->next;
			delete_node(del_node);
			--n;
		}
		throw;
	}

	connect_nodes(pos.itr_curr->prev,temp_head);
	connect_nodes(curr,pos.itr_curr);
	return iterator(temp_head);
	}
	void __cp(list const& other) {
		base_node* other_curr = other.pre_head->next;
		base_node* this_curr = this->pre_head;
		try {
			while (other_curr != other.end()) {
				node* new_node = create_node(this_curr,static_cast<node*>(other_curr)->val);
				this_curr->next = new_node;	
				this_curr = new_node;
				other_curr = other_curr->next;
			}
		} catch (...) {
			// clean up data
			base_node* curr = this->pre_head;
			base_node* del_node;
			while (curr) {
				del_node = curr;
				curr = curr->next;
				delete_node(del_node);
			}
			//delete_node(aft_tail);
			pre_head = nullptr;
			throw;
		}
		connect_nodes(this_curr,aft_tail);
		this->n = other.n;
		throw;
	}
	template <typename ...Args>
	void __resize(size_t new_size, Args &&...val) {
		auto curr = aft_tail->prev;
		if (new_size < n) {
			while (new_size < n) {
				auto del_node = curr;
				curr = curr->prev;
				delete_node(del_node);
				--n;
			}	
			connect_nodes(curr,aft_tail);
		}
		else if (new_size > n++){
			base_node* temp_head = create_node(val...);
			base_node* curr = temp_head;
			try {
				while (new_size > n++) {
					curr->next = create_node(curr,val...);
					curr = curr->next;
				}
			} catch (...) {
				auto del_node = temp_head;
				while (temp_head) {
					del_node = temp_head;
					temp_head = temp_head->next;
					delete_node(del_node);
					--n;
				}
				throw;
			}
			connect_nodes(aft_tail->prev, temp_head);
			connect_nodes(curr,aft_tail);
		}
	}
	template <typename Pred>
	size_t __remove_if(Pred  const& pred) {
		node* curr = static_cast<node*>(pre_head->next);
		size_t count = 0;
		while (curr != aft_tail) {
			base_node* prev = curr->prev;
			if (pred(curr->val)) {
				while (pred(curr->val)) {
					base_node* del_node = curr;
					curr = static_cast<node*>(del_node->next);
					delete_node(del_node);
					++count;
					if (curr == aft_tail) {
						connect_nodes(prev,aft_tail);
						n -= count;
						return count;
					}
				}
			connect_nodes(prev,curr);
			}
			curr = static_cast<node*>(curr->next);
		}
		n -= count;
		return count;
	}
	// must provide basic exception garentee if cmp throws an exception
	template <typename Cmp>
	void __merge(node* right_curr, base_node*& right_end,Cmp const& cmp) {
		if (this->pre_head->next == right_curr) return;
		node* this_curr = static_cast<node*>(this->pre_head->next);
		base_node* curr = this->pre_head;
		base_node* right_beg = right_curr->prev;
		try {
			while (this_curr != this->end().itr_curr && right_curr != right_end) {
				if (cmp(right_curr->val,this_curr->val)) {
					connect_nodes(curr, right_curr);
					curr = curr->next;
					right_curr = static_cast<node*>(right_curr->next);
				} else {
					connect_nodes(curr, this_curr);
					curr = curr->next;
					this_curr = static_cast<node*>(this_curr->next);
				}
			}
		} catch (...) {
			// prevent leak.
			connect_nodes(curr,this_curr);
			std::cout << right_curr->val << '\n';
			auto c = right_curr;
			while (c != right_end) {
				auto del_node = c;
				c = static_cast<node*>(c->next);
				delete_node(del_node);
			}
			connect_nodes(right_beg,right_end);
			throw;
		}
		if (this_curr != this->end()) {
			connect_nodes(curr, this_curr);
			connect_nodes(right_beg, right_end);
		}
		if (right_curr != right_end) {
			connect_nodes(curr, right_curr);
			connect_nodes(right_beg, this->aft_tail);
			// swap end nodes
			auto temp = this->aft_tail;
			this->aft_tail = right_end;
			right_end = temp;
		}
	}
	// use this overload for all six functions
	void __splice(const_iterator  pos,list&& other, const_iterator first, const_iterator last) {
		// move [first,last) to this at pos
		auto end_of_first = pos.itr_curr->prev; // one before pos
		auto front_of_second = pos.itr_curr; // pos
		auto pre_first = first.itr_curr->prev; // one before first

		connect_nodes(end_of_first, first.itr_curr);
		connect_nodes(last.itr_curr->prev,front_of_second);
		connect_nodes(pre_first,last.itr_curr);
	}
	template <typename Eq>
	size_type __unique(Eq const& eq) {
		node* curr = static_cast<node*>(pre_head->next);
		node* next = static_cast<node*>(curr->next);
		size_t count = 0;
		while (next != end()) {
			try {
				if (eq(curr->val,next->val)) {
					while (next != end() && eq(curr->val,next->val)) {
						auto del_node = next;	
						next = static_cast<node*>(next->next);
						delete_node(del_node);
						++count;
					}
					connect_nodes(curr,next);
				} 
			} catch (...) {
				connect_nodes(curr,next);
				n -= count;
				throw;
			}
			curr = static_cast<node*>(curr->next);
			next = (curr != end()) ? static_cast<node*>(curr->next) :
			   	static_cast<node*>(end().itr_curr);
		}
		n -= count;
		return count;
	}
	base_node* __split(base_node* start, size_t len,base_node*& next_sublist) {
		base_node* slow = start;
		base_node* fast = start->next;
		for (size_t i = 1; i < len && (fast->next || slow->next); ++i) {
			if (fast->next != end()) {
				fast = (fast->next->next != aft_tail) ? fast->next->next : fast->next;
			}
			if (slow->next != aft_tail) {
				slow = slow->next;
			}
		}
		base_node* mid = slow->next;
		next_sublist = fast->next;
		connect_nodes(slow,aft_tail);
		connect_nodes(fast,aft_tail);
		return mid;

	}
	template <typename Cmp>
	base_node* __merge_nodes(base_node* list1, base_node* list2, base_node*& tail, Cmp const& less) {
		base_node* l1 = list1, *l2 = list2;
		base_node dummy_head;
		base_node* new_tail = &dummy_head;
		while (list1 != aft_tail && list2 != aft_tail) {
			try {
				if (less(static_cast<node*>(list1)->val, static_cast<node*>(list2)->val)) {
					connect_nodes(new_tail,list1);
					list1 = list1->next;
					new_tail = new_tail->next;
				} else {
					connect_nodes(new_tail,list2);
					list2 = list2->next;
					new_tail = new_tail->next;
				}
			}
			catch(...) {
				// FIX ME: needs testing with throwing bipredicate
				base_node* end_of_list1 = l1;
				while (end_of_list1->next != aft_tail) {
					end_of_list1 = end_of_list1->next;
				}
				base_node* end_of_list2 = l2;
				while (end_of_list2 != aft_tail) {
					end_of_list2 = end_of_list2->next;
				}
				connect_nodes(end_of_list1,l2);
				end_of_list2 = tail;
				clear();
				throw;
			}
		}
		if (list1 != aft_tail) connect_nodes(new_tail,list1);
		else connect_nodes(new_tail,list2);
		while (new_tail->next != aft_tail) new_tail = new_tail->next;
		connect_nodes(tail,dummy_head.next);
		tail = new_tail;
		return dummy_head.next;
	}
	template <typename Cmp>
	void __sort(Cmp const& cmp) {
		base_node* tail;	
		base_node* next_sublist;
		if (size() < 2) return;
		base_node* start = pre_head->next;
		for (size_t size{1};size < n; size*=2) {
			tail = pre_head;
			while (start != aft_tail) {
				// maybe this should be == aft_tail
				if (start->next == aft_tail) {
					connect_nodes(tail,start);
					break;
				}
				base_node* mid = __split(start, size, next_sublist);
				try {
					__merge_nodes(start,mid,tail,cmp);
				}
				catch(...) {
					start = pre_head->next;
				}
				start = next_sublist;
			}
			start = pre_head->next;
		}
	}

public:
	// friends/non-member functions and comparators
	friend bool operator==(list const& lhs, list const& rhs) {
		// if the lengths are not equal, return false
		if (lhs.size() != rhs.size() ) return false;
		// iterate through both lists and compare each element
		for (auto lit = lhs.begin(), rit = rhs.begin(); lit != lhs.end();++lit,++rit) {
			if (*lit != *rit) {
				return false;
			}
		}
		return true;
	}
	// FIX ME: this needs to be optimized and I also need
	// to improve my understanding of the spaceship operator
	// and comparisons in general
	// also, why do I still need to default operator==
	// if I have the operator<=>?
	friend auto operator<=>(list const& lhs,list const& rhs) {
		auto l_it = lhs.begin(), r_it = rhs.begin();
		for (; l_it != lhs.end() && r_it != rhs.end();++l_it,++r_it) {
			if (*l_it != *r_it) return *l_it <=> *r_it;
		}
		if (l_it == lhs.end() && r_it == rhs.end()) return 0 <=> 0;
		else return (l_it == lhs.end()) ? 0 <=> 1 : 1 <=> 0;
	}
};// END CLASS LIST
}// END NAMESPACE BRIAN
namespace std {
template <typename T, typename A>
void swap(brian::list<T,A>& lhs, brian::list<T,A>&rhs) noexcept(noexcept(lhs.swap(rhs))) {
	lhs.swap(rhs);
}
template <typename T, typename A, typename U>
typename brian::list<T,A>::size_type
erase(brian::list<T,A>& li, U const& val) {
	return li.remove(val);
}
template <typename T, typename A, typename U>
requires std::predicate<U,T>
typename brian::list<T,A>::size_type
erase_if(brian::list<T,A>& li, U pred) {
	return li.remove_if(pred);
}
}
#include "list.hpp"
